#ifndef USER_H
#define USER_H

typedef enum {
	OFFLINE,
	ONLINE
} UserState;

#include <string>
#include <set>
#include <algorithm>

#define MAXLINE 2048
extern char wellcomeString[MAXLINE*20];
extern ssize_t writen(int fd, const void *tosend, size_t n);

class User
{
public:
	char account[100];
	char password[100];
	UserState state;
	struct sockaddr_in addr_in;
	int showfd, ctrlfd;
	pthread_mutex_t showfd_mutex;
	pthread_mutex_t ctrlfd_mutex;

	User(char *account, char *password)
	{
		pthread_mutex_init(&showfd_mutex, NULL);
		pthread_mutex_init(&ctrlfd_mutex, NULL);
		strcpy(this->account, account);
		strcpy(this->password, password);
		password[0] = '\0';
		state = OFFLINE;
	}
	~User()
	{

	}
	void catWellcomeToBuffer(char *sendBuffer)
	{
		strcat(sendBuffer, wellcomeString);
		strcat(sendBuffer, "Hello ");
		strcat(sendBuffer, account);
		strcat(sendBuffer, "!\n");
	}
	void logIn(int ctrlfd, int showfd, struct sockaddr_in * _cliaddr_in)
	{
		pthread_mutex_init(&showfd_mutex, NULL);
		pthread_mutex_init(&ctrlfd_mutex, NULL);
		this->ctrlfd = ctrlfd;
		this->showfd = showfd;
		this->state = ONLINE;
		this->addr_in = (* _cliaddr_in);
	}
	void logOut()
	{
		pthread_mutex_init(&showfd_mutex, NULL);
		pthread_mutex_init(&ctrlfd_mutex, NULL);
		this->state = OFFLINE;
	}
	void write_to_showfd(char *sendBuffer)
	{
		pthread_mutex_lock(&showfd_mutex);
		writen(showfd, sendBuffer, strlen(sendBuffer));
		pthread_mutex_unlock(&showfd_mutex);
	}
	void write_to_ctrlfd(char *sendBuffer)
	{
		pthread_mutex_lock(&ctrlfd_mutex);
		writen(ctrlfd, sendBuffer, strlen(sendBuffer));
		pthread_mutex_unlock(&ctrlfd_mutex);
	}
};

#endif