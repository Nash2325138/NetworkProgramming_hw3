#include "NP_necessary.h"

#define MAXLINE 2048
#define SHOW_PORT 7000
#define CHAT_PORT 7878
#define LISTEN_Q 1024

void hw3_client(FILE *fp, int ctrlfd);
ssize_t writen(int fd, const void *tosend, size_t n);
int create_listenfd(int port);
static void * show_thread(void *arg);

void chat_creator();
void char_connector();

void sprintFiles(char *sendline);
void fillInfo(struct sockaddr_in *servaddr, int port, const char *ip_v4);

int main(int argc, char const *argv[])
{
	setbuf(stdout, NULL);
	if(argc != 3) {
		fprintf(stderr, "Usage: ./<execute> <server IP> <port>\n");
	}

	// family, port, address setting
	struct sockaddr_in servaddr;
	fillInfo(&servaddr, atoi(argv[2]), argv[1]);

	// connect a ctrlfd
	int ctrlfd;
	if( (ctrlfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) perror("socket error");
	if( connect(ctrlfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) perror("connet error");
	printf("ctrlfd connects to %s, port: %d\n", argv[1], atoi(argv[2]));



	// family, port, address setting
	fillInfo(&servaddr, SHOW_PORT, argv[1]);

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
	char sendline[MAXLINE*100];
	char recvline[MAXLINE+1];
	int fp_fileno = fileno(fp);
	fd_set rset, allset;
	FD_ZERO(&allset);
	FD_SET(ctrlfd, &allset);
	FD_SET(fp_fileno, &allset);
	for( ; ; ) {
		rset = allset;
		int maxfd = (ctrlfd > fp_fileno) ? ctrlfd : fp_fileno;
		select(maxfd+1, &rset, NULL, NULL, NULL);

		if(FD_ISSET(fp_fileno, &rset)) { // stdin has something to read
			if( fgets(sendline, MAXLINE, fp) == NULL ) break;
			writen(ctrlfd, sendline, strlen(sendline));
		}
		if(FD_ISSET(ctrlfd, &rset)) {
			int n = read(ctrlfd, recvline, MAXLINE);
			if(n <= 0) break;
			recvline[n] = '\0';
			printf("          ctrlfd receive: %s\n", recvline);
			char command[100];
			sscanf(recvline, " %s", command);
			if(strcmp(command, "Update_file_info") == 0) {
				sprintFiles(sendline);
				writen(ctrlfd, sendline, strlen(sendline));
			} else if(strcmp(command, "Listen_Chat") == 0) {
				int chatListenfd = create_listenfd(CHAT_PORT);
				listen(chatListenfd, LISTEN_Q);
			} else if(strcmp(command, "Connect_Chat") == 0) {
				//char address[100];
			}
		}
	}

}

void chat_creator()
{

}
void char_acceptor()
{

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
void fillInfo(struct sockaddr_in *servaddr, int port, const char *ip_v4)
{
	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(port);
	memset(servaddr->sin_zero, 0, sizeof(servaddr->sin_zero));
	if(inet_pton(AF_INET, ip_v4, &servaddr->sin_addr) <= 0) perror("inet_pton error");
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