#include "NP_necessary.h"

#define LISTEN_Q 1024
#define MAXLINE 2048
#define SHOW_PORT 7000

static void *thread_function(void *arg);
void hw3_service(int connfd, int showfd, sockaddr_in cliaddr_in);

typedef struct Connect_info {
	int *connfd;
	int *showfd;
	sockaddr_in *cliaddr_in;
	Connect_info(int *_connfd, int *_showfd, struct sockaddr_in *_cliaddr_in) {
		connfd = _connfd;
		showfd = _showfd;
		cliaddr_in = _cliaddr_in;
	}
}Connect_info;

char wellcomeString[MAXLINE*20];
void initial_string()
{
	strcpy(wellcomeString, "");
	FILE * ascii = fopen("wellcome_ASCII.txt", "r");
	char buffer[1024];
	while( fgets(buffer, 1024, ascii) != NULL ){
		strcat(wellcomeString, buffer);
	}
	fclose(ascii);
}
int create_listenfd(int port);
int main(int argc, char const *argv[])
{
	if(argc != 2) {
		fprintf(stderr, "Usage: ./<execute> <port>\n");
		exit(EXIT_FAILURE);
	}
 	//printf("!");
	struct sockaddr_in cliaddr_in;
	

 	//printf("!");
	int ctrl_listenfd = create_listenfd(atoi(argv[1]));
	int show_listenfd = create_listenfd(SHOW_PORT);
	listen(ctrl_listenfd, LISTEN_Q);
	listen(show_listenfd, LISTEN_Q);

 	//printf("!");
	for( ; ; ) {
		socklen_t clilen = sizeof(cliaddr_in);
		int *ctrlfd_ptr = (int *) malloc(sizeof(int));
		int *showfd_ptr = (int *) malloc(sizeof(int));
		*ctrlfd_ptr = accept(ctrl_listenfd, (struct sockaddr *)&cliaddr_in, &clilen); // will block until some client create connect
		*showfd_ptr = accept(show_listenfd, (struct sockaddr *)&cliaddr_in, &clilen);

		char cliAddrStr[INET_ADDRSTRLEN];
		if( inet_ntop(AF_INET, &cliaddr_in.sin_addr, cliAddrStr, INET_ADDRSTRLEN) == NULL ) perror("inet_ntop error");
		printf("Connection from %s, port: %d\n", cliAddrStr, ntohs(cliaddr_in.sin_port));

		pthread_t tid;
		Connect_info * info = new Connect_info(ctrlfd_ptr, showfd_ptr, &cliaddr_in);
		if( pthread_create(&tid, NULL, thread_function, info) != 0) fprintf(stderr, "pthread_create error.\n");
	}


	return 0;
}

static void * thread_function(void *arg)
{
	Connect_info *info =  (Connect_info *)arg ;
	int connfd = *(info->connfd);
	int showfd = *(info->showfd);
	sockaddr_in cliaddr_in = *(info->cliaddr_in);
	free(info->connfd);
	free(info->showfd);

	pthread_detach(pthread_self());
	hw3_service(connfd, showfd, cliaddr_in);
	
	char cliAddrStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &cliaddr_in.sin_addr, cliAddrStr, INET_ADDRSTRLEN);	
	printf("%s, port: %d exit\n", cliAddrStr, ntohs(cliaddr_in.sin_port));
	
	close(connfd);
	return NULL;
}

void hw3_service(int ctrlfd, int showfd, struct sockaddr_in cliaddr_in)
{
	ssize_t n;
	char recvline[MAXLINE];

	while ( (n = read(ctrlfd, recvline, MAXLINE)) > 0) {
		recvline[n] = '\0';
		printf("receive: %s", recvline);
		write(showfd, recvline, n);
	}
}


// Write "n" bytes to a descriptor.
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