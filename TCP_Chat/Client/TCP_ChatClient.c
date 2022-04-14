/* TODO
--Ask user for server ip      OK
--Ask user for server port    OK

--Ask user for username       OK

--Add /exit flag to close the chat and connection

--Structure the messages: username,time,message
--Decode the messages to correctly display in terminal

--Send and receive at the same time		OK
*/


#define _WIN32_WINNT 0x0601

#include <winsock2.h>
#include <ws2tcpip.h>
#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h> //Multi-Threading

//DISPLAYS CONNECTION INFO
void print_ip_address( struct addrinfo * ip )
{
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
//DISPLAYS CONNECTION INFO

//RECEIVE MSG THREAD
void *client_recv (void *socket) {
	int internet_socket = (intptr_t) socket;
//ClIENT RECEIVE
	int number_of_bytes_received = 0;
	char recv_buffer[1000];
	while (1) {
		number_of_bytes_received = recv(internet_socket, recv_buffer, sizeof(recv_buffer), 0);
		if (number_of_bytes_received == -1) {
			//perror("recv");
		}
		if (number_of_bytes_received > 0) {
			recv_buffer[number_of_bytes_received] = '\0';
			printf("Received: %s\n", recv_buffer);
			number_of_bytes_received = 0;
		}
		//CLIENT RECEIVE
	}
	pthread_exit(NULL);
}
//RECEIVE MSG THREAD

int main( int argc, char * argv[] )
{

//START SOCKET API
	WSADATA wsaData; //WSAData wsaData; //Could be different case
	if( WSAStartup( MAKEWORD(2,0), &wsaData ) != 0 ) // MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:
	{
		fprintf( stderr, "WSAStartup failed.\n" );
		exit( 1 );
	}
//START SOCKET API

//ASK USER FOR SERVER IP, PORT AND USERNAME.
  char server_ip[45];
  char server_port[5];
  char username[20];

  printf("Enter The TCP_ChatServer IP [IPv4/IPv6]: ");
  scanf("%s", server_ip);

  printf("Enter The TCP_ChatServer PORT [1-99999]: ");
  scanf("%s", server_port);

  printf("Enter Your Username [20]: ");
  scanf("%s", username);
//ASK USER FOR SERVER IP, PORT AND USERNAME.

//SET CONNECT TO IP, IP-VERSION,  PORT, PROTOCOL
	struct addrinfo internet_address_setup, *result_head, *result_item;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	internet_address_setup.ai_socktype = SOCK_STREAM;
  internet_address_setup.ai_protocol = IPPROTO_TCP;

	int getaddrinfo_return;
	getaddrinfo_return = getaddrinfo( server_ip, server_port, &internet_address_setup, &result_head );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 2 );
	}
//SET CONNECT TO IP, IP-VERSION,  PORT, PROTOCOL

/* NOT USED
	struct sockaddr * internet_address;
	size_t internet_address_length;
*/

//CONNECT TO TARGET (internet_address_setup)
	int internet_socket;

	result_item = result_head; //take first of the linked list
	while( result_item != NULL ) //while the pointer is valid
	{
		internet_socket = socket( result_item->ai_family, result_item->ai_socktype, result_item->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			printf( "Connecting to " );
			print_ip_address( result_item );

			int connect_return;
			connect_return = connect( internet_socket, result_item->ai_addr, result_item->ai_addrlen );
			if( connect_return == -1 )
			{
				perror( "connect" );
				close( internet_socket );
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
		fprintf( stderr, "socket: no valid socket address found or could not connect!\n" );
		exit( 4 );
	}
	freeaddrinfo( result_head ); //free the linked list
//CONNECT TO TARGET (internet_address_setup)

//CREATE THREAD RECEIVE MSG
	pthread_t trd1; //
	pthread_create(&trd1, NULL, client_recv, (void *) (intptr_t) internet_socket);
//CREATE THREAD RECEIVE MSG

//ClIENT SEND
  int number_of_bytes_send = 0;
  number_of_bytes_send = send( internet_socket, username, strlen(username), 0 );
  if( number_of_bytes_send == -1 ) {
    printf( "errno = %d\n", WSAGetLastError() ); //https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
    perror( "send" );
  }
//ClIENT SEND

//CLOSE CONNECTION & CLEANUP
	int shutdown_return;
	printf("--//Shutting Down Client\\\\--\n");
	printf("//Stopping Threads...\n");
	pthread_cancel(trd1); //Close the thread
	printf("//Stopping Comms...\n");
	shutdown_return = shutdown( internet_socket, SD_SEND ); //Shutdown Send == SD_SEND ; Receive == SD_RECEIVE ; Send/Receive == SD_BOTH ; https://blog.netherlabs.nl/articles/2009/01/18/the-ultimate-so_linger-page-or-why-is-my-tcp-not-reliable --> Linux : Shutdown Send == SHUT_WR ; Receive == SHUT_RD ; Send/Receive == SHUT_RDWR
	if( shutdown_return == -1 )
	{
		printf( "errno = %d\n", WSAGetLastError() ); //https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
		perror( "shutdown" );
	}

	printf("//Closing Comms...\n");
	close( internet_socket );

	printf("//Stopping API...\n");
	WSACleanup();

	printf("--//Shutdown Complete\\\\--\n");
//CLOSE CONNECTION & CLEANUP

	return 0;
}
