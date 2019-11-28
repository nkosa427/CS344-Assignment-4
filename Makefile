CC=gcc -std=c99

all:

k:
	$(CC) keygen.c -o keygen

r:
	$(CC) smallsh.c -o smallsh -D_XOPEN_SOURCE

c:
	./smallsh

clean:
	rm -f smallsh


