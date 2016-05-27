#include "NP_necessary.h"
#include "User.h"
#include <map>
#include <set>

#define LISTEN_Q 1024
#define MAXLINE 2048
#define SHOW_PORT 7000

std::map<std::string, User *> accountMap;
std::set<User *> onlineUsers;
pthread_mutex_t onlineUsers_mutex;
void catOnlineUsers(char *sendline);

static void *thread_function(void *arg);
void hw3_service(int connfd, int showfd, struct sockaddr_in cliaddr_in);
ssize_t writen(int fd, const void *tosend, size_t n);
bool read_safe(int fd, char *recvline, int max);

typedef struct Connect_info {
	int connfd;
	int showfd;
	sockaddr_in cliaddr_in;
	Connect_info(int _connfd, int _showfd, struct sockaddr_in *_cliaddr_in) {
		connfd = _connfd;
		showfd = _showfd;
		cliaddr_in = *_cliaddr_in;
	}
}Connect_info;

char wellcomeString[MAXLINE*20];
char mainMenuString[MAXLINE];
char Update_file_info_string[50];
void initial()
{
	pthread_mutex_init(&onlineUsers_mutex, NULL);

	strcpy(wellcomeString, "");
	FILE * ascii = fopen("wellcome_ASCII.txt", "r");
	char buffer[1024];
	while( fgets(buffer, 1024, ascii) != NULL ){
		strcat(wellcomeString, buffer);
		//printf("%s", buffer);
	}
	fclose(ascii);

	strcpy(mainMenuString, "------------------------------------\n");
	strcat(mainMenuString, "  [SO]Show Others\n  [C]hat with <account>\n  [D_sure]Delete this account\n  [L]ogout\n  [H]elp\n");
	strcat(mainMenuString, "------------------------------------\n");

	strcpy(Update_file_info_string, "Update_file_info");
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
	initial();

 	//printf("!");
	int ctrl_listenfd = create_listenfd(atoi(argv[1]));
	int show_listenfd = create_listenfd(SHOW_PORT);
	listen(ctrl_listenfd, LISTEN_Q);
	listen(show_listenfd, LISTEN_Q);

 	//printf("!");
	for( ; ; ) {
		socklen_t clilen = sizeof(cliaddr_in);
		int ctrlfd = accept(ctrl_listenfd, (struct sockaddr *)&cliaddr_in, &clilen); // will block until some client create connect
		int showfd = accept(show_listenfd, (struct sockaddr *)&cliaddr_in, &clilen);

		char cliAddrStr[INET_ADDRSTRLEN];
		if( inet_ntop(AF_INET, &cliaddr_in.sin_addr, cliAddrStr, INET_ADDRSTRLEN) == NULL ) perror("inet_ntop error");
		printf("Connection from %s, port: %d\n", cliAddrStr, ntohs(cliaddr_in.sin_port));

		pthread_t tid;
		Connect_info * info = new Connect_info(ctrlfd, showfd, &cliaddr_in);
		if( pthread_create(&tid, NULL, thread_function, info) != 0) fprintf(stderr, "pthread_create error.\n");
	}


	return 0;
}

static void * thread_function(void *arg)
{
	Connect_info *info =  (Connect_info *)arg ;

	pthread_detach(pthread_self());
	hw3_service(info->connfd, info->showfd, info->cliaddr_in);
	
	char cliAddrStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &info->cliaddr_in.sin_addr, cliAddrStr, INET_ADDRSTRLEN);	
	printf("%s, port: %d exit\n", cliAddrStr, ntohs(info->cliaddr_in.sin_port));
	
	close(info->connfd);
	close(info->showfd);
	delete info;
	return NULL;
}

