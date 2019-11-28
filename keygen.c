#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

int main(int argc, char* argv[])
{
	srand(time(0));
	// printf("strlen: %d\n", argc);

	if(argc < 2){
		fprintf(stderr, "Enter valid key length\n");
		exit(1);
	}

	// printf("string: %s\n", argv[1]);

	for (int i = 0; i < strlen(argv[1]); i++){
		if(argv[1][i] < 48 || argv[1][i] > 57){
			fprintf(stderr, "Enter valid key length\n");
			exit(1);
		}
	}

	int len = atoi(argv[1]);
	char* key = malloc(len * sizeof(char));

	for (int i = 0; i < len; i++){
		key[i] = 65 + rand() % 27;
		if(key[i] == 91){		//If last char is ascii
			key[i] = 32;			// 91, make it a space.
		}
	}

	printf("%s\n", key);

	return 0;
}