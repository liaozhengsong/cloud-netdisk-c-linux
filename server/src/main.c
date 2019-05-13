#include "../include/function.h"
int main()
{
    pkg packet;
    FILE* fp = fopen("./conf/setting","r");
    perror("fopen");
    int fd;
    char ip[20];
    char port[20];
    char path[50];
    fscanf(fp,"ip = %s\n",ip);
    printf("%s\n",ip);
    fscanf(fp,"port = %s\n",port);
    printf("%s\n",port);
    fscanf(fp,"path = %s\n",path);
    printf("%s\n",path);
    struct sockaddr_in *paddr;
    paddr = tcpConnect(ip,port,&fd);
    socklen_t len = sizeof(struct sockaddr);
    //initiate the argument struct
    thrqueue *pq = (thrqueue *)calloc(1,sizeof(thrqueue));
    queueini(pq);
    arg *pa = (arg *)calloc(1,sizeof(arg));
    pa->que = pq;
    pa->pmutex = &pq->mutex;
    pthread_mutex_t sqlmutex;
    pthread_mutex_init(&sqlmutex,NULL);//sql lock
    pa->psqlmutex = &sqlmutex;
    pa->pcond = (pthread_cond_t *)calloc(1,sizeof(pthread_cond_t));
    pthread_cond_init(pa->pcond,NULL);
    strcpy(pa->f_path,path);
    //initiate the thread pool
    int num = 10;
    pthread_t *ptid = (pthread_t *)calloc(num,sizeof(pthread_t));

    for(int i = 0; i < num; i++)
    {
        pthread_create(ptid+i,NULL,pthreadhandler,pa);
    }

    node newnode;
    while(1)
    {
        int nfd = accept(fd,(struct sockaddr *)paddr,&len);
        printf("new user connected!\n");
        newnode.fd = nfd;
        printf("main thread fd = %d\n",nfd);
        newnode.next = NULL;
        pthread_mutex_lock(pa->pmutex);
        enqueue(pq,&newnode);
        pthread_mutex_unlock(pa->pmutex);
        pthread_cond_signal(pa->pcond);
    }
       return 0;
}
