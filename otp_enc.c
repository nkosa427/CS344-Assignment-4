#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define _DEFAULT_SOURCE

int main(int argc, char* argv[])
{
	int socketFD, port, charsSent, charsRead;
	struct sockaddr_in serverAddr;
	struct hostent* serverHostInfo;
	char str[256];

	// if(argc < 4){
	// 	fprintf(stderr, "Enter valid arguments\n");
	// 	exit(1);
	// }

	memset((char*)&serverAddr, '\0', sizeof(serverAddr));
	port = atoi(argv[1]);		//CHANGE LATER TO SUPPORT ARGS

	serverAddr.sin_family = AF_INET; // Create a network-capable socket
	serverAddr.sin_port = htons(port); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddr.sin_addr.s_addr, (char*)serverHostInfo->h_addr_list[0], serverHostInfo->h_length); // Copy in the address

	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if(socketFD < 0){
		fprintf(stderr, "Socket open error\n");
	}

	if(connect(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
		fprintf(stderr, "Socket connect error\n");
	}

	printf("Enter message to send: \t");
	memset(str, '\0', sizeof(str));
	fgets(str, sizeof(str)-1, stdin);



	charsSent = send(socketFD, str, strlen(str), 0);
	if(charsSent < 0){
		fprintf(stderr, "Sending to socket error\n");
	}else if(charsSent < strlen(str)){
		fprintf(stderr, "ERROR: Not all data written to socket\n");
	}



	memset(str, '\0', sizeof(str));
	charsRead = recv(socketFD, str, sizeof(str)-1, 0);
	if(charsRead < 0){
		fprintf(stderr, "Reading back from socket error\n");
	}

	printf("RECEIVED FROM SERVER: \t%s\n", str);

	close(socketFD);

	return 0;
}