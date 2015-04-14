#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZ 500
#define BACKLOG 10
#define MAX_NAMELEN 51
#define KEY_LEN 256
#define CIPHER_LEN 257

typedef struct {
	int sock;
} thread_args_t;

typedef struct k {
	char nama[MAX_NAMELEN];
	char public_key[KEY_LEN];
	int sock;
	struct k* next;
} Klien;

Klien *daftarKlien = NULL;

short isAlphaNumeric(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

int start_server(int *sockfd, char* port, int backlog) {
	struct addrinfo hints, *res;
	int status;

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

Klien* search(const char *nama, Klien *node) {
	while(node != NULL && strcmp(node->nama, nama)) {
		node = node->next;
	}
	return node;
}

void insert(const char *nama, const char *key, int sock, Klien **node) {
	if (*node == NULL) {
		*node = (Klien*) malloc(sizeof(Klien));
		strcpy((*node)->nama, nama);
		strcpy((*node)->public_key, key);
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
		strcpy(iterator->next->public_key, key);
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
	char buf[BUF_SIZ], msg[BUF_SIZ], temp[BUF_SIZ], key[KEY_LEN];
	char nama[MAX_NAMELEN], namaPenerima[MAX_NAMELEN];
	int bytes, len1;
	int i, j;
	short connected = 0;
//----------------------------------------------------------------
//	Inisialisasi
//----------------------------------------------------------------

	//Kirim pesan OK ketika klien pertama kali konek
	strcpy(buf, "OK");
	send(args->sock, buf, strlen(buf), 0);

	//terima nama
	bytes = recv(args->sock, buf, BUF_SIZ-1, 0);
	buf[bytes] = 0;
	//copy bagian nama ke variabel nama
	i = 0;
	while (i < bytes && isAlphaNumeric(buf[i])) {
		nama[i] = buf[i];
		i++;
	}
	nama[i] = 0;
	i += 1;
	//copy sisanya, masukkan ke key
	strcpy(key, buf+i);
	printf("Dapat nama-key: %s-%s\n", nama, key);
	//cari nama dalam daftar klien
	Klien* searchResult = search(nama, daftarKlien);
	//kalau nama belum ada (belum terpakai)
	if (searchResult == NULL) {
		//kirim OK
		strcpy(buf, "OK");
		send(args->sock, buf, strlen(buf), 0);
		//kirim nama klien yang baru konek ke klien-klien lain
		sprintf(buf, "!CONNECT %s", nama);
		Klien* node = daftarKlien;
		while (node != NULL) {
			send(node->sock, buf, strlen(buf), 0);
			node = node->next;
		}
		//Kirim daftar klien ke klien yang baru konek
		Klien *iterator = daftarKlien;
		while (iterator != NULL) {
			sprintf(buf, "!CONNECT %s", iterator->nama); //!CONNECT <nama klien>
			send(args->sock, buf, strlen(buf), 0);
			iterator = iterator->next;
		}
		//Masukkan klien baru ke daftar klien
		insert(nama, key, args->sock, &daftarKlien);
		connected = 1;
	}
	//kalau sudah ada klien dengan nama == buf, kirim pesan NOPE
	else {
		strcpy(buf, "NOPE");
		send(args->sock, buf, strlen(buf), 0);
		close(args->sock);
		free(args);
		pthread_exit(NULL);
	}

//-------------------------------------------------------------------------
// Inisialisasi selesai
//-------------------------------------------------------------------------
	while((bytes = recv(args->sock, buf, BUF_SIZ-1, 0)) > 0) {
		buf[bytes] = 0;
		printf("%s: %s\n", nama, buf);
		printf("--");
		if(buf[0] == '!') {
			printf("<Command>");
			i = 1;
			j = 0;
			while ((i < bytes) && isAlphaNumeric(buf[i])) {
				temp[j] = buf[i];
				i++;
				j++;
			}
			temp[j] = 0;
			printf(" (%s) ", temp);
			//kalau !GET
			if (strcmp(temp, "GET") == 0) {
				i += 1;
				//--NCAT DEBUGGING BEGIN, HARAP DI-DELETE!!!!
				//	buf[bytes-1] = 0;
				//--NCAT DEBUGGING END
				strcpy(temp, buf+i); //temp akan berisi nama klien yang public keynya di-request
				Klien *k = search(temp, daftarKlien);
				//kalau klien tidak ditemukan
				if (k == NULL) {
					strcpy(buf, "!NONAME");
					send(args->sock, buf, strlen(buf), 0);
					continue;
				}
				//kirim nama klien dan public key-nya
				printf(" GET %s\n", temp);
				sprintf(buf, "!KEYPU %s %s", temp, k->public_key);
				send(args->sock, buf, strlen(buf), 0);
			}
			if (strcmp(temp, "KEYSI") == 0) {
				printf("KEYSI\n");
				i += 1;
				j = 0;
				char sym_key[CIPHER_LEN];
				//ambil nama klien tujuan
				while ((i < bytes) && isAlphaNumeric(buf[i])) {
					temp[j] = buf[i];
					i++;
					j++;
					printf("i: %d, j: %d, buf[%d]: %c", i, j, i, buf[i]);
				}
				temp[j] = 0;
				printf("temp: %s", temp);
				Klien *k = search(temp, daftarKlien);
				if (k == NULL) {
					//kalau kien tidak ditemukan
					continue;
				}
				i += 1;
				strcpy(sym_key, buf+i);
				printf(" KEYSI %s %s\n", temp, sym_key);
				sprintf(buf, "!KEYSI %s %s", nama, sym_key);
				send(k->sock, buf, strlen(buf), 0);
			}
		}
		else {
			printf("<Chat>");
			i = 0;
			while ((i < bytes) && buf[i] != ':') {
				temp[i] = buf[i];
				i++;
			}
			if (i == bytes) { //titik dua tidak ditemukan
				continue;
			}
			temp[i] = 0;
			i += 2;
			strcpy(msg, buf+i);
			Klien *k = search(temp, daftarKlien);
			if (k == NULL) {
				continue;
			}
			sprintf(buf, "%s: %s", nama, msg);
			send(k->sock, buf, strlen(buf), 0);
		}
		printf("END\n");
	}


	//tutup socket
	close(args->sock);
	if (connected) {
		//hapus klien dari daftar klien
		delete(nama, &daftarKlien);
		//kirim pesan !DISCONNECT [nama_klien] ke klien-klien lainnya
		sprintf(buf, "!DISCONNECT %s\r\n", nama);
		Klien* node = daftarKlien;
		while (node != NULL) {
			send(node->sock, buf, strlen(buf), 0);
			node = node->next;
		}
	}
	//cleaning up
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
