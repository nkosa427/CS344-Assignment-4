CC=gcc -std=c99

all:

k:
	rm -f keygen
	$(CC) keygen.c -o keygen

ed:
	rm -f ed
	$(CC) otp_enc_d.c -o ed -D_XOPEN_SOURCE

ec:
	rm -f ec
	$(CC) otp_enc.c -o ec

dd:
	rm -f dd
	$(CC) otp_dec_d.c -o dd

dc:
	rm -f dc
	$(CC) otp_dec.c -o dc

clean:
	rm -f keygen
	rm -f ec
	rm -f ed
	rm -f dc
	rm -f dd


