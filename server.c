#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BACKLOG 10

int main (int argc, char* argv[]) {
	if (argc < 2) {
		printf("Penggunaan: server port");
		exit(-1);	
	}
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	struct addrinfo hints, *res;
	int sockfd, new_fd;
	int status;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(NULL, argv[1], &hints, &res);

	if (status) {
		printf("Gagal saat getaddrinfo!, status: %d\n", status);
		exit(-1);
	}

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	status = bind(sockfd, res->ai_addr, res->ai_addrlen);
	if (status) {
		printf("Gagal saat bind!, status: %d\n", status);
		exit (-1);
	}
	listen(sockfd, BACKLOG);

	new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size);	

	if (new_fd < 0) {
		printf("Gagal saat accept!");
		exit (-1);
	}

	printf("Berhasil bung!");
	
	close(socket);
}