void hw3_service(int ctrlfd, int showfd, struct sockaddr_in cliaddr_in)
{
	char recvline[MAXLINE+1];
	char sendline[MAXLINE*200];

	// login
	char account[200];
	char password[200];
	std::string cppAccount;
	// set no delay
	//int flag = 1; 
	//setsockopt(showfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
	bool loginSuccess = false;
	while( !loginSuccess )
	{
		// request user's account
		strcpy(sendline, "Account(enter \"new\" to register): ");
		writen(showfd, sendline, strlen(sendline));
		if( !read_safe(ctrlfd, recvline, MAXLINE) ) return;
		sscanf(recvline, " %s", account);

		if(strcmp(account, "new") == 0) {
			for( ; ; ) {
				strcpy(sendline, "New account: ");
				writen(showfd, sendline, strlen(sendline));
				if( !read_safe(ctrlfd, recvline, MAXLINE) ) return;
				sscanf(recvline, " %s", account);

				cppAccount.assign(account);
				if( accountMap.find(cppAccount) == accountMap.end() ) {
					strcpy(sendline, "---------- Account available ----------\nNew password: ");
					writen(showfd, sendline, strlen(sendline));
					if( !read_safe(ctrlfd, recvline, MAXLINE) ) return;
					sscanf(recvline, " %s", password);
					accountMap.insert( std::pair<std::string, User *>(cppAccount, new User(account, password)) );
					// or use : accountMap[cppAccount] = new User(account, password);
					loginSuccess = true;
					accountMap.at(cppAccount)->logIn(ctrlfd, showfd, &cliaddr_in);
					break;
				} else {
					strcpy(sendline, "---------- Account used ! ----------\n");
					writen(showfd, sendline, strlen(sendline));
					continue;
				}
			}
		}
		else {
			cppAccount.assign(account);
			// request user's password
			strcpy(sendline, "Password: ");
			writen(showfd, sendline, strlen(sendline));
			if( !read_safe(ctrlfd, recvline, MAXLINE) ) return;
			sscanf(recvline, " %s", password);
			if(accountMap.find(cppAccount) == accountMap.end()) {
				strcpy(sendline, "No such account\n");
			} else if( strcmp(accountMap.at(cppAccount)->password, password) != 0) {
				strcpy(sendline, "Password not fit\n");
			} else {
				strcpy(sendline, "Login success!\n");
				accountMap.at(cppAccount)->logIn(ctrlfd, showfd, &cliaddr_in);
				loginSuccess = true;
			}
			writen(showfd, sendline, strlen(sendline));
		}
			
	}
	// reset no delay
	//flag = 0; 
	// setsockopt(showfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));

	accountMap.at(cppAccount)->write_to_showfd(wellcomeString);
	accountMap.at(cppAccount)->write_to_ctrlfd(Update_file_info_string);
	read_safe(ctrlfd, recvline, MAXLINE);
	//printf("recvline: %s", recvline);
	accountMap.at(cppAccount)->update_files(recvline);

	char command[MAXLINE];
	accountMap.at(cppAccount)->write_to_showfd(mainMenuString);
	// [SO]Show Others  [C]hat with <account>  [D_sure]Delete this account  [L]ogout  [H]elp
	while( read_safe(ctrlfd, recvline, MAXLINE))
	{
		sscanf(recvline, "%s", command);
		if(strcmp(command, "SO") == 0) {
			sendline[0] = '\0';
			catOnlineUsers(sendline);
			accountMap.at(cppAccount)->write_to_showfd(sendline);
		} else if(strcmp(command, "C") == 0) {
			char target[100];
			sscanf(recvline, "%*s %s", target);
			std::string cppTarget(target);
			if(accountMap.find(cppTarget) == accountMap.end()) {
				sprintf(sendline, "   No such account: %s\n", target);
				accountMap.at(cppAccount)->write_to_showfd(sendline);
			} else if(accountMap.at(cppTarget)->state == OFFLINE) {
				sprintf(sendline, "   This account is offline\n");
				accountMap.at(cppAccount)->write_to_showfd(sendline);
			} else {
				sprintf(sendline, "   Establishing connection between %s and %s...", cppAccount.c_str(), target);
				accountMap.at(cppAccount)->write_to_showfd(sendline);
				
				sprintf(sendline, "Listen_Chat %s", target);
				accountMap.at(cppAccount)->write_to_ctrlfd(sendline);

				char temp[200];
				accountMap.at(cppAccount)->getIP(temp);
				sprintf(sendline, "Connect_Chat %s %s", temp, cppAccount.c_str());
				accountMap.at(cppTarget)->write_to_ctrlfd(sendline);
			}
		} else if(strcmp(command, "D_sure") == 0) {

		} else if(strcmp(command, "L") == 0) {
			accountMap.at(cppAccount)->logOut();
			return;
		} else if(strcmp(command, "H") == 0) {
			accountMap.at(cppAccount)->write_to_showfd(mainMenuString);
		}
	}

	accountMap.at(cppAccount)->logOut();
}

bool read_safe(int fd, char *recvline, int max)
{
	ssize_t n;
	n = read(fd, recvline, max);
	if( n <= 0) {
		recvline[n] = '\0';
		perror("read error");
		return false;
	}
	return true;
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

void catOnlineUsers(char *sendline)
{
	char temp[100];
	strcat(sendline, "Online users:\n");
	pthread_mutex_lock(&onlineUsers_mutex);
	for(std::set<User *>::iterator iter = onlineUsers.begin() ; iter != onlineUsers.end() ; iter++) {
		sprintf(temp, "%10s |", (*iter)->account);
		strcat(sendline, temp);
	}
	strcat(sendline, "\n");
	pthread_mutex_unlock(&onlineUsers_mutex);
}