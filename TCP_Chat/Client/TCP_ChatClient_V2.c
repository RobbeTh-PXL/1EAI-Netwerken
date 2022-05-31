#define _WIN32_WINNT 0x0601

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>

int thread_stop = 0;

//PRINT CONNECTION INFO
void print_ip_address( struct addrinfo * ip ) {
	void * ip_address;
	char * ip_version;
	char ip_string[INET6_ADDRSTRLEN];

	if( ip->ai_family == AF_INET )
	{ // IPv4
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)ip->ai_addr;
		ip_address = &(ipv4->sin_addr);
		ip_version = "IPv4";
	}
	else
	{ // IPv6
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ip->ai_addr;
		ip_address = &(ipv6->sin6_addr);
		ip_version = "IPv6";
	}

	inet_ntop( ip->ai_family, ip_address, ip_string, sizeof ip_string );
	printf( "%s -> %s\n", ip_version, ip_string );
}
//PRINT CONNECTION INFO

//RECEIVE DATA
void *data_recv(void *server_socket) {
  int recv_socket = (intptr_t) server_socket;
  int number_of_bytes_received = 0;
  char buffer[1000] = "\0";

  while (thread_stop == 0) {
    number_of_bytes_received = recv(recv_socket, buffer, sizeof(buffer), 0);
    if (number_of_bytes_received == -1) {
      perror("recv");
    }
    buffer[number_of_bytes_received] = '\0';
    if (strlen(buffer)>0) {
      printf("[*] %s\n", buffer);
    }
  }
  pthread_exit(NULL);
  return NULL; //To make compiler happy
}
//RECEIVE DATA

int main( int argc, char * argv[] )
{
//START WINSOCK API
	WSADATA wsaData; //WSAData wsaData; //Could be different case
	if( WSAStartup( MAKEWORD(2,0), &wsaData ) != 0 ) // MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:
	{
		fprintf( stderr, "WSAStartup failed.\n" );
		exit( 1 );
	}
//START WINSOCK API

//ASK USER FOR SERVER INFO
  char server_ip[45] = "\0";
  char server_port[5] = "\0";

  printf("SERVER IP:\n");
  printf("[>] ");
  scanf("%s", server_ip);
  fflush(stdin);

  printf("\nSERVER PORT:\n");
  printf("[>] ");
  scanf("%s", server_port);
  fflush(stdin);
//ASK USER FOR SERVER INFO

//SETUP SOCKET
	struct addrinfo internet_address_setup, *result_head, *result_item;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	internet_address_setup.ai_socktype = SOCK_STREAM;

	int getaddrinfo_return;
	getaddrinfo_return = getaddrinfo( server_ip, server_port, &internet_address_setup, &result_head );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 2 );
	}
//SETUP SOCKET

/* NOT USED
	struct sockaddr * internet_address;
	size_t internet_address_length;
*/

//CREATE SOCKET
  int server_socket;

	result_item = result_head; //take first of the linked list
	while( result_item != NULL ) //while the pointer is valid
	{
		server_socket = socket( result_item->ai_family, result_item->ai_socktype, result_item->ai_protocol );
		if( server_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			printf( "Connecting to " );
			print_ip_address( result_item );

			int connect_return;
			connect_return = connect( server_socket, result_item->ai_addr, result_item->ai_addrlen );
			if( connect_return == -1 )
			{
				perror( "connect" );
				close( server_socket );
			}
			else
			{
				printf( "Connected\n" );
				break; //stop running through the linked list
			}
		}
		result_item = result_item->ai_next; //take next in the linked list
	}
	if( result_item == NULL )
	{
		fprintf( stderr, "socket: no valid socket address found\n" );
		exit( 4 );
	}
	freeaddrinfo( result_head ); //free the linked list
//CREATE SOCKET

//CREATE RECV_THREAD
  pthread_t recv_thread;

  if (pthread_create(&recv_thread, NULL, data_recv, (void *) (intptr_t) server_socket) != 0) {
    printf("[-] Error creating recv_thread!\n");
    return 0;
  }
//CREATE RECV_THREAD

//SEND DATA
	int number_of_bytes_send = 0;
  char data_send[1000] = "\0";

  while (1) {
    printf("[>] ");
    fflush(stdin);
    gets(data_send);
    if (strcmp(data_send, "/exit") == 0) {
      number_of_bytes_send = send( server_socket, data_send, strlen(data_send), 0 );
    	if( number_of_bytes_send == -1 )
    	{
    		printf( "errno = %d\n", WSAGetLastError() ); //https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
    		perror( "send" );
    	}
      break;
    }
    strcat(data_send, "\n");
    number_of_bytes_send = send( server_socket, data_send, strlen(data_send), 0 );
  	if( number_of_bytes_send == -1 )
  	{
  		printf( "errno = %d\n", WSAGetLastError() ); //https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
  		perror( "send" );
  	}
  }
//SEND DATA

//SHUTDOWN
	int shutdown_return;
  thread_stop = 1;
	shutdown_return = shutdown( server_socket, SD_RECEIVE ); //Shutdown Send == SD_SEND ; Receive == SD_RECEIVE ; Send/Receive == SD_BOTH ; https://blog.netherlabs.nl/articles/2009/01/18/the-ultimate-so_linger-page-or-why-is-my-tcp-not-reliable --> Linux : Shutdown Send == SHUT_WR ; Receive == SHUT_RD ; Send/Receive == SHUT_RDWR
	if( shutdown_return == -1 )
	{
		printf( "errno = %d\n", WSAGetLastError() ); //https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
		perror( "shutdown" );
	}

	close( server_socket );
  pthread_join(recv_thread, NULL);
	WSACleanup();
//SHUTDOWN

	return 0;
}
