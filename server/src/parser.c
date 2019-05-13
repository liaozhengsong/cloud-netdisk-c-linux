#include "../include/function.h"

int parsecmd(char *cmd, int len, targ *pa)
{
    char cmdlist[][10] = {"cd","ls","puts","gets","remove","pwd"};
    char p[256] = {0};
    for(int i = 0; i < len; i++)
    {
        if(cmd[i] != ' ')
            p[i] = cmd[i];
        else
            break;
    }
    p[len] = '\0';
    
    if(strcmp (cmdlist[0],p) == 0)
    {
        for(int i = strlen(p); i < len; i++)
        {
            if(cmd[i] != ' ')
            {
                f_cd(cmd + i, pa);
                goto end;
            }
        }
        goto end;
    }
   else if(strcmp(cmdlist[1],p) == 0)
   {
        for(int i = strlen(p); i < len && cmd[i] != '\0'; i++)
        {
            if(cmd[i] != ' ')
            {
                goto end;
            }
        }
        f_ls(pa);
   }
   else if(strcmp(cmdlist[2],p) == 0)
   {
        for(int i = strlen(p); i < len; i++)
        {
            if(cmd[i] != ' ')
            {
                f_puts(cmd + i, pa);
                goto end;
            }
        }
   }
   else if(strcmp(cmdlist[3],p) == 0)
   {
        for(int i = strlen(p); i < len; i++)
        {
            if(cmd[i] != ' ')
            {
                f_gets(cmd + i, pa);
                goto end;
            }
        }
   }
    else if(strcmp(cmdlist[4],p) == 0)
    {
        for(int i = strlen(p); i < len; i++)
        {
            if(cmd[i] != ' ')
            {
                f_remove(cmd + i, pa);
                goto end;
            }
        }
    }
    else if(strcmp(cmdlist[5],p) == 0)
    {
        for(int i = strlen(p); i < len && cmd[i] != '\0'; i++)
        {
            if(cmd[i] != ' ')
            {
                goto end;
            }
        }
        f_pwd(pa);
    }
    else
    {
        goto end;        
    }

end:
    return 0;
}
int f_cd(char* path, targ * args)
{
    DIR* dirp =  opendir(args->f_path);
    pkg packet;
    int flag = 1;
    struct dirent *pdirent;
    while((pdirent = readdir(dirp)) != NULL)
    {
        if(strcmp(path,pdirent->d_name) == 0)
        {
            flag = 0;
            sprintf(args->f_path,"%s/%s",args->f_path,path);
            chdir(args->f_path);
            strcpy(args->f_path,getcwd(NULL,0));
           
            strcpy(packet.buf,"success!\n");
            strcat(packet.buf,args->f_path);
            packet.len = strlen(packet.buf);
            break;
        }
    }
    closedir(dirp);
    if(flag == 1)
    {
        strcpy(packet.buf,"No such directory!");
        packet.len = strlen(packet.buf);
        return 0;
    }
    printf("current path is %s\n",args->f_path);
    return 0;
}
int f_ls(targ *args)
{
    DIR *dirp = opendir(args->f_path);
    struct dirent *pdirent;
    pkg packet;
    strcpy(packet.buf,"all files:\n");
    while((pdirent = readdir(dirp)) != NULL)
    {
        strcat(packet.buf,pdirent->d_name);
        strcat(packet.buf,"  ");
    }
    printf("%s\n",packet.buf);
    packet.len = strlen(packet.buf);
    return 0;
}
int f_puts(char *path,targ *args)
{
    chdir(args->f_path);
    int nfd = open(path,O_CREAT|O_RDWR,0777);
    pkg packet;
    char buf[1000]={0};
    int ret;
    if(nfd != -1)
    {
        strcpy(packet.buf,"file creat!");
        packet.len = strlen(packet.buf);
        sendpkg(args->fd,&packet);
    }
    else
    {
        strcpy(packet.buf,"create error!");
        packet.len = strlen(packet.buf);
        sendpkg(args->fd,&packet);
    }
    off_t f_size;
    printf("%ld\n",f_size);
    close(nfd);
    return 0;
}
int f_gets(char *path,targ *args)
{
    pkg packet;
    char buf[1000] = {0};
    chdir(args->f_path);
    int nfd = open(path,O_RDWR,0777);
    if(nfd == -1)
    {
        off_t err_sig = 0;
        packet.len = sizeof(off_t);
        memcpy(packet.buf,(char *)&err_sig,sizeof(off_t));
        sendpkg(args->fd,&packet);
        perror("open");
        return -1;
    }
    struct stat statbuf;
    fstat(nfd,&statbuf);
    off_t f_size = statbuf.st_size;
    packet.len = sizeof(f_size);
    memcpy(packet.buf,(char *)&f_size,sizeof(f_size));
    sendpkg(args->fd,&packet);
    printf("%ld\n",f_size);
    
    if(*(int *)buf != 0)
    {
        printf("%s\n",packet.buf);
        close(nfd);
        return -1;
    }
    memset(buf,0,sizeof(buf));
    printf("%s\n",buf);
    
    while(1)
    {
        memset(packet.buf,0,sizeof(packet.buf));
        int ret = read(nfd,packet.buf,sizeof(packet.buf));
        packet.len = ret;
        sendpkg(args->fd,&packet);
        if(ret == 0)
            break;
    }

    close(nfd);
    return 0;

}
int f_remove(char *path,targ* args)
{
    chdir(args->f_path);
    int ret = unlink(path);
    pkg packet;
    if(ret != 0)
    {
        perror("unlink");
        strcpy(packet.buf,"remove fail!");
        packet.len = strlen(packet.buf);
        sendpkg(args->fd,&packet);
    }
    else
    {
        strcpy(packet.buf,"remove success!");
        packet.len = strlen(packet.buf);
        sendpkg(args->fd,&packet);
    }
    f_ls(args);
    return 0;
}
int f_pwd(targ* args)
{
    char str[] = ".";
    f_cd(str,args);
    return 0;
}

