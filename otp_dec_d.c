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

char* decryptMsg(char* cipher, char* key){
	char* msg = malloc(strlen(cipher) * sizeof(char));
	memset(msg, '\0', sizeof(msg));
	int p;
	int k;
	int sum;

	for (int i = 0; i < strlen(cipher); i++){
		p = (int)cipher[i] - 65;
		if(p < 0){		//32(space) - 65 = -33, so make it the 27th char
			p = 26;
		}

		k = (int)key[i] - 65;
		if(k < 0){
			k = 26;
		}

		sum = p-k;
		sum = sum % 27;
		if(sum < 0){
			sum += 27;	//Prevents space from being negative
		}
		sum += 65;			//turn back into ascii
		if(sum == 91){	//91 = 26 (space) + 65
			sum = 32;			//Make 27th character a space
		}
		msg[i] = (char)sum;
	}

	return msg;

}

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

	if(strcmp(str, "otp_dec") == 0){
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
	char str[100000];
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	struct hostent* serverHostInfo;

	pid_t childProc;

	char* cipher;
	char* key;
	char* msg;

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

		// printf("SERVER: NEXT LOOP\n");

		// printf("SERVER connected to client on port %d\n", clientAddr.sin_port);

		childProc = fork();
		switch(childProc){
			case -1:
				perror("forking error");
				exit(1);
			case 0:
				memset(str, '\0', 100000);
				charsRead = recv(estFD, str, 100000, 0);
				if(charsRead < 0){
					fprintf(stderr, "Reading error\n");
				}

				// printf("SERVER0: RECEIVED FROM CLIENT:\t%s\n", str);

				if(strcmp(str, "otp_dec$$") != 0){
					charsSent = send(estFD, "Must use otp_dec with otp_dec_d", 31, 0);
				}else{
					charsSent = send(estFD, "Message received\n", 17, 0);
					rightClient = 1;
				}

				if(charsSent < 0){
					fprintf(stderr, "Error sending to client\n");
				}

				close(estFD);

				// FILE* file;
				// file = fopen("clientName", "w+");
				// fprintf(file, "%s\n", str);
				// fclose(file);

				if(rightClient == 1){
					clientInfo = sizeof(clientAddr);
					estFD = accept(listenFD, (struct sockaddr*)&clientAddr, &clientInfo);
					if(estFD < 0){
						fprintf(stderr, "Accept error\n");
					}

					memset(str, '\0', 100000);
					charsRead = recv(estFD, str, 100000, 0);
					if(charsRead < 0){
						fprintf(stderr, "Reading error\n");
					}

					// printf("SERVER1: RECEIVED FROM CLIENT:\t%s\n", str);

					char c;
					int count = 0;

					cipher = separateStrings(str, count);

					do{
						c = str[count];
						count++;
					}while(c != 36);

					key =  separateStrings(str, count);


					// printf("SERVER: plaintext: %s\n", plaintext);
					// printf("SERVER: key: %s\n", key);

					msg = decryptMsg(cipher, key);
					// printf("SERVER: Encrypted msg: %s\n", msg);

					charsSent = send(estFD, msg, strlen(msg), 0);
					if(charsSent < 0){
						fprintf(stderr, "Error sending to client\n");
					}
					close(estFD);

				}
				
				exit(0);

			default:
				forkCount++;
				do{
					childPID = waitpid(-1, &childExitMethod, WNOHANG);
					if(childPID > 0){
						forkCount--;
					}
				}while(childPID > 0);

				// if(forkCount > 4){
					childPID = wait(&childExitMethod);
					forkCount--;
				// }

		}
		
	}while(1);

	close(listenFD);
	printf("Server stopped. %d\n", getpid());


	return 0;
}