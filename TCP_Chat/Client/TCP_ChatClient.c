/* TODO
--Ask user for server ip      OK
--Ask user for server port		OK

--Ask user for username       OK
--Ask server if username is already taken		OK

--Add /exit flag to close the chat and connection		OK
--Add other flags																		OK

--Structure the messages: time, username, message		OK

--Send and receive at the same time		OK

--Show sender info
*/


#define _WIN32_WINNT 0x0601

#include <winsock2.h>
#include <ws2tcpip.h>
#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>

int thread_stop = 0; //Flag for breaking continuous loop in threads, allowing them to exit
int number_of_bytes_received = 0; //For system messages
char recv_buffer[1000]; //For system messages

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

//RECEIVE MSG THREAD
void *client_recv(void *socket) {
	int internet_socket = (intptr_t) socket;

	//RECEIVE MSG
	while (thread_stop == 0) {
		number_of_bytes_received = recv(internet_socket, recv_buffer, sizeof(recv_buffer), 0);
		if (number_of_bytes_received == -1) {
			perror("recv");
		}
		if (number_of_bytes_received > 4) { //Message for user
			recv_buffer[number_of_bytes_received] = '\0';
			printf("%s\n> ", recv_buffer);
			number_of_bytes_received = 0;
		}
		if (number_of_bytes_received <= 4) { //Message for system
			recv_buffer[number_of_bytes_received] = '\0';
			number_of_bytes_received = 0;
		}
	//RECEIVE MSG
	}
	pthread_exit(NULL);
	return NULL; //To make compiler happy
}
//RECEIVE MSG THREAD

//SEND MSG
void client_send(int internet_socket, char *data) {
	int number_of_bytes_send = 0;

	number_of_bytes_send = send( internet_socket, data, strlen(data), 0 );
	if( number_of_bytes_send == -1 ) {
		printf( "errno = %d\n", WSAGetLastError() ); //https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
		perror( "send" );
	}
}
//SEND MSG

//COMPOSE MSG
void msg_compose(char *username, char *interf_input, char *send_buffer) {
	//TIME
	time_t seconds;
  struct tm *timeStruct;

  seconds = time(NULL);

  timeStruct = localtime(&seconds);
	//TIME

	sprintf(send_buffer, "[%d:%02d] %s: %s\n", timeStruct->tm_hour, timeStruct->tm_min, username, interf_input);
}
//COMPOSE MSG

//PRINTS HELP MENU
void help_menu() {
	printf("\n--Help Menu--\n");
	printf("Commands:\n");
	printf("/help \t\t -- \t Shows this menu\n");
	printf("/exit \t\t -- \t Closes the chat and application\n");
	printf("/senderInfo \t -- \t Toggles the display of sender connection info\n");
	printf("\n");
}
//PRINTS HELP MENU

int main( int argc, char * argv[] )
{
printf("--//TCP Chat Client\\\\--\n");

//START SOCKET API
printf("//Starting API...\n");
	WSADATA wsaData; //WSAData wsaData; //Could be different case
	if( WSAStartup( MAKEWORD(2,0), &wsaData ) != 0 ) // MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:
	{
		fprintf( stderr, "WSAStartup failed.\n" );
		exit( 1 );
	}
//START SOCKET API

//ASK USER FOR SERVER IP, PORT
  char server_ip[45];
  char server_port[5];

	printf("\n--Connection Settings--\n");

  printf("TCP_ChatServer IP [IPv4/IPv6]: ");
  scanf("%s", server_ip);

  printf("TCP_ChatServer PORT [1-99999]: ");
  scanf("%s", server_port);
//ASK USER FOR SERVER IP, PORT

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

	printf("\n//Establishing Connection...\n");

	result_item = result_head; //take first of the linked list
	while( result_item != NULL ) { //while the pointer is valid
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
				printf( "//Connection Established\n\n");
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
	pthread_t thread_recv; //
	if (pthread_create(&thread_recv, NULL, client_recv, (void *) (intptr_t) internet_socket) != 0) {
		perror("Error Creating thread");
		exit(3);
	}
//CREATE THREAD RECEIVE MSG

//ASK USERNAME
	char username[20];

	printf("Enter A Username [20]: ");
	scanf("%s", username);
//ASK USERNAME

//VALIDATE USERNAME
	printf("//Validating Username...\n");
	client_send(internet_socket, username);

	while (1) {
		if (number_of_bytes_received <= 4) {
			if (strcmp(recv_buffer, "0\r") == 0) {
				printf("\n");
				break;
			}
			if (strcmp(recv_buffer, "1\r") == 0) {
				strcpy(recv_buffer, "\0"); //While loop is faster than client_send writing to recv_buffer (val stays 1\r) -> infinite loop in this if statement
				printf("//Username already in use!\n");
				printf("New username: ");
				scanf("%s", username);
				printf("//Validating Username...\n");
				client_send(internet_socket, username);
			}
		}
	}
//VALIDATE USERNAME

//CHAT USER INTERFACE
	char interf_input[1000] = "\0";
	char send_buffer[1030];

	printf("Use /help to view all commands.\n");

	while (1) {
		printf("> ");
		fflush(stdin);
		gets(interf_input);
		if (strcmp(interf_input, "/help") == 0) {
			help_menu();
			strcpy(interf_input, "\0");
		}

		if (strcmp(interf_input, "/exit") == 0) {
			break;
		}

		if (strlen(interf_input) > 0) {
			msg_compose(username, interf_input, send_buffer);
			client_send(internet_socket, send_buffer);
			strcpy(interf_input, "\0");
		}
	}
//CHAT USER INTERFACE

//CLOSE CONNECTION & CLEANUP
	int shutdown_return;
	printf("\n--//Shutting Down Client\\\\--\n");

	thread_stop = 1; //flag for breaking continuous loop in threads, allowing them to exit

	printf("  //Stopping Comms...\n");
	shutdown_return = shutdown( internet_socket, SD_SEND ); //Shutdown Send == SD_SEND ; Receive == SD_RECEIVE ; Send/Receive == SD_BOTH ; https://blog.netherlabs.nl/articles/2009/01/18/the-ultimate-so_linger-page-or-why-is-my-tcp-not-reliable --> Linux : Shutdown Send == SHUT_WR ; Receive == SHUT_RD ; Send/Receive == SHUT_RDWR
	if( shutdown_return == -1 )
	{
		printf( "errno = %d\n", WSAGetLastError() ); //https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
		perror( "shutdown" );
	}

	printf("  //Closing Socket...\n");
	close( internet_socket );

	printf("  //Stopping Threads...\n");
	pthread_join(thread_recv, NULL); //Check if thread is stopped by joining it.

	printf("  //Stopping API...\n");
	WSACleanup();

	printf("--//Shutdown Complete\\\\--\n");
//CLOSE CONNECTION & CLEANUP

	return 0;
}
