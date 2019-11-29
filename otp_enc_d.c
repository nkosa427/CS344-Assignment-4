#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

int checkClient(){
	FILE* file;
	char c;
	char str[256];
	int count = 0;
	file = fopen("clientName", "r");

	do{
		c = fgetc(file);
		if(c != 10 && count < 256){
			str[count] = c;
		}
		count++;
	}while(c != 10 && count < 256);

	fclose(file);
	remove("clientName");

	if(strcmp(str, "otp_enc") == 0){
		printf("check0\n");
		return 1;
	}else{
		return 0;
	}

}

int main(int argc, char* argv[])
{
	int listenFD, bindFD, estFD;
	int port;
	int charsRead, charsSent;

	int forkCount = 0;
	int parentPID = getpid();
	int childExitMethod = -5;
	int childPID = -5;

	int rightClient = 0;

	socklen_t clientInfo;
	char str[256];
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	struct hostent* serverHostInfo;

	pid_t childProc;

	if(argc < 2){
		fprintf(stderr, "Enter correct number of args. \n");
		exit(1);
	}

	// printf("Parent pid: %d\n", getpid());
	// for (int i = 0; i < 5; i++){
	// 	processes[i] = fork();
	// 	printf("%s\n", );
	// }

	// memset((char*)&serverAddr, '\0', sizeof(serverAddr));
	// port = atoi(argv[1]);
	
	// serverAddr.sin_family = AF_INET;
	// serverAddr.sin_port = htons(port);

	// struct hostnet* serverInfo = gethostbyname("localhost");
	// // serverAddr.sin_addr.s_addr = 
	// memcpy((char*)&serverAddr.sin_addr.s_addr, (char*)serverInfo->h_addr_list[0], serverInfo->h_length);

	memset((char*)&serverAddr, '\0', sizeof(serverAddr));
	port = atoi(argv[1]);		//CHANGE LATER TO SUPPORT ARGS

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverHostInfo = gethostbyname("localhost");
	if(serverHostInfo == NULL){
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(0); 
	}
	memcpy((char*)&serverAddr.sin_addr.s_addr, (char*)serverHostInfo->h_addr_list[0], serverHostInfo->h_length);



	listenFD = socket(AF_INET, SOCK_STREAM, 0);
	if(listenFD < 0){
		fprintf(stderr, "Socket open error\n");
	}

	bindFD = bind(listenFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(bindFD < 0){
		fprintf(stderr, "Binding error\n");
	}

	listen(listenFD, 5);

	do{
		clientInfo = sizeof(clientAddr);
		estFD = accept(listenFD, (struct sockaddr*)&clientAddr, &clientInfo);
		if(estFD < 0){
			fprintf(stderr, "Accept error\n");
		}

		printf("SERVER connected to client on port %d\n", clientAddr.sin_port);

		childProc = fork();
		switch(childProc){
			case -1:
				perror("forking error");
				exit(1);
			case 0:
				memset(str, '\0', 256);
				charsRead = recv(estFD, str, 255, 0);
				if(charsRead < 0){
					fprintf(stderr, "Reading error\n");
				}

				printf("RECEIVED FROM CLIENT:\t%s\n", str);

				if(rightClient == 0){
					FILE* file;
					file = fopen("clientName", "w+");
					fprintf(file, "%s\n", str);
					fclose(file);
				}

				charsSent = send(estFD, "Message received\n", 17, 0);
				if(charsSent < 0){
					fprintf(stderr, "Error sending to client\n");
				}
				close(estFD);
				exit(0);
			default:
				printf("background PID: %d\n", childProc);
				forkCount++;

				if(rightClient == 0){
					childPID = waitpid(childProc, &childExitMethod, 0);
					forkCount--;
					rightClient = checkClient();

					if(rightClient == 0){
						fprintf(stderr, "Not connected to otp_enc\n");
						exit(1);
						// charsSent = send(estFD, "Must use otp_enc with dec\n", 26, 0);
					}
				}

				do{
					childPID = waitpid(-1, &childExitMethod, WNOHANG);
					printf("childpid:%d\n", childPID);
					if(childPID > 0){
						forkCount--;
					}
				}while(childPID > 0);

				if(forkCount > 4){
					childPID = wait(&childExitMethod);
					forkCount--;
				}
		}
		
	}while(1);

	close(listenFD);
	printf("Server stopped. %d\n", getpid());


	return 0;
}