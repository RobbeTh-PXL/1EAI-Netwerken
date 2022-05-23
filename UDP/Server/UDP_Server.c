#define _WIN32_WINNT 0x0601

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <unistd.h>

//PRINTS CONNECTION INFO
void print_ip_address( unsigned short family, struct sockaddr * ip ) {
	void * ip_address;
	char * ip_version;
	char ip_string[INET6_ADDRSTRLEN];

	if( family == AF_INET )
	{ // IPv4
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)ip;
		ip_address = &(ipv4->sin_addr);
		ip_version = "IPv4";
	}
	else
	{ // IPv6
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ip;
		ip_address = &(ipv6->sin6_addr);
		ip_version = "IPv6";
	}

	inet_ntop( family, ip_address, ip_string, sizeof ip_string );
	printf( "%s -> %s\n", ip_version, ip_string );
}

void ai_print_ip_address( struct addrinfo * ip ) {
	print_ip_address( ip->ai_family, ip->ai_addr );
}

void ss_print_ip_address( struct sockaddr_storage * ip ) {
	print_ip_address( ip->ss_family, (struct sockaddr*) ip );
}
//PRINTS CONNECTION INFO

int main( int argc, char * argv[] ) {
//START SOCKET API
	WSADATA wsaData; //WSAData wsaData; //Could be different case
	if( WSAStartup( MAKEWORD(2,0), &wsaData ) != 0 ) // MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:
	{
		fprintf( stderr, "WSAStartup failed.\n" );
		exit( 1 );
	}
//START SOCKET API

//ASK USER FOR SERVER IP, PORT, AMOUNT
  char server_ip[45] = "\0";
  char server_port[5] = "\0";
  int amount = 0;
  int timeout = 0;

  printf("\nSet Server IP:\n");
  printf("[?] > ");
  scanf("%s", server_ip);
  fflush(stdin);

  printf("\nSet UDP Server PORT:\n");
  printf("[?] > ");
  scanf("%s", server_port);
  fflush(stdin);

  printf("\nAmount Of Packets:\n");
	printf("[?] > ");
	scanf("%d", &amount);
	fflush(stdin);

  printf("\nTimeout:\n");
	printf("[?] > ");
	scanf("%d", &timeout);
	fflush(stdin);
//ASK USER FOR SERVER IP, PORT, AMOUNT

//SOCKET SETUP
	struct addrinfo internet_address_setup, *result_head, *result_item;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_INET; // AF_INET or AF_INET6 to force version AF_UNSPEC
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	internet_address_setup.ai_flags = AI_PASSIVE; // use ANY address for IPv4 and IPv6

	int getaddrinfo_return;
	getaddrinfo_return = getaddrinfo( server_ip, server_port, &internet_address_setup, &result_head );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 2 );
	}
//SOCKET SETUP

//CREATE SOCKET (internet_address_setup)
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
			int return_code;
			return_code = bind( internet_socket, result_item->ai_addr, result_item->ai_addrlen );
			if( return_code == -1 )
			{
				close( internet_socket );
				perror( "bind" );
			}
			else
			{
				printf( "Bind to " );
				ai_print_ip_address( result_item );
				break; //stop running through the linked list
			}
		}
		result_item = result_item->ai_next; //take next in the linked list
	}
	if( result_item == NULL )
	{
		fprintf( stderr, "socket: no valid socket address found\n" );
		exit( 3 );
	}
	freeaddrinfo( result_head ); //free the linked list
//CREATE SOCKET (internet_address_setup)

//RECEIVE MSG
	int number_of_bytes_received = 0;
	char buffer[1000];

  struct sockaddr_storage client_ip_address;
	socklen_t client_ip_address_length = sizeof client_ip_address;

  for (int i = 0; i < amount; i++) {
    number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_ip_address, &client_ip_address_length );
  	if( number_of_bytes_received == -1 )
  	{
  		printf( "errno = %d\n", WSAGetLastError() ); //https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
  		perror( "recvfrom" );
  	}
  	buffer[number_of_bytes_received] = '\0';
  	printf( "Got %s from ", buffer );
  	ss_print_ip_address( &client_ip_address );
  }
//RECEIVE MSG

//CLOSE CONNECTION & CLEANUP
	close( internet_socket );
	WSACleanup();
//CLOSE CONNECTION & CLEANUP

	return 0;
}
