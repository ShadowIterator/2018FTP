//
// Created by shadowiterator on 18-10-27.
//

#include "global.h"
#include "msghandler.h"
#include "server.h"
#include "sistring.h"



int countParam(char* cmd)
{
//    int p = 0;
    int rtn = 0;
    int p = 0;
//    for(int p = 0;; ++p, ++rtn)
//    {
    while(cmd[p] && cmd[p] != ' ')
        ++p;
    if(cmd[p] && cmd[p] == ' ')
        ++p;

    if(!cmd[p])
        return 0;
    else
        return 1;
//    }
}


int getIP(char* cmd, int len, unsigned int* ip)
{
    int data[6];
    int p = 0;
    int t;
    int port;
    char tt;
    for(int i = 0; i < 6; ++i)
    {
        for(t = 0; p < len && (tt = cmd[p]) != ','; ++p)
        {
            if(tt < '0' || tt > '9')
                return -1;
            t = t * 10 + tt - '0';
        }
        if(t & 0xffffff00)
            return -1;
        data[i] = t;
        ++p;
    }
    if(p <= len)
        return -1;
    port = (data[4] << 8 ) + data[5];
    if((port) & 0xffff0000)
        return -1;
    *ip = (unsigned int)(data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
    return port;
}

int getParam(char* cmd, int k)
{
    int p = 0;
    for(int i = 0; i < k; ++i)
    {
        while(cmd[p] && cmd[p] != ' ')
            ++p;
        if(cmd[p] && cmd[p] == ' ')
            ++p;
        if(!cmd[p])
            return -1;
    }
//    *len = p;
//    while(cmd[p] && cmd[p] != ' ') ++p;
//    p -= (*len);
//    (*len)^=p^=(*len)^=p;
    return p;
}

int checkParamterN(char* cmd, int n)
{
//    int len = 0;
//    int p;
//    return (p = getParam(cmd, n, &len)) > 0 && getParam(&cmd[p + len], 1, &len) == -1;
    return countParam(cmd) == n;
}

void decodePathName(char *pathname, int n)
{
    for(int i = 0; i < n; ++i)
        if(pathname[i] == 0x00)
            pathname[i] = 0x0A;
}

//int getNextDir(char* path, int p, int n)
//{
//    for(; p < n && path[p] !='/'; ++p);
//    return p + 1;
//}
//
//int getPrevDir(char* path, int p, int n)
//{
//    for(; p >= 0 && path[p] != '/'; --p);
//    for(--p; p >= 0 && path[p] != '/'; --p);
//    ++p;
//    return p;
//}

int getDirEd(char* path, int p, int n)
{
    while(p < n && path[p] == '/') ++p;
    for(; p < n && path[p] != '/'; ++p);
//    if(p != n)
//        --p;
    return p;
}

int getDirSt(char* path, int p, int n)
{
    while(p >=0 && path[p] == '/') --p;
    for(; p >= 0 && path[p] != '/'; --p);
    return p;
}

int reducePath(char* path, int n)
{
    int p = 0;
    int q = 0;
    int eq;
    if(path[q] != '/')
        return -1;
    if(n == 1)
    {
        path[0] = '\0';
        return 0;
    }
    while(q < n)
    {
        eq = getDirEd(path, q, n);
        if(sistrcmp("/..", &path[q], 0, 0, MAX(eq - q, 3)) == 0)
        {
            if(p == 0)
                return -1;
            else
            {
                p = getDirSt(path, p, n);
            }
        }
        else if(sistrcmp("/.", &path[q], 0, 0, MAX(eq - q, 2)) == 0)
        {
            // do nothing
        }
        else
        {
            for(;q < eq;)
            {
                path[p++] = path[q++];
            }
        }
        q = eq;
    }
    path[p] = '\0';
    return 0;
}


void wait_for_connection(ConnectArg* args)
{
//    printf("wait for connect\n");
    int datafd;
    while(1)
    {
        if(args->psvlistenfd < 0)
            break;
        if ((datafd = accept(args->psvlistenfd, NULL, NULL)) == -1)
        {
//            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            sendFmtMsg(&args->connfd, args, "accept failed", 0, 530);
            puts("accept failed");
            return ;
        }
        else
            break;

    }
    if(args->psvlistenfd < 0)
    {
        sendFmtMsg(&args->connfd, args, "listenfd closed", 0, 530);
        if(args->psvlistenfd != -1)
        {
            close(-args->psvlistenfd);
            puts("close listen");
        }
        return ;
    }
    printf("data connection from %d\n", datafd);
    args->datafd = datafd;
    sendFmtMsg(&args->connfd, args, "data connection established", 0, 230);

}

int user_handler(ConnectArg* args, char* cmd, int cmdn)
{
    int len = 0;
    int p;
    char* msg;
    int code;
    if(countParam(cmd) == 0)
    {
        msg = " expected exact 1 parameter";
        code = 530;
        p = -1;
    }
    else
    {
        p = getParam(cmd, 1);
        len = cmdn - p;
        if(sistrcmp("anonymous", cmd, 0, p, MAX(len, strlen("anonymous"))) != 0)
        {
            msg = " only support anonymous";
            code = 530;
            p = -1;
        }
    }
    if(p == -1)
        sendFmtMsg(&args->connfd, args, msg, 0, code);
    else
    {
        set_cmd_status_all(args, CMD_DISABLE);
        set_cmd_status(args, PASS, CMD_ENABLE);
//        sendFmtMsg(args, &cmd[p], len, 100);
        sendFmtMsg(&args->connfd, args, " password?", 0, 331);
    }
    return 0;
}

int pass_handler(ConnectArg* args, char* cmd, int cmdn)
{
    int len = 0;
    int p;
    char* msg;
    int code;
    if(countParam(cmd) == 0)
    {
//        msg = "expected exact 1 parameter";
//        code = 530;
//        p = -1;
        sendFmtMsg(&args->connfd, args, "expected exact 1 parameter", 0, 530);
        return 0;
    }
    else
    {
//        p = getParam(cmd, 1, &len);
//        valid!
        set_cmd_status_all(args, CMD_ENABLE);
        set_cmd_status(args, USER, CMD_DISABLE);
        set_cmd_status(args, PASS, CMD_DISABLE);
        set_cmd_status(args, LIST, CMD_DISABLE);
        set_cmd_status(args, STOR, CMD_DISABLE);
        set_cmd_status(args, RETR, CMD_DISABLE);

        sendFmtMsg(&args->connfd, args, "-login!", 0, 230);
        sendFmtMsg(&args->connfd, args, " welcome", 0, 230);
    }

    return 0;
}

int list_handler(ConnectArg* args, char* cmd, int cmdn)
{
    if(args->datafd < 0)
    {
        sendFmtMsg(&args->connfd, args, "no connection established", 0, 425);
        return 0;
    }
    int p = countParam(cmd);
//    if(p > 1)
//    {
//        sendFmtMsg(&args->connfd, args, " too many args", 0, 530);
//        return 0;
//    }
    DIR *dir;
    struct dirent *ptr;
    struct stat finfo;
    char path[1024];
    char msg[1024];
    dir = opendir("/home/shadowiterator/2018FTP/For_Student/ftp_dev/ftp_root");
    while(ptr = readdir(dir))
    {
//        printf("%s ", ptr->d_name);
//        stat(path, &finfo);
//        printf("%d ", S_ISDIR(finfo.st_mode));
//        printf("%d %d %d %d\n", finfo.st_uid, finfo.st_gid, finfo.st_atim, finfo.st_mtim);
        sprintf(path, "/home/shadowiterator/%s", ptr->d_name);
        stat(path, &finfo);
        sprintf(msg, " %s %d", ptr->d_name, S_ISDIR(finfo.st_mode));
        sendFmtMsg(&args->datafd, args, msg, 0, 230);
    }
//    if(args->datafd >= 0)
//    {
//        close(args->datafd);
//        args->datafd = -1;
//    }
    if(args->datafd != -1)
    {
        args->datafd = args->datafd > 0 ? -(args->datafd) : args->datafd;
//        shutdown(args->datafd, 0);
//        shutdown(args->datafd, 1);

        close(-args->datafd);
        puts("close datafd");
        args->datafd = -1;
    }
    set_cmd_status(args, LIST, CMD_DISABLE);
    set_cmd_status(args, STOR, CMD_DISABLE);
    set_cmd_status(args, RETR, CMD_DISABLE);

    return 0;
}

int syst_handler(ConnectArg* args, char* cmd, int cmdn)
{
    sendFmtMsg(&args->connfd, args, " UNIX Type:L8", 0, 215);
    return 0;
}

int type_handler(ConnectArg* args, char* cmd, int cmdn)
{
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(&args->connfd, args, " expected 1 parameter", 0, 530);
        return 0;
    }
    int p;
    int len;
    p = getParam(cmd, 1);
    len = cmdn - p;
    if(sistrcmp("I", cmd, 0, p, MAX(len, 1)) != 0)
    {
        sendFmtMsg(&args->connfd, args, "expected I as parameter", 0, 530);
        return 0;
    }
    else
    {
        sendFmtMsg(&args->connfd, args, " Type set to I", 0, 215);
        return 0;
    }
}

