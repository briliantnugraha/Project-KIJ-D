#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BACKLOG 10
#define MAX_NAMELEN 50
#define len 2048

struct klien{
	char nama[MAX_NAMELEN];
	int sock;
};

int main (int argc, char* argv[]) {
	if (argc < 2) {
		printf("Penggunaan: server port");
		exit(-1);	
	}
	struct klien client;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	struct addrinfo hints, *res;
	int sockfd, new_fd,a;
	int status, bytes_sent, bytes_recv;

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
	
	while(1)
	{
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size);	
		client.sock = new_fd;
		printf("Client %d konek\n",client.sock);
		char msg[len]={};

		while(bytes_recv = recv(client.sock, msg, len, 0))
		{
			bytes_sent = send(client.sock, msg,len,0);
			printf("Ini isi sent: ");
			for(a=0; a<len; a++)printf("%c",msg[a]);
			printf("\n\n");
			if (new_fd < 0) {
				printf("Gagal saat accept!");
				exit (-1);
			}	
		}	
		printf("Berhasil bung!");
	
		close(socket);

	}
	
}
