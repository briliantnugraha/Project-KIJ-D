#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFSIZ 1080
#define BACKLOG 10
#define MAX_NAMELEN 51
#define len 2048

typedef enum {
	NAME, MESSAGE
} state_t;

typedef struct {
	int sock;
} thread_args_t;

typedef struct k{
	char nama[MAX_NAMELEN];
	int sock;
	k* next;
} Klien;

Klien daftarKlien;

int start_server(int *sockfd, char* port, int backlog) {
	struct addrinfo hints, *res;
	int status, bytes_sent, bytes_recv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(NULL, port, &hints, &res);

	if (status) {
		printf("Gagal saat getaddrinfo!, status: %d\n", status);
		return(-1);
	}

	*sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	status = bind(sockfd, res->ai_addr, res->ai_addrlen);
	if (status) {
		printf("Gagal saat bind!, status: %d\n", status);
		return(-1);
	}
	listen(sockfd, backlog);
	return 0;
}

void populate(Klien *node) {
	node->sock = 2;
	strcpy(node->nama, "Agus");
	node->next = (Klien*) malloc(sizeof(Klien));
	node = node->next;

	node->sock = 3;
	strcpy(node->nama, "Budi");
	node->next = (Klien*) malloc(sizeof(Klien));
	node = node->next;

	node->sock = 4;
	strcpy(node->nama, "Chandra");
	node->next = (Klien*) malloc(sizeof(Klien));
	node = node->next;

	node->sockfd = 5;
	strcpy(node->nama, "Diana");
	node->next = NULL;
}

int search(const char *nama, Klien *node) {
	while(node != NULL && strcmp(node->nama, nama)) {
		node = node->next;
	}
	if (node != NULL)
		return node->sock;
	else return -1;
}

void *run(void *t_args) {
	thread_args_t *args = (thread_args_t*) t_args;
	char buf[BUFSIZ], msg[BUFSIZ];
	char nama[MAX_NAMELEN], namaPenerima[MAX_NAMELEN];
	int bytes;

	//baca nama
	while ((bytes = recv(args->sock, buf, BUFSIZ-1, 0)) > 0) {
		buf[bytes] = 0;
		strncpy(nama, buf, MAX_NAMELEN-1);
		int searchResult = search(nama, &daftarKlien);
		if (searchResult == -1) {
			strcpy(buf, "OK\r\n");
			send(args->sock, buf, sizeof(buf), 0);
			break;
		}
		else {
			strcpy(buf, "NOPE\r\n");
			send(args->sock, buf, sizeof(buf), 0);
		}
	}
	//baca command dan sejenisnya
	while ((bytes = recv(args->sock, buf, BUFSIZ-1, 0)) > 0) {
		buf[bytes] = 0;

	}

	free(args);
	pthread_exit(NULL);
}

int main (int argc, char* argv[]) {
	if (argc < 2) {
		printf("Penggunaan: server port");
		exit(-1);
	}
//--------------------------------------------------
	Klien *node = &daftarKlien;
	populate(node);

	int status;
	int sockfd, new_fd;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;

	start_server(&sockfd, argv[1], BACKLOG);

	while(1)
	{
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size);
		thread_args_t *args = (thread_args_t*) malloc sizeof(thread_args_t);
		args->sock = new_fd;
		run((void*)args);
		close(socket);
	}

	pthread_exit(NULL);
}