int port_handler(ConnectArg* args, char* cmd, int cmdn)
{
    if(countParam(cmd)!=1)
    {
        sendFmtMsg(&args->connfd, args, " expected 1 parameter", 0, 530);
        return 0;
    }
    int p;
    int len;
    p = getParam(cmd, 1);
    len = cmdn - p;
    unsigned int ip;
    int port;
    port = getIP(&cmd[p], len, &ip);
    if(port == -1)
    {
        sendFmtMsg(&args->connfd, args, " invalid ip address", 0, 530);
        return 0;
    }
    //-----------------------------------------------
    //--------set socket-----------------------------

    if(args->datafd != -1)
    {
        close(args->datafd > 0? args->datafd : -args->datafd);
        puts("close datafd");
        args->datafd = -1;
    }
    if(args->psvlistenfd != -1)
    {
        close(args->psvlistenfd > 0? args->psvlistenfd : -args->psvlistenfd);
        puts("close listen");
        args->psvlistenfd = -1;
    }

    int sockfd;
    struct sockaddr_in addr;
    //创建socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 0;
    }

    printf("Xdata connection: %d\n", sockfd);

    //设置目标主机的ip和port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, &ip, sizeof(unsigned int));
//    addr.sin_addr = ip;
//    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {			//转换ip地址:点分十进制-->二进制
//        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
//        return 1;
//    }

    int i = 3;
    //连接上目标主机（将socket和目标主机连接）-- 阻塞函数
    for (i = 3; i && (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0); --i) {
        sleep(1);
    }

    if(!i)
    {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        sendFmtMsg(&args->connfd, args, " connect fail", 0, 530);
        return 0;
    }

    puts("socket connection established");

    args->datafd = sockfd;
    sendFmtMsg(&args->connfd, args, "data connection established", 0, 200);
    printf("data connection: %d\n", sockfd);
    set_cmd_status(args, LIST, CMD_ENABLE);
    set_cmd_status(args, STOR, CMD_ENABLE);
    set_cmd_status(args, RETR, CMD_ENABLE);

    return 0;
}

