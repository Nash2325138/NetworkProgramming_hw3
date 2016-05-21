#include "NP_necessary.h"

#define MAXLINE 2048

void hw3_client(FILE *fp, int servfd);
ssize_t writen(int fd, const void *tosend, size_t n);
int create_listenfd(int port);

int main(int argc, char const *argv[])
{
	if(argc != 3) {
		fprintf(stderr, "Usage: ./<execute> <server IP> <port>\n");
	}

	struct sockaddr_in servaddr;
	// family, port, address setting
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	memset(servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));
	if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) perror("inet_pton error");

	int servfd;
	if( (servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) perror("socket error");
	if( connect(servfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) perror("connet error");

	printf("Connect to %s, port: %d\n", argv[1], atoi(argv[2]));
	
	hw3_client(stdin, servfd);

	return 0;
}

void hw3_client(FILE *fp, int servfd)
{

	char recvline[MAXLINE+1];
	char sendline[MAXLINE];
	//pthread_t tid;

	int n;
	while( fgets(sendline, MAXLINE, fp) != NULL ) {
		if( write(servfd, sendline, MAXLINE) < 0) perror("write error");
		if( (n = read(servfd, recvline, MAXLINE) ) > 0 ) {
			recvline[n] = '\0';
			fputs(recvline, stdout);	
		} else {
			perror("read error");
		}
	}
}


ssize_t writen(int fd, const void *tosend, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;
	ptr = (const char *)tosend;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0; // and call write() again
			else
				return (-1); // error
		}
		nleft -= nwritten;
		ptr += nwritten;
 	}
	return (n);
}

int create_listenfd(int port)
{
	int listenfd;
	struct sockaddr_in servaddr_in;

	memset(&servaddr_in, 0, sizeof(servaddr_in));
	servaddr_in.sin_family = AF_INET;
	servaddr_in.sin_addr.s_addr = htonl(INADDR_ANY); // for any interface
	servaddr_in.sin_port = htons(port);

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bind(listenfd, (struct sockaddr *)&servaddr_in, sizeof(servaddr_in));
	return listenfd;
}