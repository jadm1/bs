#include "bs.h"

#ifndef min
#define min(a, b) ((b > a) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((b > a) ? (b) : (a))
#endif

int stcb(int sent_total, int send_size, int elapsed_ms, double speed_Bps, void* tcb_data) {
	printf("sent %.1f/%.1f MB, elapsed time: %.1f s, speed: %.1f MBps\n", (double)sent_total/1000000.0, (double)send_size/1000000.0, (double)elapsed_ms/1000.0, speed_Bps/1000000.0);
	return 0;
}
int sender(int socket, FILE* f, void* query, int query_size, int buf_size, int file_size, int use_len_pfx) {
	int ret = 0;
	int pos;
	int delta_t;

	if (use_len_pfx) {
		if (file_size == 0) {
			pos = ftell(f);
			fseek(f, 0, SEEK_END);
			file_size = ftell(f) - pos;
			fseek(f, pos, SEEK_SET);
		}

		ret = sendint(socket, file_size);
		if (ret < 0) {
			return -1;
		}
	}

	delta_t = get_ticks_ms();
	ret = sendf(socket, f, file_size, buf_size, stcb, 500, NULL);
	if (ret < 0) {
		return -1;
	}
	delta_t = get_ticks_ms() - delta_t;
	printf("total sent: %d bytes in %.1f s at %.1f MBps\n", ret, (double)delta_t/1000.0, (double)ret/(1000.0*(double)delta_t));

	return ret;
}


int rtcb(int rcvd_total, int recv_size, int elapsed_ms, double speed_Bps, void* tcb_data) {
	printf("received %.1f/%.1f MB, elapsed time: %.1f s, speed: %.1f MBps\n", (double)rcvd_total/1000000.0, (double)recv_size/1000000.0, (double)elapsed_ms/1000.0, speed_Bps/1000000.0);
	return 0;
}
int receiver(int socket, FILE* f, void* query, int query_size, int buf_size, int file_size, int use_len_pfx) {
	int ret = 0;
	int delta_t;

	if (query != NULL) {
		ret = sendl(socket, (const void*)query, query_size, 0);
		if (ret < 0) {
			return -1;
		}
		printf("sent %d bytes\n", ret);
	}

	if (use_len_pfx) {
		ret = recvint(socket, &file_size);
		if (ret < 0) {
			return -1;
		}
	}

	delta_t = get_ticks_ms();
	ret = recvf(socket, f, file_size, buf_size, rtcb, 500, NULL);
	if (ret < 0) {
		return -1;
	}
	delta_t = get_ticks_ms() - delta_t;
	printf("total received: %d bytes in %.1f s at %.1f MBps\n", ret, (double)delta_t/1000.0, (double)ret/(1000.0*(double)delta_t));

	return ret;
}


int server(char* host_address, int host_port, int is_sender, FILE* f, void* query, int query_size, int buf_size, int file_size, int use_len_pfx) {
	int ret = 0;
	int socket_s, socket_c;
	struct sockaddr_in sin_s, sin_c;
	unsigned int accept_sockaddr_len;
	char client_ip[16];
	int client_port;


	ret = loadsocklib();
	if (ret < 0)
		return -1;

	socket_s = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_s < 0) {
		unloadsocklib();
		return -1;
	}

	sin_s.sin_family = AF_INET;
	sin_s.sin_port = htons((unsigned short)host_port);
	sin_s.sin_addr.s_addr = hnametoipv4(host_address);

	ret = bind(socket_s, (const struct sockaddr*)&sin_s, sizeof(sin_s));
	if (ret < 0) {
		sclose(socket_s);
		unloadsocklib();
		return -1;
	}

	ret = listen(socket_s, 1);
	if (ret < 0) {
		sclose(socket_s);
		unloadsocklib();
		return -1;
	}

	printf("Listening on %s:%d\n", host_address, host_port);

	printf("Accepting connections...\n");

	accept_sockaddr_len = sizeof(struct sockaddr_in);
	socket_c = accept(socket_s, (struct sockaddr*)&sin_c, &accept_sockaddr_len);
	if (socket_c < 0) {
		printf("Error on accept() !\n");
		sclose(socket_s);
		unloadsocklib();
		return -1;
	}

	ipv4tostr(client_ip, 16, sin_c.sin_addr.s_addr);
	client_port = (int)ntohs(sin_c.sin_port);

	printf("Client connected ! from %s:%d\n", client_ip, client_port);

	if (is_sender) {
		ret = sender(socket_c, f, query, query_size, buf_size, file_size, use_len_pfx);
	}
	else {
		ret = receiver(socket_c, f, query, query_size, buf_size, file_size, use_len_pfx);
	}

	sclose(socket_c);
	printf("Shutting down server...\n");
	sclose(socket_s);
	unloadsocklib();
	return ret;
}

