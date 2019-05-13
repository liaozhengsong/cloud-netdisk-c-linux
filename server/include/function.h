#ifndef FUNCTION_
#define FUNCTION_
#define _XOPEN_SOURCE
#include "typeattr.h"
#include <openssl/md5.h>
#include <mysql/mysql.h>
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
#define DEBUG
typedef struct node
{
    int fd;
    struct node *next;
}node;
typedef struct
{
    pthread_mutex_t mutex;
    node *front;
    node *rear;
    int size;
}thrqueue;
typedef struct 
{
    int len;
    int type;
    char buf[1000];
}pkg;
typedef struct
{
    int fd;
    thrqueue* que;
    pthread_cond_t* pcond;
    pthread_mutex_t* pmutex;
    pthread_mutex_t* psqlmutex;
    char f_path[128];
} arg;
typedef struct
{
    int fd;
    char f_path[128];
} targ;
int login(int,char *,MYSQL *,pthread_mutex_t*);
int signup(int,char *,char *,MYSQL *,pthread_mutex_t*);
int salt(char*, int);
void sighandler(int);
void *pthreadhandler(void *);
int queueini(thrqueue *);
int dequeue(thrqueue *,node *);
int enqueue(thrqueue *,node *);
int pack(pkg*,int*,char*);
int unpack(pkg*,int*,char*);
int recvpkg(int fd, pkg *ad);
int sendpkg(int fd, pkg *ad);
int parsecmd(char *,int len,targ*);
int userlog(int fd,char *opname);
int f_ls(targ*);
int f_cd(char*,targ*);
int f_puts(char *,targ*);
int f_gets(char *,targ*);
int f_remove(char *,targ*);
int f_pwd(targ*);
int sha512(char *newkey,char *key,char *salt);
struct sockaddr_in * tcpConnect(char *ip, char *port,int *fdnet);
#endif