int pasv_handler(ConnectArg* args, char* cmd, int cmdn)
{
    int listenfd, datafd;
    struct sockaddr_in addr;

    if(args->datafd != -1)
    {
        close(args->datafd > 0? args->datafd : -args->datafd);
        puts("close datafd");
        args->datafd = -1;
    }
    if(args->psvlistenfd != -1)
    {
        close(args->psvlistenfd > 0? args->psvlistenfd : -args->psvlistenfd);
        puts("close listen");
        args->psvlistenfd = -1;
    }

    //创建socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    int rport;
    int k;
    for(k = 20000; k; --k)
    {
        rport = (rand() % (45536)) + 20000;
        //设置本机的ip和port
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(rport);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"0.0.0.0"

        //将本机的ip和port与socket绑定
        if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            printf("Error bind(): %s(%d)\n", strerror(errno), errno);
//            return 1;
            continue;
        }
        else
            break;

    }
    if(!k)
    {
        sendFmtMsg(&args->connfd, args, "unable to find a port", 0, 530);
        return 0;
    }
//    printf("port = %d\n", rport);
    //开始监听socket
    if (listen(listenfd, 1) == -1) {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
//            return 1;
        sendFmtMsg(&args->connfd, args, "listen failed", 0, 530);
        return 0;
    }

