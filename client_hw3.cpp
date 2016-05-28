#include "NP_necessary.h"
#include <thread>

#define MAXLINE 2048
#define SHOW_PORT 7000
#define CHAT_PORT 7878
#define LISTEN_Q 1024

void hw3_client(FILE *fp, int ctrlfd);
ssize_t writen(int fd, const void *tosend, size_t n);
int create_listenfd(int port);
static void * show_thread(void *arg);

void chat_creator(const char *peerAccount, bool *isChatting);
void chat_connector(const char *IP, const char *peerAccount, bool *isChatting);
void simpleChat(int peerfd, const char *peerAccount);

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
	bool isChatting = false;

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
			if( isChatting == false ) {
				if( fgets(sendline, MAXLINE, fp) == NULL ) break;
				writen(ctrlfd, sendline, strlen(sendline));
			}
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
				char peerAccount[100];
				sscanf(recvline, "%*s %s", peerAccount);
				isChatting = true;
				std::thread (chat_creator, peerAccount, &isChatting).detach();
				//chat_creator(peerAccount, &isChatting);
			} else if(strcmp(command, "Connect_Chat") == 0) {
				char address[100];
				char peerAccount[100];
				sscanf(recvline, "%*s %s %s", address, peerAccount);
				isChatting = true;
				std::thread (chat_connector, address, peerAccount, &isChatting).detach();
				//chat_connector(address, peerAccount, &isChatting);
			}
		}
	}

}

void chat_creator(const char *peerAccount, bool *isChatting)
{
	int chatListenfd = create_listenfd(CHAT_PORT);
	listen(chatListenfd, LISTEN_Q);

	sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);

	int peerfd = accept(chatListenfd, (struct sockaddr *)&peeraddr, &peerlen);
	close(chatListenfd);

	simpleChat(peerfd, peerAccount);
	*isChatting = false;
	close(peerfd);
}
void chat_connector(const char *ip_v4, const char *peerAccount, bool *isChatting)
{
	sockaddr_in peeraddr;
	fillInfo(&peeraddr, CHAT_PORT, ip_v4);

	int peerfd;
	printf("ip_v4 : %s\n", ip_v4);
	if( (peerfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) perror("socket error");
	while( connect(peerfd, (struct sockaddr *)&peeraddr, sizeof(peeraddr)) < 0) {
		fprintf(stdout, "Waiting...\n");
		sleep(1);
	}
	simpleChat(peerfd, peerAccount);
	*isChatting = false;
	close(peerfd);
}
void simpleChat(int peerfd, const char *pa)
{
	setbuf(stdin, NULL);
	char peerAccount[100];
	strcpy(peerAccount, pa);
	fprintf(stdout, "     Chat start!\n");
	char sendline[MAXLINE*100];
	char recvline[MAXLINE+1];

	fd_set rset, allset;
	FD_ZERO(&allset);
	FD_SET(peerfd, &allset);
	FD_SET(STDIN_FILENO, &allset);
	for( ; ; ) {
		rset = allset;
		int maxfd = (peerfd > STDIN_FILENO) ? peerfd : STDIN_FILENO;
		select(maxfd+1, &rset, NULL, NULL, NULL);

		if(FD_ISSET(STDIN_FILENO, &rset)) { // stdin has something to read
			if( fgets(sendline, MAXLINE, stdin) == NULL ) break;
			writen(peerfd, sendline, strlen(sendline));
		}
		if(FD_ISSET(peerfd, &rset)) {
			int n = read(peerfd, recvline, MAXLINE);
			if(n <= 0) break;
			recvline[n] = '\0';
			fprintf(stdout, "   %s: %s", peerAccount, recvline);
		}
	}
	fprintf(stdout, "======= Chat is terminated! =======\n");
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
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &servaddr_in, sizeof(servaddr_in));
	if ( bind(listenfd, (struct sockaddr *)&servaddr_in, sizeof(servaddr_in)) < 0) perror("bind error");
	return listenfd;
}
void fillInfo(struct sockaddr_in *servaddr, int port, const char *ip_v4)
{
	memset(servaddr, 0, sizeof(sockaddr_in));
	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(port);
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