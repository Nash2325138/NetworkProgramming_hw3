#ifndef USER_H
#define USER_H

typedef enum {
	OFFLINE,
	ONLINE
} UserState;

#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <iostream>

#define MAXLINE 2048
extern char wellcomeString[MAXLINE*20];
extern ssize_t writen(int fd, const void *tosend, size_t n);
class User;
extern std::set<User *> onlineUsers;

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
	std::vector< std::pair<std::string, long long> > fileList;

	User(char *account, char *password)
	{
		pthread_mutex_init(&showfd_mutex, NULL);
		pthread_mutex_init(&ctrlfd_mutex, NULL);
		strcpy(this->account, account);
		strcpy(this->password, password);
		password[0] = '\0';
		state = OFFLINE;
		fileList.clear();
	}
	~User()
	{

	}
	void logIn(int ctrlfd, int showfd, struct sockaddr_in * _cliaddr_in)
	{
		pthread_mutex_init(&showfd_mutex, NULL);
		pthread_mutex_init(&ctrlfd_mutex, NULL);
		this->ctrlfd = ctrlfd;
		this->showfd = showfd;
		this->state = ONLINE;
		this->addr_in = (* _cliaddr_in);
		onlineUsers.insert(this);
	}
	void logOut()
	{
		pthread_mutex_init(&showfd_mutex, NULL);
		pthread_mutex_init(&ctrlfd_mutex, NULL);
		this->state = OFFLINE;
		onlineUsers.erase(this);
	}
	void write_to_showfd(const char *sendBuffer)
	{
		pthread_mutex_lock(&showfd_mutex);
		writen(showfd, sendBuffer, strlen(sendBuffer));
		pthread_mutex_unlock(&showfd_mutex);
	}
	void write_to_ctrlfd(const char *sendBuffer)
	{
		pthread_mutex_lock(&ctrlfd_mutex);
		writen(ctrlfd, sendBuffer, strlen(sendBuffer));
		pthread_mutex_unlock(&ctrlfd_mutex);
	}
	void update_files(char *recvline)
	{
		fileList.clear();
		char num[10];
		sscanf(recvline, "%s", num);
		char *position = recvline + (strlen(num) + 1);
		char getName[100];
		char getSize[50];
		for(int i=0 ; i<atoi(num) ; i++) {
			sscanf(position, "%s %s", getName, getSize);
			fileList.push_back( std::pair<std::string, long long>(std::string(getName), atoll(getSize)) );
			position += (strlen(getName) + 1 + strlen(getSize) + 1);
		}
		std::cout << "User " << this->account << " has files:" << std::endl;
		for(auto iter = fileList.begin(), end = fileList.end() ; iter != end ; iter++) {
			printf("\t%-20s  (%lld bytes)\n", iter->first.c_str(), iter->second);
		}
	}
	void getIP(char *buffer)
	{
		inet_ntop(AF_INET, &this->addr_in.sin_addr, buffer, INET_ADDRSTRLEN);
	}
	int getPort()
	{
		return (int)(ntohs(addr_in.sin_port));
	}
};

#endif