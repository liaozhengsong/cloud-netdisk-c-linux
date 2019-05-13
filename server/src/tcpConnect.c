#include "../include/function.h"

struct sockaddr_in * tcpConnect(char *ip, char *port,int *fdnet)
{
    struct sockaddr_in *paddr= (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    memset(paddr,0,sizeof(struct sockaddr_in));
    paddr->sin_port = htons(atoi(port));
    paddr->sin_addr.s_addr= inet_addr(ip);
    paddr->sin_family = AF_INET;
    int flag = 1;
    *fdnet = socket(AF_INET,SOCK_STREAM,0);
    setsockopt(*fdnet,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(int));
    bind(*fdnet,(struct sockaddr*)paddr,sizeof(struct sockaddr));
    listen(*fdnet,10);
    return paddr;
}

