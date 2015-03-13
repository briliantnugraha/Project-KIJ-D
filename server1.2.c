#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZ 1080
#define BACKLOG 10
#define MAX_NAMELEN 51
#define len 2048

typedef struct {
	int sock;
} thread_args_t;

typedef struct k {
	char nama[MAX_NAMELEN];
	int sock;
	struct k* next;
} Klien;

Klien *daftarKlien = NULL;

short isAlphaNumeric(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

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
	status = bind(*sockfd, res->ai_addr, res->ai_addrlen);
	if (status) {
		printf("Gagal saat bind!, status: %d\n", status);
		return(-1);
	}
	listen(*sockfd, backlog);
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

	node->sock = 5;
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

void insert(const char *nama, int sock, Klien **node) {
	if (*node == NULL) {
		*node = (Klien*) malloc(sizeof(Klien));
		strcpy((*node)->nama, nama);
		(*node)->sock = sock;
		(*node)->next = NULL;
	}
	else {
		Klien* iterator = *node;
		while (iterator->next != NULL) {
			iterator = iterator->next;
		}
		iterator->next = (Klien*) malloc (sizeof(Klien));
		strcpy(iterator->next->nama, nama);
		iterator->next->sock = sock;
		iterator->next->next = NULL;
	}
}

void delete(const char *nama, Klien **node) {
	if (strcmp((*node)->nama, nama) == 0) {
		Klien* temp = *node;
		*node = (*node)->next;
		free(temp);
	}
	else {
		Klien* iterator = *node;
		while(iterator->next != NULL && strcmp(iterator->next->nama, nama)) {
			iterator = iterator->next;
		}
		if (iterator->next != NULL) {
			Klien* temp = iterator->next->next;
			free(iterator->next);
			iterator->next = temp;
		}
	}
}

void *run(void *t_args) {
	thread_args_t *args = (thread_args_t*) t_args;
	char buf[BUF_SIZ], msg[BUF_SIZ];
	char nama[MAX_NAMELEN], namaPenerima[MAX_NAMELEN];
	int bytes, len1;

	//kirim daftar user
	strcpy(buf, "OK\r\n\0");
	send(args->sock, buf, strlen(buf), 0);
	Klien *iterator = daftarKlien;
	while (iterator != NULL) {
		sprintf(buf, "!CONNECT %s\r\n", iterator->nama);
		send(args->sock, buf, strlen(buf), 0);
		iterator = iterator->next;
	}

	//terima nama
	while ((bytes = recv(args->sock, buf, BUF_SIZ-1, 0)) > 0) {
		//trimming
		int i = 0;
		while (i < bytes && isAlphaNumeric(buf[i]))
			i++;
		buf[i] = 0;
		//selesai trimming
		strncpy(nama, buf, MAX_NAMELEN-1);
		int searchResult = search(nama, daftarKlien);
		if (searchResult == -1) {
			strcpy(buf, "OK\r\n\0");
			send(args->sock, buf, strlen(buf), 0);
			sprintf(buf, "!CONNECT %s\r\n", nama);
			Klien* node = daftarKlien;
			while (node != NULL) {
				send(node->sock, buf, strlen(buf), 0);
				node = node->next;
			}
			insert(nama, args->sock, &daftarKlien);
			break;
		}
		else {
			strcpy(buf, "NOPE\r\n\0");
			send(args->sock, buf, strlen(buf), 0);
			close(args->sock);
			free(args);
			pthread_exit(NULL);
		}
	}

	while ((bytes = recv(args->sock, buf, BUF_SIZ-1, 0)) > 0) {
		buf[bytes] = 0;
		char* titikdua = strchr(buf, ':');
		if (titikdua != NULL) {
			int indexTD = (int)(titikdua - buf);
			if (indexTD <= 50) {
				strncpy(namaPenerima, buf, indexTD);
			}
			strcpy(msg, titikdua+1);

			namaPenerima[indexTD] = 0;
			int searchResult = search(namaPenerima, daftarKlien);
			if (searchResult == -1) {
				sprintf(buf, "NOPE\r\n", namaPenerima);
				send(args->sock, buf, strlen(buf), 0);
			}
			else {
				sprintf(buf, "%s:%s\r\n", nama, msg);
				send(searchResult, buf, strlen(buf), 0);
			}
		}
	}
	close(args->sock);
	delete(nama, &daftarKlien);
	sprintf(buf, "!DISCONNECT %s\r\n", nama);
	Klien* node = daftarKlien;
	while (node != NULL) {
		send(node->sock, buf, strlen(buf), 0);
		node = node->next;
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

	int status;
	int sockfd, new_fd;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;

	start_server(&sockfd, argv[1], BACKLOG);

	while(1)
	{
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size);
		thread_args_t *args = (thread_args_t*) malloc(sizeof(thread_args_t));
		args->sock = new_fd;
		pthread_t thread;
		status = pthread_create(&thread, NULL, run, (void*)args);
		if (status) {
			printf("pthread_create error: %d\n", status);
		}
	}

	pthread_exit(NULL);
}
