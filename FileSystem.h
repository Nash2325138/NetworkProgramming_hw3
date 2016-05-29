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
		long long size;
		FileInfo(const std::string &cppName, long long _size) : name(cppName), size(_size) {}
		FileInfo(const char *cName, long long _size) : name(cName), size(_size) {}
		void insertOwner(User *owner) {
			owners.insert(owner);
		}
	};
	std::set<FileInfo *>files;
	
	class FileSendTracker {
	public:
		FileInfo *sentFile;
		User *target;
		FileSendTracker(FileInfo *_in, User *_target) : sentFile(_in), target(_target) {}
		void start() {
			long long startPosition = 0;
			int senderNum = sentFile->owners.size();
			long long partSize = sentFile->size / senderNum;
			char temp[2000];

			while(partSize < 2000) {
				if(senderNum <= 1) break;
				senderNum /= 2;
				partSize = sentFile->size / senderNum;
			}

			char targetIP[100];
			target->getIP(targetIP);

			auto iter = sentFile->owners.begin();
			for(int i=1 ; i<=senderNum ; i++, iter++) {
				long long sendSize = (i == senderNum) ? (sentFile->size - startPosition) : (partSize);
				sprintf(temp, "SendFile %s %s %s %lld %lld", target->account, sentFile->name.c_str(),
															 targetIP, startPosition, sendSize);
				(*iter)->write_to_ctrlfd(temp);
				startPosition += sendSize;
			}
		}
		void suspend() {

		}
		void resume() {

		}
		void terminate() {

		}
	};
	std::vector<FileSendTracker *> trackers;
public:
	FileSystem() {}
	~FileSystem() {}
	void update(std::set<User *> &onlineUsers)
	{
		for(auto iter = files.begin(), end = files.end() ; iter != end ; iter++) delete *iter;
		files.clear();
		for(auto iter = onlineUsers.begin(), end = onlineUsers.end() ; iter != end ; iter++) {
			for(auto fileIter = (*iter)->fileList.begin() , fileEnd = (*iter)->fileList.end() ; fileIter != fileEnd ; fileIter++) {
				bool alreadyIn = false;
				FileInfo *fileInfo;
				for(auto setIter = files.begin(), setEnd = files.end() ; setIter != setEnd ; setIter++) {
					if((*setIter)->name.compare(fileIter->first) == 0) {
						alreadyIn = true;
						fileInfo = (*setIter);
						break;
					}
				}
				if( alreadyIn == false ) {
					fileInfo = new FileInfo(fileIter->first.c_str(), fileIter->second);
					files.insert( fileInfo );
				}
				fileInfo->insertOwner((*iter));
			}
		}
	}
	void catFiles(char *sendline, std::set<User *> &onlineUsers)
	{
		char temp[4000];
		strcat(sendline, SGR_GRN_BOLD "\tServer:\n" SGR_YEL);
		for(auto iter = files.begin(), end = files.end() ; iter != end ; iter++) {
			sprintf(temp, "\t\t%-20s  (%lld bytes)\n", (*iter)->name.c_str(), (*iter)->size);
			strcat(sendline, temp);
		}
		strcat(sendline, "\n");
		for(auto userIter = onlineUsers.begin(), userEnd = onlineUsers.end() ; userIter != userEnd ; userIter++) {
			sprintf(temp, SGR_GRN_BOLD "\t%s:\n" SGR_YEL, (*userIter)->account);
			strcat(sendline, temp);
			for(auto fileIter = (*userIter)->fileList.begin() , fileEnd = (*userIter)->fileList.end() ; fileIter != fileEnd ; fileIter++ ) {
				sprintf(temp, "\t\t%-20s  (%lld bytes)\n", fileIter->first.c_str(), fileIter->second);
				strcat(sendline, temp);
			}
			strcat(sendline, "\n");
		}

		strcat(sendline, SGR_RESET);
	}
	bool hasFileName(char *cName)
	{
		for(auto iter = files.begin(), end = files.end() ; iter != end ; iter++) {
			if((*iter)->name.compare(cName) == 0) return true;
		}
		return false;
	}
	bool transFileTo(char *cName, User *requester)
	{
		auto iter = files.begin();
		for(auto end = files.end() ; iter != end ; iter++) {
			if((*iter)->name.compare(cName) == 0) break;
		}
		if(iter == files.end()) {
			fprintf(stderr, "You can't trans something not in file system\n");
			return false;
		}
		FileInfo *targetFile = (*iter);
		FileSendTracker *tracker = new FileSendTracker(targetFile, requester);
		trackers.push_back(tracker);
		tracker->start();
		return true;
	}
};

#endif