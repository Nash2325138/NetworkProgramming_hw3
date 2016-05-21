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

class User
{
public:
	char account[100];
	char password[100];
	UserState state;
	struct sockaddr_in addr_in;

	User(char *account, char *password)
	{
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
	void logIn(struct sockaddr_in * _cliaddr_in)
	{
		this->state = ONLINE;
		this->addr_in = (* _cliaddr_in);
	}
	void logOut()
	{
		this->state = OFFLINE;
	}
};

#endif