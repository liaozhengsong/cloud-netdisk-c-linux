#include "function.h"
int sha512(char *newkey,char *key,char *salt)
{
    strcpy(newkey,crypt(key,salt));
    return 0;
}
unsigned long checklocal(char *file)
{
    char localfile[100];
    sprintf(localfile,"/home/liaozs/pan/client/local/%s",file);
    int fd = open(localfile,O_RDONLY|O_CREAT,0666);
    struct stat statbuf;
    fstat(fd,&statbuf);
    close(fd);
    return statbuf.st_size;
}
int parsecmd(int fd,char *cmd,int *ptype,pkg *ppacket)
{
    char cmdlist[][10] = {"cd","ls","puts","gets","remove","pwd","mkdir"};
    char p[128] = {0};
    char buf[128] = {0};
    size_t len = strlen(cmd);
    unsigned long i;
    for(i = 0; i < len; i++)
    {
        if(cmd[i] != ' ')
            p[i] = cmd[i];
        else
            break;
    }
    while(cmd[i] == ' ')
    {
        ++i;
    }
    if(strcmp(cmdlist[0],p) == 0)
    {
        *ptype = CD;
        strcpy(buf,cmd+i);
        pack(ppacket,ptype,buf);
        sendpkg(fd,ppacket);
    }
    else if (strcmp(cmdlist[1],p) == 0)
    {
        *ptype = LS;
        pack(ppacket,ptype,buf);
        sendpkg(fd,ppacket);

    }
    else if (strcmp(cmdlist[2],p) == 0)
    {
        *ptype = PUTS;
        strcpy(buf,cmd+i);
        pack(ppacket,ptype,buf);
        printf("%s\n",ppacket->buf);
        sendpkg(fd,ppacket);
    }
    else if (strcmp(cmdlist[3],p) == 0)
    {
        *ptype = GETS;
        sprintf(buf,"%s %ld",cmd+i,checklocal(cmd+i));
        printf("buf = %s\n",buf);
        pack(ppacket,ptype,buf);
        sendpkg(fd,ppacket);

    }
    else if (strcmp(cmdlist[4],p) == 0)
    {
        *ptype = REMOVE;
        strcpy(buf,cmd+i);
        pack(ppacket,ptype,buf);
        sendpkg(fd,ppacket);
    }
    else if (strcmp(cmdlist[5],p) == 0)
    {
        *ptype = PWD;
        pack(ppacket,ptype,buf);
        sendpkg(fd,ppacket);
    }
    else if (strcmp(cmdlist[6],p) == 0)
    {
        *ptype = MKDIR;
        strcpy(buf,cmd+i);
        pack(ppacket,ptype,buf);
        sendpkg(fd,ppacket);
    }

    return 0;
}
char checkmode(mode_t mode)
{
    mode = mode >> 12;
    switch(mode)
    {
    case 01:
        return 'p';
    case 02:
        return 'c';
    case 04:
        return 'r';
    case 06:
        return 'b';
    case 010:
        return 'f';
    case 012:
        return 's';
    case 014:
        return 'k';
    default:
        return 0;
    }
}
int main()
{
    FILE* fp = fopen("./setting","r");
    char ip[20];
    char port[20];
    char path[50];
    fscanf(fp,"ip = %s\n",ip);
    printf("%s\n",ip);
    fscanf(fp,"port = %s\n",port);
    fscanf(fp,"path = %s\n",path);
    printf("%s\n",path);
    chdir(path);
    printf("%s\n",getcwd(NULL,0));
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(atoi(port));
    addr.sin_family = AF_INET;
    int fd;
    fd = socket(AF_INET,SOCK_STREAM,0);
    bind(fd,(struct sockaddr *)&addr,sizeof(struct sockaddr));
    connect(fd,(struct sockaddr *)&addr,sizeof(struct sockaddr));
    //login routine
    char buf[256] = {0};
    char input[128] = {0};
    char output[128] = {0};
    pkg packet;
    int type;
    //login&signup routine
    while(1)
    {
        recvpkg(fd,&packet);
        unpack(&packet,&type,buf);
        switch(type)
        {
        case USEROPT:
            printf("please select an option:\n1.login\n2.signup\n");
            scanf("%s",input);
            memset(&packet,0,sizeof(pkg));
            if(strcmp(input,"1")==0)
            {
                type = LOGINSUB;
                pack(&packet,&type,input);
                sendpkg(fd,&packet);
            }
            else if(strcmp(input,"2")==0)
            {
                type = SIGNUPSUB;
                pack(&packet,&type,input);
                sendpkg(fd,&packet);
            }
            else
            {
                type = USEROPTERROR;
                pack(&packet,&type,input);
                sendpkg(fd,&packet);
            }
            break;
        case LOGINUSER:
            //读取用户名
            printf("login :input your username:\n");
            scanf("%s",input);
            strcpy(output,input);
            type = LOGINUSER;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case LOGINSALT:
            //已知salt 获取密码并加密 传输密文
            printf("login :input your password:\n");
            scanf("%s",input);
            sha512(output,input,buf);
            type = LOGINPASSWORD;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case LOGINSUCCESS:
            printf("login success!\n");
            type = LOGINSUCCESS;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            goto endopt; 
        case LOGINFAIL:
            printf("wrong password!\n");
            type = LOGINFAIL;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case SIGNUPSUB:
            printf("signup :input your username:\n");
            type = SIGNUPUSER;
            scanf("%s",output);
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case SIGNUPPASSWORD:
            printf("signup :input your passwd:\n");
            type = SIGNUPPASSWORD;
            scanf("%s",output);
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case SQLERROR:    
            printf("sql error!\n");
            printf("%s\n",buf);
            type = SQLERROR;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        default:
            printf("unknown error!\n");
            break;
        }
    }
endopt:
    printf("file routine!\n");
    int nfd;
    int ret;
    char tmp[10] = {0};
    char dbmd5[100] = {0};
    char localfile[128] = {0};
    struct stat statbuf;
    mode_t mode;
    unsigned char md[100] = {0};
    MD5_CTX ctx;
    char c;
    off_t fsize;
    off_t done = 0;
    double percent;
    MD5_Init(&ctx);
    while(1)
    {
        recvpkg(fd,&packet);
        unpack(&packet,&type,buf);
        printf("type = %d\n",type);
        switch(type)
        {
        case CMD:
            printf("please input command:\n");
            while(1)
            {
                c = getc(stdin);
                if(c != '\n')
                    break;
            }
            input[0] = c;
            for(int i = 1; ; i++)
            {
                scanf("%c",&input[i]);
                if(input[i] == '\n')
                {
                    input[i] = '\0';
                    break;
                }
            }
            parsecmd(fd,input,&type,&packet);
            break;
        case LS:
            printf("%s\n",packet.buf);
            type = CMD;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case CD:
            printf("%s\n",packet.buf);
            type = CMD;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case PUTS:
            printf("111\n");
            strcpy(input,packet.buf);
            sprintf(localfile,"/home/liaozs/pan/client/local/%s",input);
            printf("%s\n",localfile);  
            nfd = open(localfile,O_RDWR);
            if(nfd == -1)
            {
                perror("open");
                type = CMD;
                pack(&packet,&type,output);
                sendpkg(fd,&packet);
                break;
            }
            memset(buf,0,sizeof(buf));
            while((ret = read(nfd,buf,sizeof(buf))) != 0)
            {
                MD5_Update(&ctx,(void *)buf,(unsigned long)ret);
                memset(buf,0,sizeof(buf));
            }
            MD5_Final(md,&ctx);
            memset(&ctx,0,sizeof(ctx));
            MD5_Init(&ctx);
            memset(dbmd5,0,sizeof(dbmd5));
            for(int i = 0; i < 16;i++)
            {
                sprintf(tmp,"%02x",md[i]);
                strcat(dbmd5,tmp);
            }
            ret = fstat(nfd,&statbuf);
            if(ret == -1)
                perror("fstat");
            fsize = statbuf.st_size;
            mode = statbuf.st_mode;
            type = PUTSFILE;
            sprintf(output,"%ld %s",statbuf.st_size,dbmd5);
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case PUTSFILE:
            printf("222\n");
            done = 0;
            lseek(nfd,0,SEEK_SET);
            while(1)
            {
                memset(packet.buf,0,sizeof(packet.buf));
                ret = read(nfd,packet.buf,sizeof(packet.buf));
                if(ret == -1)
                    perror("read");

                write(fd,packet.buf,ret);
               
                done += ret;
                percent = 100.0*done/fsize;
                printf("\r%5.2lf%%",percent); 
                if(ret == 0)
                    break;
            }
           // type = PUTSEND;
           // pack(&packet,&type,output);
           // sendpkg(fd,&packet);
            break;
        case PUTSEND:
            memset(output,0,sizeof(output));
            *output = checkmode(mode);
            close(nfd);
            type = PUTSTER;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case GETS:
            sscanf(packet.buf,"%ld %s",&fsize,input);
            sprintf(localfile,"/home/liaozs/pan/client/local/%s",input);
            nfd = open(localfile,O_RDWR|O_CREAT|O_APPEND,0666);
            if(nfd == -1)
            {
                perror("open");
                type = CMD;
                pack(&packet,&type,output);
                sendpkg(fd,&packet);
                break;
            }
            type = GETSFILE;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            done = 0;
            while(1)
            {
                ret = read(fd,packet.buf,sizeof(packet.buf));
                done += ret;
                printf("\r%5.2lf%%",100.0*done/fsize);
                if(done + (long)sizeof(packet.buf) >= fsize)
                {
                    write(nfd,packet.buf,ret);
                    ret = read(fd,packet.buf,fsize-done);
                    done += ret;
                    write(nfd,packet.buf,ret);
                    break;
                }
                else
                {
                    write(nfd,packet.buf,ret);
                    memset(packet.buf,0,sizeof(packet.buf));
                }
            }
            printf("%ld\n",done);
            close(nfd);
            type = GETSEND;
            write(fd,&type,sizeof(int));
            break;
        case GETSEND:
            printf("done!\n");
            type = CMD;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case FILEERROR:
            printf("%s,file error!\n",packet.buf);
            type = CMD;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case MKDIR:
            printf("%s\n",packet.buf);
            type = CMD;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case PWD:
            printf("%s\n",packet.buf);
            type = CMD;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;
        case REMOVE:
            printf("%s\n",packet.buf);
            type = CMD;
            pack(&packet,&type,output);
            sendpkg(fd,&packet);
            break;

        }
    }
    return 0;
}
int recvpkg(int fd, pkg *ad)
{
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

