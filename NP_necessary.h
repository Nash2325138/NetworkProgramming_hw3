#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/select.h>

#include <sys/stat.h>
#include <dirent.h>
#include <netinet/tcp.h>

#include <pthread.h>