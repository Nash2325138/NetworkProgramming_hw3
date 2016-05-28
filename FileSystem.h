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
	FileSystem() {}
	~FileSystem() {}
	void update(std::set<User *> &onlineUsers)
	{
		for(auto iter = files.begin(), end = files.end() ; iter != end ; iter++) delete *iter;
		files.clear();
		for(auto iter = onlineUsers.begin(), end = onlineUsers.end() ; iter != end ; iter++) {
			for(auto fileIter = (*iter)->fileList.begin() , fileEnd = (*iter)->fileList.end() ; fileIter != fileEnd ; fileIter++) {
				files.insert( new FileInfo(fileIter->c_str()) );
			}
		}
	}
	void catFiles(char *sendline, std::set<User *> &onlineUsers)
	{
		char temp[4000];
		strcat(sendline, SGR_GRN_BOLD "\tServer:\n" SGR_YEL);
		for(auto iter = files.begin(), end = files.end() ; iter != end ; iter++) {
			sprintf(temp, "\t\t%s\n", (*iter)->name.c_str());
			strcat(sendline, temp);
		}
		strcat(sendline, "\n");
		for(auto userIter = onlineUsers.begin(), userEnd = onlineUsers.end() ; userIter != userEnd ; userIter++) {
			sprintf(temp, SGR_GRN_BOLD "\t%s:\n" SGR_YEL, (*userIter)->account);
			strcat(sendline, temp);
			for(auto fileIter = (*userIter)->fileList.begin() , fileEnd = (*userIter)->fileList.end() ; fileIter != fileEnd ; fileIter++ ) {
				sprintf(temp, "\t\t%s\n", fileIter->c_str());
				strcat(sendline, temp);
			}
			strcat(sendline, "\n");
		}

		strcat(sendline, SGR_RESET);
	}
};

#endif