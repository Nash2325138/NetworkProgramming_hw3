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

#define SGR_RED_BOLD "\x1b[;31;1m"
#define SGR_BLU_BOLD "\x1b[;34;1m"
#define SGR_YEL_BOLD "\x1b[;33;1m"
#define SGR_GRN_BOLD "\x1b[;32;1m"
#define SGR_CYAN_BOLD_ITALIC "\x1b[;36;1;3m"

#define SGR_RED "\x1b[;31m"
#define SGR_BLU "\x1b[;34m"
#define SGR_YEL "\x1b[;33m"
#define SGR_GRN "\x1b[;32m"
#define SGR_CYAN "\x1b[;36m"

#define SGR_RESET "\x1b[0;m"