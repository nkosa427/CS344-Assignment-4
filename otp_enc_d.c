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

// char* encrypt(){

// }

char* separateStrings(char* str, int arg){
	char c;
	int count = 0;
	int len = strlen(str) / 2 + 1;

	char* text = malloc(len * sizeof(char));
	memset(text, '\0', sizeof(text));

	do{
		c = str[arg];
		if(c != 36){			//36 is ascii for $
			text[count] = c;
		}
		arg++;
		count++;
	}while(c != 36 && arg < strlen(str));

	return text;

}

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

	char* plaintext;
	char* key;

	if(argc < 2){
		fprintf(stderr, "Enter correct number of args. \n");
		exit(1);
	}

	memset((char*)&serverAddr, '\0', sizeof(serverAddr));
	port = atoi(argv[1]);		

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

		// printf("SERVER connected to client on port %d\n", clientAddr.sin_port);

		childProc = fork();
		if(rightClient == 0){
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
					if(strcmp(str, "otp_enc") != 0){
						charsSent = send(estFD, "Must use otp_enc with otp_end_d", 31, 0);
					}else{
						charsSent = send(estFD, "Message received\n", 17, 0);
					}

					if(charsSent < 0){
						fprintf(stderr, "Error sending to client\n");
					}

					FILE* file;
					file = fopen("clientName", "w+");
					fprintf(file, "%s\n", str);
					fclose(file);

					close(estFD);
					exit(0);
				default:
					childPID = waitpid(childProc, &childExitMethod, 0);
					rightClient = checkClient();

					if(rightClient == 0){
						fprintf(stderr, "Not connected to otp_enc\n");
						// exit(1);
						charsSent = send(estFD, "Must use otp_enc with otp_end_d", 31, 0);
					}
			}
		}else if(rightClient == 1){
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

					char c;
					int count = 0;
					
					plaintext = separateStrings(str, count);

					do{
						c = str[count];
						count++;
					}while(c != 36);

					key =  separateStrings(str, count);

					printf("plaintext: %s\n", plaintext);
					printf("key: %s\n", key);

					// FILE *file;
					// file = fopen("toEncrypt", "a");
					// fprintf(file, "%s\n", str);
					// fclose(file);

					charsSent = send(estFD, "Message received\n", 17, 0);
					if(charsSent < 0){
						fprintf(stderr, "Error sending to client\n");
					}
					close(estFD);
					exit(0);

					
				default:
					forkCount++;

					do{
						childPID = waitpid(-1, &childExitMethod, WNOHANG);
						if(childPID > 0){
							forkCount--;
						}
					}while(childPID > 0);

					if(forkCount > 4){
						childPID = wait(&childExitMethod);
						forkCount--;
					}
			}
		}	
		
	}while(1);

	close(listenFD);
	printf("Server stopped. %d\n", getpid());


	return 0;
}