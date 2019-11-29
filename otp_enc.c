#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#define _DEFAULT_SOURCE

char* getText(FILE *file){
	char c;
	char* text;
	int count = 0;

	do{
		c = fgetc(file);
		if(c < 65 && c != 32 && c != 10 || c > 90){
			// printf("char in question: %c\n", c);
			fprintf(stderr, "Invalid file contents\n");
			exit(1);
		}
		count++;
	}while(c != 10);

	text = malloc(count * sizeof(char));
	memset(text, '\0', sizeof(text));

	count = 0;
	fseek(file, 0, SEEK_SET);	//Sets file pointer back to beginning.

	do{
		c = fgetc(file);
		if(c != 10){
			text[count] = c;
		}
		count++;
	}while(c != 10);

	return text;
}

int main(int argc, char* argv[])
{
	int socketFD, port, charsSent, charsRead;
	struct sockaddr_in serverAddr;
	struct hostent* serverHostInfo;
	char str[256];
	char exitstr[4];
	char progName[8];
	char* plaintext;
	char* key;
	char* toSend;

	memset(progName, '\0', 8);
	strcpy(progName, "otp_enc");

	int goodConnection = 0;

	if(argc < 4){
		fprintf(stderr, "Enter valid arguments\n");
		exit(1);
	}

	for (int i = 0; i < strlen(argv[3]); i++){
		if(argv[3][i] < 48 || argv[3][i] > 57){	//If port arg is not an int.
			fprintf(stderr, "Enter valid port\n");
			exit(1);
		}
	}

	FILE *file;
	file = fopen(argv[1], "r");
	plaintext = getText(file);
	fclose(file);

	file = fopen(argv[2], "r");
	key = getText(file);
	fclose(file);

	if(strlen(plaintext) > strlen(key)){
		fprintf(stderr, "Key not large enough\n");
		exit(1);
	}else if(strlen(key) > strlen(plaintext)){
		key[strlen(plaintext)] = '\0';
	}

	// file = fopen("test", "w+");
	// fprintf(file, "%s", key);
	// fclose(file);

	toSend = malloc((strlen(plaintext)*2 +2 ) * sizeof(char));
	memset(toSend, '\0', sizeof(toSend));
	strcat(toSend, plaintext);
	strcat(toSend, "$");
	strcat(toSend, key);

	// printf("plaintext: %s\n", plaintext);
	// printf("key: %s\n", key);

	memset((char*)&serverAddr, '\0', sizeof(serverAddr));
	port = atoi(argv[3]);	//Changes port to integer

	serverAddr.sin_family = AF_INET; // Create a network-capable socket
	serverAddr.sin_port = htons(port); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddr.sin_addr.s_addr, (char*)serverHostInfo->h_addr_list[0], serverHostInfo->h_length); // Copy in the address

for (int i = 0; i < 2; i++){

		socketFD = socket(AF_INET, SOCK_STREAM, 0);
		if(socketFD < 0){
			fprintf(stderr, "Socket open error\n");
		}

		if(connect(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
			fprintf(stderr, "Socket connect error\n");
		}

		// printf("Enter message to send: \t");
		// memset(str, '\0', sizeof(str));
		// fgets(str, sizeof(str)-1, stdin);

		if(i == 0){
			charsSent = send(socketFD, progName, strlen(progName), 0);
		}

		if(i == 1 && goodConnection == 1){
			charsSent = send(socketFD, toSend, strlen(toSend), 0);
		}

		// if(i == 2 && goodConnection == 1){
		// 	toSend = strlen(plaintext);
		// 	charsSent = send(socketFD, key, toSend, 0);
		// }	
		// else{
		// 	memset(exitstr, '\0', 4);
		// 	strcpy(exitstr, "@@@");
		// 	toSend = strlen(exitstr);
		// 	charsSent = send(socketFD, exitstr, toSend, 0);
		// }
		
		if(charsSent < 0){
			fprintf(stderr, "Sending to socket error\n");
		}else if(i == 0 && charsSent < strlen(progName)){
			fprintf(stderr, "ERROR: Not all data written to socket\n");
		}else if(i == 1 && charsSent < strlen(toSend)){
			fprintf(stderr, "ERROR: Not all data written to socket\n");
		}

		memset(str, '\0', sizeof(str));
		charsRead = recv(socketFD, str, sizeof(str)-1, 0);
		if(charsRead < 0){
			fprintf(stderr, "Reading back from socket error\n");
		}

		if(i == 0 && strcmp(str, "Message received\n") != 0){
			fprintf(stderr, "ERROR: Client mismatch, exiting now\n");
			exit(1);
		}else{
			goodConnection = 1;
		}

		printf("RECEIVED FROM SERVER: \t%s\n", str);
	}

	close(socketFD);

	return 0;
}