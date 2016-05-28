#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "User.h"
#include <set>
#include <string>

class FileSystem
{
private:
	class FileInfo {
	public:
		std::string name;
		std::set<User *> owners;
		FileInfo(const std::string &cppName) : name(cppName) {}
		FileInfo(const char *cName) : name(cName) {}
	};
	std::set<FileInfo *>files;
public:
	FileSystem();
	~FileSystem();
	void update(std::set<User *> &onlineUsers)
	{

	}
	void catFiles(char *sendline)
	{
		
	}
};

#endif