int client(char* host_address, int host_port, int is_sender, FILE* f, void* query, int query_size, int buf_size, int file_size, int use_len_pfx) {
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
	sin_c.sin_addr.s_addr = hnametoipv4(host_address);

	printf("Connection to %s:%d\n", host_address, host_port);

	ret = connect(socket_c, (const struct sockaddr*)&sin_c, sizeof(sin_c));
	if (ret < 0) {
		printf("connect() failed !\n");
		sclose(socket_c);
		unloadsocklib();
		return -1;
	}
	printf("Connection successful !\n");

	if (is_sender) {
		ret = sender(socket_c, f, query, query_size, buf_size, file_size, use_len_pfx);
	}
	else {
		ret = receiver(socket_c, f, query, query_size, buf_size, file_size, use_len_pfx);
	}

	printf("Disconnecting...\n");
	sclose(socket_c);
	unloadsocklib();
	return ret;
}

int main(int argc, char** argv) {
	int ret = 0;
	int i;
	int query_size = 0;
	int buf_size;
	int file_size;
	int use_len_prefix;

	int is_server;
	int is_sender;
	char *address;
	int port;
	void *query = NULL;
	FILE *f, *fquery;

	// Default parameters under any circumstances
	port = 12345;
	f = NULL;
	fquery = NULL;
	query_size = 0;
	buf_size = 1*1024*1024;
	file_size = 0;
	use_len_prefix = 0;

	if(argc <= 3) {
		printf("usage: %s <-s (server) / -c (client)> <-i file to send / -o file to recv> [-a address] [-p port] [-q query file] [-b buffer size] [-l file size] [-x use length prefix]\n", argv[0]);
		ret = -1;
		goto BACK;
	}

	if (!strcmp(argv[1], "-s")) {
		// Default parameters for server
		is_server = 1;
		address = "0.0.0.0";
		is_sender = 0;
	}
	else if (!strcmp(argv[1], "-c")) {
		// Default parameters for client
		is_server = 0;
		address = "localhost";
		is_sender = 1;
	}
	else {
		printf("bad option %s\n", argv[1]);
		ret = -1;
		goto BACK;
	}

	if (!strcmp(argv[2], "-i")) {
		is_sender = 1;
		f = fopen(argv[3], "rb");
		if (f == NULL) {
			ret = -1;
			return ret;
		}
	}
	else if (!strcmp(argv[2], "-o")) {
		is_sender = 0;
		f = fopen(argv[3], "wb");
		if (f == NULL) {
			ret = -1;
			return ret;
		}
	}
	else {
		printf("bad option %s\n", argv[2]);
		ret = -1;
		goto BACK;
	}

	for(i = 4; i < argc; i++) {
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
		else if (!strcmp(argv[i], "-l")) {
			i += 1;
			file_size = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-x")) {
			use_len_prefix = 1;
		}
		else if (!strcmp(argv[i], "-m")) {
			i += 1;
			fquery = fopen(argv[i], "rb");
			if (fquery == NULL) {
				ret = -1;
				return ret;
			}
			fseek(fquery, 0, SEEK_END);
			query_size = ftell(fquery);
			fseek(fquery, 0, SEEK_SET);
			query = malloc(max(query_size, 1024));
			fread((void*)query, query_size, 1, fquery);
			fclose(fquery);
		}
		else {
			printf("bad option %s\n", argv[i]);
			ret = -1;
			goto BACK;
		}
	}



	if (is_server) {
		server(address, port, is_sender, f, query, query_size, buf_size, file_size, use_len_prefix);
	}
	else {
		client(address, port, is_sender, f, query, query_size, buf_size, file_size, use_len_prefix);
	}

	if (query != NULL)
		free(query);
	fclose(f);

	BACK:
	return ret;
}

