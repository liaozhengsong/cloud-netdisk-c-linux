#include "../include/function.h"
int recvpkg(int fd, pkg *ad)
{
    memset(ad,0,sizeof(pkg));
    read(fd,&ad->len,sizeof(int));
    read(fd,&ad->type,sizeof(int));
    int ret = read(fd,ad->buf,ad->len);
    return ret;
}
int sendpkg(int fd, pkg *ad)
{
    write(fd,&ad->len,sizeof(int));
    write(fd,&ad->type,sizeof(int));
    int ret = write(fd,ad->buf,ad->len);
    return ret;
}
int pack(pkg* ppacket,int *ptype,char *buf)
{
    // buf char array is terminated with '\0'
    memset(ppacket,0,sizeof(pkg));
    strcpy(ppacket->buf,buf);
    ppacket->type = *ptype;
    ppacket->len = strlen(buf)+1;
    return ppacket->len;
}
int unpack(pkg* ppacket,int *ptype,char *buf)
{
    strcpy(buf,ppacket->buf);
    *ptype = ppacket->type;
    return ppacket->len;
}

