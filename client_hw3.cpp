#include "NP_necessary.h"

#define MAXLINE 2048
#define SHOW_PORT 7000

void hw3_client(FILE *fp, int ctrlfd);
ssize_t writen(int fd, const void *tosend, size_t n);
int create_listenfd(int port);
static void * show_thread(void *arg);

int main(int argc, char const *argv[])
{
	setbuf(stdout, NULL);
	if(argc != 3) {
		fprintf(stderr, "Usage: ./<execute> <server IP> <port>\n");
	}

	// family, port, address setting
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	memset(servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));
	if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) perror("inet_pton error");

	// connect a ctrlfd
	int ctrlfd;
	if( (ctrlfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) perror("socket error");
	if( connect(ctrlfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) perror("connet error");
	printf("ctrlfd connects to %s, port: %d\n", argv[1], atoi(argv[2]));



	// family, port, address setting
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SHOW_PORT);
	memset(servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));
	if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) perror("inet_pton error");

	// connect a showfd
	int showfd;
	if( (showfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) perror("socket error");
	if( connect(showfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) perror("connet error");
	printf("showfd connects to %s, port: %d\n", argv[1], SHOW_PORT);


	// create show thread
	pthread_t tid;
	if( pthread_create(&tid, NULL, show_thread, &showfd) != 0) fprintf(stderr, "pthread_create error.\n");

	// ctrl thread (just use main thread)
	hw3_client(stdin, ctrlfd);

	return 0;
}
void hw3_client(FILE *fp, int ctrlfd)
{
	//char recvline[MAXLINE+1];
	char sendline[MAXLINE*100];
	
	while( fgets(sendline, MAXLINE, fp) != NULL ) {
		writen(ctrlfd, sendline, strlen(sendline));
	}
}

/* use thread to receive ctrlfd's message ? Or I can use select ?? */
static void * receive_ctrl_thread(void *arg)
{
	pthread_detach(pthread_self());
	int ctrlfd = *((int *)arg);

	char recvline[MAXLINE+1];
	ssize_t n;
	while( (n = read(ctrlfd, recvline, MAXLINE)) > 0) {
		recvline[n] = '\0';
		char command[100];
		sscanf(recvline, " %s", command);
		if(strcmp(command, "Update_file_info") == 0) {

		}
	}

	return NULL;
}

static void * show_thread(void *arg)
{
	pthread_detach(pthread_self());
	int showfd = *((int *)arg);

	char recvline[MAXLINE+1];
	int n;
	while( (n = read(showfd, recvline, MAXLINE) ) > 0 ) {
		recvline[n] = '\0';
		fputs(recvline, stdout);
	}
	return NULL;
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

void sprintFiles(char *sendline)
{
	DIR *dir;
	if( (dir = opendir("."))==NULL ){
		perror("opendir in listdir()");
		return;
	}
    
	struct dirent *entry;
	char sendBuffer[MAXLINE];
    int count = 0;
    sendBuffer[0] = '\0';

	entry = readdir(dir);
	while(entry!=NULL)
	{
		if(entry->d_type == DT_DIR);
		else {
			strcat(sendBuffer, " ");
	 		strcat(sendBuffer, entry->d_name);
	 		count++;
		}
		entry = readdir(dir);
	}
	sprintf(sendline, "%d %s", count, sendBuffer);
	return;
}