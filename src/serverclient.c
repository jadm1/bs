#include "bs.h"

int sender(int socket) {
	int ret = 0;
	char *buf;
	int buf_size;
	int to_s;

	buf_size = 65536;
	buf = (char*)malloc(buf_size);
	if (buf == NULL) {
		printf("Error on malloc() !\n");
		return -1;
	}

	getsockrcvbsize(socket, &to_s);
	printf("network recv buf size: %d\n", to_s);

	sendprintf(&ret, socket, "%s", "Hello !\n");



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


	recvprintf(&ret, socket, 1024);


	free(buf);

	return ret;
}


int server(char* host_address, int host_port, int is_sender) {
	int ret = 0;
	int socket_s, socket_c;
	char client_ip[16];
	int client_port;


	ret = loadsocklib();
	if (ret < 0)
		return -1;

	socket_s = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_s < 0) {
		freesocklib();
		return -1;
	}

	ret = socklisten(socket_s, host_address, host_port, 1);
	if (ret < 0) {
		sockclose(socket_s);
		freesocklib();
		return -1;
	}

	printf("Listening on %s:%d\n", host_address, host_port);

	printf("Accepting connections...\n");

	ret = sockaccept(socket_s, &socket_c, client_ip, &client_port);
	if (ret < 0) {
		sockclose(socket_s);
		freesocklib();
		return -1;
	}

	printf("Client connected ! from %s:%d\n", client_ip, client_port);

	if (is_sender) {
		ret = sender(socket_c);
	}
	else {
		ret = receiver(socket_c);
	}

	sockclose(socket_c);
	printf("Shutting down server...\n");
	sockclose(socket_s);
	freesocklib();
	return ret;
}

int client(char* host_address, int host_port, int is_sender) {
	int ret = 0;
	int socket_c;

	ret = loadsocklib();
	if (ret < 0)
		return -1;

	socket_c = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_c < 0) {
		freesocklib();
		return -1;
	}

	printf("Connection to %s:%d\n", host_address, host_port);

	ret = sockconnect(socket_c, host_address, host_port);
	if (ret < 0) {
		printf("connect() failed !\n");
		sockclose(socket_c);
		freesocklib();
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
	sockclose(socket_c);
	freesocklib();
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
		is_sender = 1;
	}
	else if (!strcmp(argv[1], "-c")) {
		// Default parameters for client
		is_server = 0;
		address = "localhost";
		is_sender = 0;
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

