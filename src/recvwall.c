#include "bs.h"
#include <math.h>

#ifndef min
#define min(a, b) ((b > a) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((b > a) ? (b) : (a))
#endif


int recvwcb(void* buf, int* prcvd_last, void* data) {
	FILE *fout = (FILE*)data;
	int rcvd_last = *prcvd_last;
	if (fout == stdout) {

	}
	else {
		printf("%d ", rcvd_last);
	}
	fwrite((const void*)buf, 1, rcvd_last, fout);
	//fflush(fout);
	return 1;
}

int sender(int socket, FILE* fin, FILE* fout, int buf_size) {
	int ret = 0;
	char *buf;
	int fp, fs;
	int delta_t;

	buf = (char*)malloc(max(buf_size, 1024));
	if (buf == NULL) {
		printf("Error on malloc() !\n");
		return -1;
	}

	fp = ftell(fin);
	fseek(fin, 0, SEEK_END);
	fs = ftell(fin);
	fs -= fp;
	fseek(fin, fp, SEEK_SET);
	fread((void*)buf, fs, 1, fin);

	ret = sendl(socket, buf, fs, 0);
	if (ret<0) {
		printf("Error on sendl() !\n");
		free(buf);
		return -1;
	}
	printf("sent %d bytes\n", ret);

	printf("receiving...\n");
	printf("\n");
	delta_t = get_ticks_ms();
	ret = recvw(socket, buf, buf_size, recvwcb, (void*)fout);
	if (ret<0) {
		printf("Error on recvw() !\n");
		free(buf);
		return -1;
	}
	delta_t = get_ticks_ms() - delta_t;
	printf("\n");
	printf("received a total of %d bytes in %.3f seconds at %d KB/s\n", ret, (double)delta_t/1000.0, (int)floor((double)ret/(double)delta_t));


	free(buf);

	return ret;
}


int client(char* host_address, int host_port, FILE* fin, FILE* fout, int buf_size) {
	int ret = 0;
	int socket_c;
	struct sockaddr_in sin_c;

	ret = loadsocklib();
	if (ret < 0)
		return -1;

	socket_c = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_c < 0) {
		unloadsocklib();
		return -1;
	}

	sin_c.sin_family = AF_INET;
	sin_c.sin_port = htons((unsigned short)host_port);
	hnametoipv4(host_address, &sin_c.sin_addr);

	printf("Connection to %s:%d\n", host_address, host_port);

	ret = connect(socket_c, (const struct sockaddr*)&sin_c, sizeof(sin_c));
	if (ret < 0) {
		printf("connect() failed !\n");
		sclose(socket_c);
		unloadsocklib();
		return -1;
	}
	printf("Connection successful !\n");

	ret = sender(socket_c, fin, fout, buf_size);

	printf("Disconnecting...\n");

	sclose(socket_c);
	unloadsocklib();

	return ret;
}

int main(int argc, char** argv) {
	int ret = 0;
	int i;
	FILE *fin;
	FILE *fout;
	int buf_size;

	char *address;
	int port;

	// Default parameters under any circumstances
	port = 80;
	fout = stdout;
	fin = stdin;
	buf_size = 1*1024*1024;

	if(argc <= 1){
		printf(" usage: %s [-a address] [-p port] [-o output file] [-i input file] [-b buffer size (def=1 MB)]\n", argv[0]);
		ret = -1;
		goto BACK;
	}

	for(i = 1; i < argc; i++){
		if (!strcmp(argv[i], "-a")) {
			i += 1;
			address = argv[i];
		}
		else if (!strcmp(argv[i], "-p")) {
			i += 1;
			port = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-b")) {
			i += 1;
			buf_size = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-o")) {
			i += 1;
			fout = fopen(argv[i], "wb");
			if (fout == NULL) {
				printf("%s could not be opened for writing\n", argv[i]);
				fout = stdout;
			}
		}
		else if (!strcmp(argv[i], "-i")) {
			i += 1;
			fin = fopen(argv[i], "rb");
			if (fin == NULL) {
				printf("%s could not be opened for reading\n", argv[i]);
				fin = stdin;
			}
		}
		else {
			printf("bad option %s\n", argv[i]);
			ret = -1;
			goto BACK;
		}
	}



	client(address, port, fin, fout, buf_size);
	if (fin != stdin)
		fclose(fin);
	if (fout != stdout)
		fclose(fout);

	BACK:
	return ret;
}

