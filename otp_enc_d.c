#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char* argv[])
{
	int listenFD, bindFD, estFD;
	int port;
	int charsRead, charsSent;

	socklen_t clientInfo;
	char str[256];
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;

	if(argc < 2){
		fprintf(stderr, "Enter correct number of args. \n");
		exit(1);
	}

	memset((char*)&serverAddr, '\0', sizeof(serverAddr));
	port = atoi(argv[1]);
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	listenFD = socket(AF_INET, SOCK_STREAM, 0);
	if(listenFD < 0){
		fprintf(stderr, "Socket open error\n");
	}

	bindFD = bind(listenFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(bindFD < 0){
		fprintf(stderr, "Binding error\n");
	}

	listen(listenFD, 5);

	clientInfo = sizeof(clientAddr);
	estFD = accept(listenFD, (struct sockaddr*)&clientAddr, &clientInfo);
	if(estFD < 0){
		fprintf(stderr, "Accept error\n");
	}

	memset(str, '\0', 256);
	charsRead = recv(estFD, str, 255, 0);
	if(charsRead < 0){
		fprintf(stderr, "Reading error\n");
	}

	printf("Server received message from client:\t");
	printf("%s\n", str);

	charsSent = send(estFD, "Message received\n", 17, 0);
	if(charsSent < 0){
		fprintf(stderr, "Error sending to client\n");
	}

	close(estFD);
	close(listenFD);

	return 0;
}