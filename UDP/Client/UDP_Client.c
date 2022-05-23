#define _WIN32_WINNT 0x0601

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <unistd.h>

#include <string.h>

//PRINTS CONNECTION INFO
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
//PRINTS CONNECTION INFO

int main( int argc, char * argv[] ) {
  printf("__//UDP Server\\\\__\n");

//START SOCKET API
  printf("\n[+] Starting API...\n");
	WSADATA wsaData; //WSAData wsaData; //Could be different case
	if( WSAStartup( MAKEWORD(2,0), &wsaData ) != 0 ) // MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:
	{
		fprintf( stderr, "WSAStartup failed.\n" );
		exit( 1 );
	}
//START SOCKET API

//ASK USER FOR SERVER IP, PORT, AMOUNT, MESSAGE
  char server_ip[45] = "\0";
  char server_port[5] = "\0";
	char msg[100] = "\0";
	int amount = 0;

  printf("\nUDP Server IP:\n");
  printf("[?] > ");
  scanf("%s", server_ip);
  fflush(stdin);

  printf("\nUDP Server Port:\n");
  printf("[?] > ");
  scanf("%s", server_port);
  fflush(stdin);

	printf("\nAmount Of Packets:\n");
	printf("[?] > ");
	scanf("%d", &amount);
	fflush(stdin);

	printf("\nMessage:\n");
	printf("[?] > ");
	scanf("%s", msg);
	fflush(stdin);
//ASK USER FOR SERVER IP, PORT, AMOUNT, MESSAGE

//SOCKET SETUP
	struct addrinfo internet_address_setup, *result_head, *result_item;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	internet_address_setup.ai_socktype = SOCK_DGRAM;
  internet_address_setup.ai_protocol = IPPROTO_UDP;

	int getaddrinfo_return;
	getaddrinfo_return = getaddrinfo( server_ip, server_port, &internet_address_setup, &result_head );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 2 );
	}
//SOCKET SETUP

//CREATE SOCKET (internet_address_setup)
	struct sockaddr * internet_address;
	size_t internet_address_length;
	int internet_socket;


	result_item = result_head; //take first of the linked list
	while( result_item != NULL ) {//while the pointer is valid
		internet_socket = socket( result_item->ai_family, result_item->ai_socktype, result_item->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			printf( "[+] Sending to " );
			print_ip_address( result_item );
			internet_address_length = result_item->ai_addrlen;
			internet_address = (struct sockaddr*) malloc( internet_address_length );
			memcpy( internet_address, result_item->ai_addr, internet_address_length );
			break; //stop running through the linked list
		}
		result_item = result_item->ai_next; //take next in the linked list
	}
	if(result_item == NULL)
	{
		fprintf(stderr, "socket: no valid socket address found\n");
		exit(3);
	}
	freeaddrinfo(result_head); //free the linked list
//CREATE SOCKET (internet_address_setup)

//SEND MSG
	int number_of_bytes_send = 0;

	for (int i = 0; i < amount; i++) {
		printf("[+] Sending Message %d/%d\n", i+1, amount);
		number_of_bytes_send = sendto(internet_socket, msg, strlen(msg), 0, internet_address, internet_address_length);
		if(number_of_bytes_send == -1)
		{
			perror("sendto");
		}
	}
//SEND MSG

//CLOSE CONNECTION & CLEANUP
	close( internet_socket );
	WSACleanup();
//CLOSE CONNECTION & CLEANUP

	return 0;
}