//    int connfd;
//    if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
//        printf("Error accept(): %s(%d)\n", strerror(errno), errno);
//        return 0;
//    }

    // set non blocking
    int flags = fcntl(listenfd, F_GETFL, 0);
    fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);

    args->psvlistenfd = listenfd;

    pthread_t pid;
    int pret = pthread_create(&pid, NULL, (void*)wait_for_connection, (void*)args);
    if(pret)
    {
        sendFmtMsg(&args->connfd, args, "create thread failed", 0, 530);
        return 0;
    }
    pthread_detach(pid);

    char retMsg[100];
//    printf("xrport = %d\n",rport);
    sprintf(retMsg, "=%s,%d,%d", "127,0,0,1", (rport >> 8) & 0xff, rport & 0xff);

    set_cmd_status(args, LIST, CMD_ENABLE);
    set_cmd_status(args, STOR, CMD_ENABLE);
    set_cmd_status(args, RETR, CMD_ENABLE);

    sendFmtMsg(&args->connfd, args, retMsg, 0, 227);

    return 0;
}

int retr_handler(ConnectArg* args, char* cmd, int cmdn)
{
#define FBUFFSIZE 2000086
    int p, len;
    decodePathName(cmd, cmdn);
    sendFmtMsg(&args->connfd, args, "get RETR request", 0, 150);
    char path[1024];
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(&args->connfd, args, "no path specified", 0, 530);
        return 0;
    }
    if(args->datafd < 0)
    {
        sendFmtMsg(&args->connfd, args, "no connection established", 0, 425);
        return 0;
    }
    p = getParam(cmd, 1);
    len = cmdn - p;
//    decodePathName(&cmd[p], len);
    sprintf(path, "%s%s%s", SERVERDIR, args->dir, &cmd[p]);
    printf("trying to open %s\n", path);
    int fd = open(path, O_RDONLY);
    char *fbuffer = malloc(FBUFFSIZE * sizeof(char));
    int filelen;
    if((filelen = read(fd, fbuffer, FBUFFSIZE - 1)) < 0)
    {
        sendFmtMsg(&args->connfd, args, "unable to open file", 0, 551);
        goto retr_end;
    }
    fbuffer[filelen] = '\0';
    int msgRet = sendMsg(&args->datafd, args, fbuffer, filelen);
    if(msgRet < 0)
    {
        sendFmtMsg(&args->connfd, args, "connection broken", 0, 426);
        goto retr_end;
    }
    sendFmtMsg(&args->connfd, args, "transform success", 0, 226);
retr_end:
    free(fbuffer);
    if(args->datafd != -1)
    {
        close(args->datafd > 0? args->datafd : -args->datafd);
        puts("close datafd");
        args->datafd = -1;
    }
    if(args->psvlistenfd != -1)
    {
        close(args->psvlistenfd > 0? args->psvlistenfd : -args->psvlistenfd);
        puts("close listen");
        args->psvlistenfd = -1;
    }
    return 0;
}

int cwd_handler(ConnectArg* args, char* cmd, int cmdn)
{
    decodePathName(cmd,cmdn);
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(&args->connfd, args, "a parameter is required", 0, 550);
        return 0;
    }
    int p = getParam(cmd, 1);
    int len = cmdn - p;
    if(reducePath(&cmd[p], len) < 0)
    {
        sendFmtMsg(&args->connfd, args, "invalid directory", 0, 550);
        return 0;
    }
    strcpy(args->dir, &cmd[p]);
    sendFmtMsg(&args->connfd, args, "Okey", 0, 250);
    return 0;
}

int pwd_handler(ConnectArg* args, char* cmd, int cmdn)
{
    char msg[1040];
    sprintf(msg, "\"%s\"", args->dir);
    sendFmtMsg(&args->connfd, args, msg, 0, 550);
    return 0;
}