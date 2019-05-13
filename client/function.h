#ifndef FUNCTION_
#define FUNCTION_
#define _XOPEN_SOURCE
#include "typeattr.h"
#include <openssl/md5.h>
#include <errno.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/msg.h>
#include <strings.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#define ARGS_CHECK(argc,val) {if(argc!=val) \
	{printf("error args\n");return -1;}}
#define ERROR_CHECK(ret,retval,funcName) {if(ret==retval) \
	{printf("%d:",__LINE__);fflush(stdout);perror(funcName);return -1;}}
#define THREAD_ERROR_CHECK(ret,funcName) {if(ret!=0) \
	{printf("%s:%s\n",funcName,strerror(ret));return -1;}}
typedef struct 
{
    int len;
    int type;
    char buf[1000];
}pkg;
int pack(pkg*,int*,char*);
int unpack(pkg*,int*,char*);
int sha512(char *newkey,char *key,char *salt);
int f_cd(int fd);
int f_ls(int fd);
int f_puts(int fd,char *file);
int f_gets(int fd,char *file);
int f_remove(int fd, char *file);
int f_pwd(int fd);
int recvpkg(int,pkg*);
int sendpkg(int,pkg*);
int parsecmd(int fd,char *cmd,int *ptype,pkg *ppacket);
int signup(int fd);
int login(int fd);
#endif
