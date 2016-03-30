#include "bs.h"

int sender(int socket) {
	int ret = 0;
	char *buf;
	int buf_size;

	buf_size = 65536;
	buf = (char*)malloc(buf_size);
	if (buf == NULL) {
		printf("Error on malloc() !\n");
		return -1;
	}


	snprintf(buf, buf_size, "GET / HTTP/1.1\r\n\r\n");
	ret = sendsm(socket, buf);
	printf("Sent %d bytes !\n", ret);

	ret = recvsm(socket, buf, buf_size);
	printf("Received %d bytes ! Message:\n%s\n", ret, buf);


	free(buf);

	return ret;
}

int receiver(int socket) {
	int ret = 0;
	char *buf;
	int buf_size;

	buf_size = 65536;
	buf = (char*)malloc(buf_size);
	if (buf == NULL) {
		printf("Error on malloc() !\n");
		return -1;
	}


	ret = recvsm(socket, buf, buf_size);
	printf("Received %d bytes ! Message:\n%s\n", ret, buf);


	free(buf);

	return ret;
}


int server(char* host_address, int host_port, int is_sender) {
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
	hnametoipv4(host_address, &sin_s.sin_addr);

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

	ipv4tostr(&sin_c.sin_addr, client_ip, 16);
	client_port = (int)ntohs(sin_c.sin_port);

	printf("Client connected ! from %s:%d\n", client_ip, client_port);

	if (is_sender) {
		ret = sender(socket_c);
	}
	else {
		ret = receiver(socket_c);
	}

	sclose(socket_c);
	printf("Shutting down server...\n");
	sclose(socket_s);
	unloadsocklib();
	return ret;
}

int client(char* host_address, int host_port, int is_sender) {
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

	if (is_sender) {
		ret = sender(socket_c);
	}
	else {
		ret = receiver(socket_c);
	}

	printf("Disconnecting...\n");
	sclose(socket_c);
	unloadsocklib();
	return ret;
}

int main(int argc, char** argv) {
	int ret = 0;
	int i;

	int is_server;
	int is_sender;
	char *address;
	int port;

	// Default parameters under any circumstances
	port = 12345;


	if(argc <= 1){
		printf(" usage: %s <-s (server) / -c (client)> [-a address] [-p port] [-i (sender) / -o (receiver)]\n", argv[0]);
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

	for(i = 2; i < argc; i++){
		if (!strcmp(argv[i], "-a")) {
			i += 1;
			address = argv[i];
		}
		else if (!strcmp(argv[i], "-p")) {
			i += 1;
			port = atoi(argv[i]);
		}
		else {
			printf("bad option %s\n", argv[i]);
			ret = -1;
			goto BACK;
		}
	}



	if (is_server) {
		server(address, port, is_sender);
	}
	else {
		client(address, port, is_sender);
	}


	BACK:
	return ret;
}

