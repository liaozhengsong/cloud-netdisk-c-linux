#include "../include/function.h"
void sighandler(int No)
{
    printf("%d signal, disconnected!\n",No);
    pthread_exit(NULL);
}
void *pthreadhandler(void *args)
{
    //为了防止当前状态被其他线程修改，在访问队列时，复制创建一份本线程状态
    node nnode;
    targ targs;
    pkg packet;
    signal(SIGPIPE,sighandler);
    while(1)
    {
        //加锁访问队列和当前状态等临界资源
        pthread_mutex_lock(((arg *)args)->pmutex);
        if(((arg *)args)->que->size == 0)
            pthread_cond_wait(((arg *)args)->pcond,((arg *)args)->pmutex);
        dequeue(((arg *)args)->que,&nnode);
        targs.fd = nnode.fd;
        strcpy(targs.f_path,((arg *)args)->f_path);
        pthread_mutex_unlock(((arg *)args)->pmutex);
        printf("fd = %d\n",nnode.fd);
        int fd = nnode.fd;

        char user[100] = {0};
        char passwd[100] = {0};
        char ssalt[17] = {0};
        char logpath[100] = {0};
        char root[] = "/home/liaozs/pan/server/log";
        MYSQL *conn;
        MYSQL_RES *res;
        MYSQL_ROW row;
        char server[] = "localhost";
        char dbuser[] = "root";
        char dbpasswd[] = "0012300";
        char db[] = "vfs";
        char cmd[100] = {0};
        int ret;
        //lock&unlock before and after query&store_result
        conn = mysql_init(NULL);
        if(mysql_real_connect(conn,server,dbuser,dbpasswd,db,0,NULL,0))
        {
            printf("database connect successfully!\n");
        }
        else
        {
            printf("%s\n",mysql_error(conn));   
        }
        
        int type = USEROPT;
        char buf[100] = {0};
        int logfd;
        while(1)
        {
            pack(&packet,&type,buf);
            sendpkg(fd,&packet);
            recvpkg(fd,&packet);
            unpack(&packet,&type,buf);
            switch(type)
            {
            case LOGINSUB:
                type = LOGINUSER;
                break;
            case LOGINUSER:
                sprintf(cmd,"select * from user where user = '%s'",buf);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(ret != 0)
                {
                    type = SQLERROR;
                    strcpy(buf,mysql_error(conn));
                    break;
                }
                ret = mysql_field_count(conn);
                if(ret == 0)
                {
                    type = SQLERROR;
                    break;
                }
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                res = mysql_store_result(conn);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(res)
                {
                    row = mysql_fetch_row(res);
                    if(row != NULL)
                    {
                        strcpy(user,row[0]);
                        strcpy(passwd,row[1]);
                        strcpy(logpath,row[3]);
                        type = LOGINSALT;
                        strcpy(buf,row[2]);
                    }
                    else
                    {
                        type = SQLERROR;
                        break;
                    }
                }
                else
                {
                    type = SQLERROR;
                    strcpy(buf,mysql_error(conn));
                }
                mysql_free_result(res);
                break;
            case LOGINPASSWORD:
                if(strcmp(buf,passwd) == 0)
                {
                    type = LOGINSUCCESS;
                    break;
                }
                else
                {
                    type = LOGINFAIL;
                    break;
                }
            case LOGINSUCCESS:
                goto endopt;
            case LOGINFAIL:
                type = USEROPT;
                break;
            case SIGNUPSUB:
                type = SIGNUPSUB;
                break;
            case SIGNUPUSER:
                strcpy(user,buf);
                type = SIGNUPPASSWORD;
                break;
            case SIGNUPPASSWORD:
                salt(ssalt,sizeof(ssalt)-1);
                sha512(passwd,buf,ssalt);
                sprintf(logpath,"%s/%s",root,user);
                sprintf(cmd,"insert into user values('%s','%s','%s','%s')",user,passwd,ssalt,logpath);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(ret != 0)
                {
                    type = SQLERROR;
                    strcpy(buf,mysql_error(conn));
                    break;
                }
                logfd = open(logpath,O_CREAT|O_RDWR,0666);
                close(logfd);
                type = USEROPT; 
                break;
            case SQLERROR:
                type = USEROPT;
                break;
            case USEROPTERROR:
                type = USEROPT;
                break;
            default:
                break;
            }
        }
endopt:
        printf("%s\n",user);
        char cloud[] = "/home/liaozs/pan/server/cloud";
        char vpath[128] = "~/";
        char vfname[128] = {0};
        char file[100] = {0};
        //user file systems
        type = CMD;
        int precode = 0;
        off_t offset = 0;
        off_t fsize;
        int nfd;
        char dbmd5[100] = {0};
        struct stat statbuf;
        mode_t mode;
        while(1)
        {   
            pack(&packet,&type,buf);
            sendpkg(fd,&packet);
            recvpkg(fd,&packet);
            unpack(&packet,&type,buf);
            switch(type)
            {
            case LS:
                sprintf(cmd,
                        "select fname from filepool where precode = %d and user = '%s'",
                        precode,user);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(ret != 0)
                {
                    type = FILEERROR;
                    strcpy(buf,mysql_error(conn));
                    break;
                }
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                res = mysql_store_result(conn);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                memset(file,0,sizeof(file));
                for(unsigned int i = 0; i < mysql_num_rows(res); i++)
                {
                    row = mysql_fetch_row(res);
                    strcat(file,row[0]);
                    strcat(file," ");
                }
                mysql_free_result(res);
                strcpy(buf,file);
                type = LS;
                break;

            case CD:
                strcpy(vfname,packet.buf);
                if(strcmp(vfname,".") == 0)
                {
                    strcpy(buf,"current path");
                    type = CD;
                    break;
                }
                else if(strcmp(vfname,"..") == 0)
                {
                    if(precode == 0)
                    {
                        strcpy(buf,"it is root");
                        type = FILEERROR;
                        break;
                    }
                    else
                    {
                        sprintf(cmd,
                            "select precode from filepool where code = %d",
                            precode);
                        pthread_mutex_lock(((arg *)args)->psqlmutex);
                        ret = mysql_query(conn,cmd);
                        pthread_mutex_unlock(((arg *)args)->psqlmutex);
                        if(ret != 0)
                        {
                            type = FILEERROR;
                            strcpy(buf,mysql_error(conn));
                            break;
                        }
                        pthread_mutex_lock(((arg *)args)->psqlmutex);
                        res = mysql_store_result(conn);
                        pthread_mutex_unlock(((arg *)args)->psqlmutex);
                        if(res -> row_count == 0)
                        {
                            strcpy(buf,"No such directory!");
                            type = FILEERROR;
                            break;
                        }
                        row = mysql_fetch_row(res);
                        sscanf(row[0],"%d",&precode);
                        mysql_free_result(res);
                        strcpy(buf,"change success");
                        type = CD;
                        break;
                    }
                }
                mode = 'r';
                sprintf(cmd,
                        "select code from filepool where type = '%c' and fname = '%s' and precode = %d and user = '%s'",
                        mode,vfname,precode,user);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(ret != 0)
                {
                    type = FILEERROR;
                    strcpy(buf,mysql_error(conn));
                    break;
                }
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                res = mysql_store_result(conn);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                memset(file,0,sizeof(file));
                if(res -> row_count == 0)
                {
                    strcpy(buf,"No such directory!");
                    type = FILEERROR;
                    break;
                }
                row = mysql_fetch_row(res);
                sscanf(row[0],"%d",&precode);
                mysql_free_result(res);
                strcpy(buf,"change success");
                type = CD;
                break;

            case PUTS:
                strcpy(vfname,packet.buf);
                sprintf(cmd,
                        "select * from filepool where precode = %d and fname = '%s'",
                        precode,vfname);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(ret != 0)
                {
                    type = FILEERROR;
                    strcpy(buf,mysql_error(conn));
                    break;
                }
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                res = mysql_store_result(conn);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(res->row_count != 0)
                {
                    strcpy(buf,"repeated filename!");
                    type = FILEERROR;
                    break;
                }
                type = PUTS;
                break;
            case PUTSFILE:
                sscanf(packet.buf,"%ld %s",&fsize,dbmd5);
                sprintf(cmd,
                        "select * from filepool where md5 = '%s'",
                        dbmd5);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(ret != 0)
                {
                    type = FILEERROR;
                    strcpy(buf,mysql_error(conn));
                    break;
                }
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                res = mysql_store_result(conn);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                
                if(res->row_count != 0)
                {
                    type = PUTSEND;
                    break;
                }
                
                sprintf(file,"%s/%s",cloud,dbmd5);
                nfd = open(file,O_CREAT|O_RDWR,0666);
                if(nfd == -1)
                {
                    perror("open");
                    type = FILEERROR;
                    sprintf(buf, "%s",strerror (errno));
                    break;
                }
                type = PUTSFILE;
                pack(&packet,&type,buf);
                sendpkg(fd,&packet);
                offset = 0;
                while(1)
                {
                    ret = read(fd,packet.buf,sizeof(packet.buf));
                    offset += ret;
                    printf("\r%5.2lf%%",100.0*offset/fsize);
                    if(offset + (long)sizeof(packet.buf) >= fsize)
                    {
                        write(nfd,packet.buf,ret);
                        ret = read(fd,packet.buf,fsize-offset);
                        offset += ret;
                        write(nfd,packet.buf,ret);
                        break;
                    }
                    else
                    {
                        write(nfd,packet.buf,ret);
                        memset(packet.buf,0,sizeof(packet.buf));
                    }
                }
                close(nfd);
                type = PUTSEND;
                break;
            case PUTSTER:
                mode = packet.buf[0];
                sprintf(cmd,
                        "insert into filepool(precode,user,md5,type,path,fname) values(%d,'%s','%s','%c','%s','%s')",
                        precode,user,dbmd5,mode,vpath,vfname);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(ret != 0)
                {
                    type = FILEERROR;
                    strcpy(buf,mysql_error(conn));
                    break;
                }
                type = CMD;
                break;
            case GETS:
                sscanf(packet.buf,"%s %ld",vfname,&offset);
                sprintf(cmd,
                        "select md5 from filepool where type = 'f' and fname = '%s' and precode = %d",
                        vfname,precode);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(ret != 0)
                {
                    type = FILEERROR;
                    strcpy(buf,mysql_error(conn));
                    break;
                }
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                res = mysql_store_result(conn);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(res->row_count == 0)
                {
                    type = FILEERROR;
                    strcpy(buf,"no such file");
                    break;
                }
                row = mysql_fetch_row(res);
                sprintf(file,"%s/%s",cloud,row[0]);
                printf("file = %s\n",file);
                mysql_free_result(res);
                nfd = open(file,O_RDWR);
                lseek(nfd,offset,SEEK_SET);
                if(nfd == -1)
                {
                    perror("open");
                    type = FILEERROR;
                    break;
                }
                
                ret = fstat(nfd,&statbuf);
                if(ret == -1)
                { 
                    perror("fstat");
                    type = FILEERROR;
                    break;
                }
                fsize = statbuf.st_size;
                sprintf(buf,"%ld %s",statbuf.st_size,vfname);
                type = GETS;
                break;
            case GETSFILE:
                offset = 0;
                while(1)
                {
                    memset(packet.buf,0,sizeof(packet.buf));
                    ret = read(nfd,packet.buf,sizeof(packet.buf));
                    if(ret == -1)
                        perror("read");

                    write(fd,packet.buf,ret);
                   
                    offset += ret;
                    printf("\r%5.2lf%%",100.0*offset/fsize); 
                    if(ret == 0)
                        break;
                }
                read(fd,&type,sizeof(int));
                break;
            case REMOVE:
                strcpy(vfname,packet.buf);
                sprintf(cmd,
                        "select md5 from filepool where fname = '%s' and precode = %d",
                        vfname,precode);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(ret != 0)
                {
                    type = FILEERROR;
                    strcpy(buf,mysql_error(conn));
                    break;
                }
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                res = mysql_store_result(conn);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                memset(file,0,sizeof(file));
                printf("%s\n",row[0]);
                if(res->row_count == 1)
                {
                    sprintf(file,"%s/%s",cloud,row[0]);
                    unlink(file);
                }
                mysql_free_result(res);
                sprintf(cmd,
                        "delete from filepool where precode = %d and fname = '%s'",
                        precode,vfname);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(ret != 0)
                {
                    type = FILEERROR;
                    strcpy(buf,mysql_error(conn));
                    break;
                }
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                res = mysql_store_result(conn);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                mysql_free_result(res);
                strcpy(buf,"remove success");
                type = REMOVE;
                break;

            case PWD:
                if(precode != 0)
                {
                    sprintf(cmd,
                        "select path,fname from filepool where code = %d",
                        precode);
                    pthread_mutex_lock(((arg *)args)->psqlmutex);
                    ret = mysql_query(conn,cmd);
                    pthread_mutex_unlock(((arg *)args)->psqlmutex);
                    if(ret != 0)
                    {
                        type = FILEERROR;
                        strcpy(buf,mysql_error(conn));
                        break;
                    }
                    pthread_mutex_lock(((arg *)args)->psqlmutex);
                    res = mysql_store_result(conn);
                    pthread_mutex_unlock(((arg *)args)->psqlmutex);
                    memset(buf,0,sizeof(buf));
                    row = mysql_fetch_row(res);
                    for(unsigned int i = 0; i < mysql_num_fields(res); i++)
                    {
                        strcat(buf,row[i]);
                    }
                    mysql_free_result(res);
                    strcat(buf,"/");
                }
                else
                    strcpy(buf,"~/");
                type = PWD;
                break;
            case MKDIR:
                mode = 'r';
                strcpy(vfname,packet.buf);
                sprintf(cmd,
                        "select * from filepool where precode = %d and fname = '%s'",
                        precode,vfname);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(ret != 0)
                {
                    type = FILEERROR;
                    strcpy(buf,mysql_error(conn));
                    break;
                }
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                res = mysql_store_result(conn);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                if(res->row_count != 0)
                {
                    strcpy(buf,"repeated filename!");
                    type = FILEERROR;
                    break;
                }
                mysql_free_result(res);
                if(precode != 0)
                {
                    sprintf(cmd,
                        "select path,fname from filepool where code = %d",
                        precode);
                    pthread_mutex_lock(((arg *)args)->psqlmutex);
                    ret = mysql_query(conn,cmd);
                    pthread_mutex_unlock(((arg *)args)->psqlmutex);
                    if(ret != 0)
                    {
                        type = FILEERROR;
                        strcpy(buf,mysql_error(conn));
                        break;
                    }
                    pthread_mutex_lock(((arg *)args)->psqlmutex);
                    res = mysql_store_result(conn);
                    pthread_mutex_unlock(((arg *)args)->psqlmutex);
                    row = mysql_fetch_row(res);
                    sprintf(vpath,"%s%s/",row[0],row[1]);
                    mysql_free_result(res);
                }
                else
                {
                    strcpy(vpath,"~/");
                }
                sprintf(cmd,
                        "insert into filepool(precode,user,type,path,fname) values(%d,'%s','%c','%s','%s')",
                        precode,user,mode,vpath,vfname);
                pthread_mutex_lock(((arg *)args)->psqlmutex);
                ret = mysql_query(conn,cmd);
                pthread_mutex_unlock(((arg *)args)->psqlmutex);
                strcpy(packet.buf,"create success!");
                type = MKDIR;
                break;
            default:
                break;
            }
         }
    }
}
