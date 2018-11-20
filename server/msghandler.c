//
// Created by shadowiterator on 18-10-27.
//

#include "global.h"
#include "msghandler.h"
#include "server.h"
#include "sistring.h"

typedef struct
{
    int filefd;
    ConnectArg* cargs;
}FileArgs;

int countParam(char* cmd)
{
    int p = 0;
    while(cmd[p] && cmd[p] != ' ')
        ++p;
    if(cmd[p] && cmd[p] == ' ')
        ++p;

    if(!cmd[p])
        return 0;
    else
        return 1;
}

int file_exists(char* path)
{
    return access(path, F_OK) == 0;
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
    return countParam(cmd) == n;
}

void decodePathName(char *pathname, int n)
{
    for(int i = 0; i < n; ++i)
        if(pathname[i] == 0x00)
            pathname[i] = 0x0A;
}

int getFullPathName(ConnectArg* args, char* ecdpathName, char* buffer)
{
    if(ecdpathName[0] == '/')
        sprintf(buffer, "%s%s", SERVERDIR, ecdpathName);
    else
        sprintf(buffer, "%s%s/%s", SERVERDIR, args->dir, ecdpathName);
    return 0;
}

int getUserPathName(ConnectArg* args, char* ecdpathName, char* buffer)
{
    if(ecdpathName[0] == '/')
        sprintf(buffer, "%s", ecdpathName);
    else
        sprintf(buffer, "%s/%s", args->dir, ecdpathName);
    return 0;
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
//            sendFmtMsg(&args->connfd, args, "accept failed", 0, 530);
//            puts("accept failed");
            printf("accept failed %d\n", args->connfd);
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
//    sendFmtMsg(&args->connfd, args, "data connection established", 0, 230);
}

int send_file(FileArgs* cargs)
{
#define RETRFBUFFSIZE 1024
    int fd = cargs->filefd;
    ConnectArg* args = cargs->cargs;
    char *fbuffer = malloc(RETRFBUFFSIZE * sizeof(char));
    int filelen;
    int msgRet = 1;
    while((filelen = read(fd, fbuffer, RETRFBUFFSIZE)))
    {
        usleep(100000);
//        sleep(1);
        if(filelen < 0)
        {
            sendFmtMsg(&args->connfd, args, "unable to open file", 0, 551);
            goto send_file_end;
        }
        else
        {
            msgRet = sendMsg(&args->datafd, args, fbuffer, filelen);
            if(msgRet < 0)
            {
                sendFmtMsg(&args->connfd, args, "connection broken", 0, 426);
                goto send_file_end;
            }
        }
    }

    sendFmtMsg(&args->connfd, args, "transfer success", 0, 226);

    send_file_end:
    cargs->cargs->sp = 0;
    free(fbuffer);
    free(cargs);
    close(fd);
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

int recv_file(FileArgs* cargs)
{
#define STORFBUFFSIZE 1024
    char* buffer = malloc(STORFBUFFSIZE * sizeof(char));

    int fd = cargs->filefd;
    ConnectArg* args = cargs->cargs;
    int flen = 0;
    while((flen = readMsg(&args->datafd, args, buffer, STORFBUFFSIZE)))
    {
        if(flen < 0)
        {
            sendFmtMsg(&args->connfd, args, "connection broken", 0, 426);
            goto recv_file_end;
        }
        if(write(fd, buffer, flen) < 0)
        {
            printf("Error: %s(%d)\n", strerror(errno), errno);
            sendFmtMsg(&args->connfd, args, "failed to write file", 0, 452);
            goto recv_file_end;
        }
    }
    sendFmtMsg(&args->connfd, args, "STOR success", 0, 226);
    recv_file_end:
    close(fd);
    free(buffer);
    free(cargs);
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
        printf("user len = %d cmdn = %d\n", len, cmdn);
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
        set_cmd_status(args, QUIT, CMD_ENABLE);
        sendFmtMsg(&args->connfd, args, " password?", 0, 331);
    }
    return 0;
}

int pass_handler(ConnectArg* args, char* cmd, int cmdn)
{
//    int len = 0;
//    int p;
//    char* msg;
//    int code;
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
//        set_cmd_status(args, LIST, CMD_DISABLE);
//        set_cmd_status(args, STOR, CMD_DISABLE);
//        set_cmd_status(args, RETR, CMD_DISABLE);

        sendFmtMsg(&args->connfd, args, "login!", 0, 230);
//        sendFmtMsg(&args->connfd, args, " welcome", 0, 230);
    }

    return 0;
}

int list_handler(ConnectArg* args, char* cmd, int cmdn)
{
    decodePathName(cmd, cmdn);
    if(args->datafd < 0)
    {
        sendFmtMsg(&args->connfd, args, "no connection established", 0, 425);
        return 0;
    }
    int p;
    char dpath[1024];

    if(countParam(cmd) == 1)
    {
        p = getParam(cmd, 1);
        getFullPathName(args, &cmd[p], dpath);
    }
    else
        getFullPathName(args, "", dpath);
//    int p = countParam(cmd);
//    if(p > 1)
//    {
//        sendFmtMsg(&args->connfd, args, " too many args", 0, 530);
//        return 0;
//    }
    char si_cmd[2048];
//    sprintf(si_cmd, "cd %s", dpath);
//    system(si_cmd);
    sprintf(si_cmd, "ls -a -l %s", dpath);
    FILE *fpread = popen(si_cmd, "r");
    char res[2048];
    int si_len;
//    char tt = fgetc(fpread);
//    fscanf(fpread, "%[^\n]%c", res);
    while(fgetc(fpread) != '\n');
//        tt = fgetc(fpread);
    while((si_len = fread(res, 1, 2047, fpread))>0)
    {
        res[si_len] = '\0';
        printf("%s", res);
        if(sendMsg(&args->datafd, args, res, si_len) < 0)
            break;
        printf("%s", res);
    }
//
//    DIR *dir;
//    struct dirent *ptr;
//    struct stat finfo;
//    char path[1024];
//    char msg[1024];
//    dir = opendir(dpath);
//    ptr = readdir(dir);
//    while(ptr)
//    {
//        sprintf(path, "%s%s", dpath, ptr->d_name);
//        stat(path, &finfo);
//        sprintf(msg, " %s %d", ptr->d_name, S_ISDIR(finfo.st_mode));
//        sendFmtMsg(&args->datafd, args, msg, 0, 230);
//        ptr = readdir(dir);
//    }

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

int syst_handler(ConnectArg* args, char* cmd, int cmdn)
{
    sendFmtMsg(&args->connfd, args, "UNIX Type: L8", 0, 215);
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
//    int len;
    p = getParam(cmd, 1);
//    len = cmdn - p;
    if(sistrcmp("I", cmd, 0, p, 1) != 0)
    {
        sendFmtMsg(&args->connfd, args, "expected I as parameter", 0, 530);
        return 0;
    }
    else
    {
        sendFmtMsg(&args->connfd, args, "Type set to I.", 0, 200);
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
    int listenfd;
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
    sprintf(retMsg, "=%s,%d,%d", "127,0,0,1", (rport >> 8) & 0xff, rport & 0xff);



    sendFmtMsg(&args->connfd, args, retMsg, 0, 227);

    return 0;
}

int rest_handler(ConnectArg* args, char* cmd, int cmdn)
{
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(&args->connfd, args, "no path specified", 0, 530);
        return 0;
    }
    int p = getParam(cmd, 1);
//    int plen = cmdn - p;
    char* ptr = NULL;
    int nsp = (int)(strtol(&cmd[p], &ptr, 10));
    if (errno != 0 && nsp == 0)
    {
        sendFmtMsg(&args->connfd, args, "invalid parameter", 0, 550);
        return 0;
    }
    args->sp = nsp;
    sendFmtMsg(&args->connfd, args, "successfully set starting point", 0, 350);

    return 0;
}

int retr_handler(ConnectArg* args, char* cmd, int cmdn)
{
    int p;
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
//    len = cmdn - p;
//    decodePathName(&cmd[p], len);
//    sprintf(path, "%s%s%s", SERVERDIR, args->dir, &cmd[p]);
    getFullPathName(args, &cmd[p], path);
    printf("trying to open %s with sp = %d\n", path, args->sp);



    FileArgs* cargs = malloc(sizeof(FileArgs));
    cargs->filefd = open(path, O_RDONLY);
    cargs->cargs = args;
    if(lseek(cargs->filefd, args->sp, SEEK_SET) < 0 )
    {
        printf("Error: %s(%d)\n", strerror(errno), errno);
        sendFmtMsg(&args->connfd, args, "filed in lseek, try to set starting point first", 0, 452);
        lseek(cargs->filefd, 0, SEEK_END);
    }

    pthread_t pid;
    int pret = pthread_create(&pid, NULL, (void*)send_file, (void*)cargs);
    if(pret)
    {
        sendFmtMsg(&args->connfd, args, "create thread failed", 0, 530);
        return 0;
    }
    pthread_detach(pid);

    return 0;
}

int cwd_handler(ConnectArg* args, char* cmd, int cmdn)
{
    decodePathName(cmd, cmdn);
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(&args->connfd, args, "a parameter is required", 0, 550);
        return 0;
    }
    int p = getParam(cmd, 1);
//    int len = cmdn - p;
    char pathName[1040];
    char fullPathName[1040];
    getUserPathName(args, &cmd[p], pathName);
    getFullPathName(args, &cmd[p], fullPathName);
    if(reducePath(pathName, strlen(pathName)) < 0)
    {
        sendFmtMsg(&args->connfd, args, "invalid directory", 0, 550);
        return 0;
    }
    DIR* dir = opendir(fullPathName);
    if(dir == NULL)
    {
        sendFmtMsg(&args->connfd, args, "no such directory", 0, 550);
        return 0;
    }
    strcpy(args->dir, pathName);
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

int quit_handler(ConnectArg* args, char* cmd, int cmdn)
{
    sendFmtMsg(&args->connfd, args, "Bye", 0, 221);
    close(args->connfd);
    args->connfd = -1;
    return 0;
}

int stor_handler(ConnectArg* args, char* cmd, int cmdn)
{

    decodePathName(cmd, cmdn);
    sendFmtMsg(&args->connfd, args, "get STOR request", 0, 150);
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(&args->connfd, args, "a parameter expected", 0, 550);
        return 0;
    }
    if(args->datafd < 0)
    {
        sendFmtMsg(&args->connfd, args, "no connection established", 0, 425);
        return 0;
    }
    int p = getParam(cmd, 1);
    char path[1040];

    getFullPathName(args, &cmd[p], path);
    printf("stor : %s\n", path);

    FileArgs* cargs = malloc(sizeof(FileArgs));
    cargs->filefd = open(path, O_WRONLY | O_CREAT);
    cargs->cargs = args;

    pthread_t pid;
    int pret = pthread_create(&pid, NULL, (void*)recv_file, (void*)cargs);
    if(pret)
    {
        sendFmtMsg(&args->connfd, args, "create thread failed", 0, 530);
        return 0;
    }
    pthread_detach(pid);
    return 0;
}

int appe_handler(ConnectArg* args, char* cmd, int cmdn)
{

    decodePathName(cmd, cmdn);
    sendFmtMsg(&args->connfd, args, "get APPE request", 0, 150);
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(&args->connfd, args, "a parameter expected", 0, 550);
        return 0;
    }
    if(args->datafd < 0)
    {
        sendFmtMsg(&args->connfd, args, "no connection established", 0, 425);
        return 0;
    }
    int p = getParam(cmd, 1);
    char path[1040];

    getFullPathName(args, &cmd[p], path);
    printf("stor : %s\n", path);

    FileArgs* cargs = malloc(sizeof(FileArgs));
    cargs->filefd = open(path, O_WRONLY | O_CREAT | O_APPEND);
    cargs->cargs = args;

    pthread_t pid;
    int pret = pthread_create(&pid, NULL, (void*)recv_file, (void*)cargs);
    if(pret)
    {
        sendFmtMsg(&args->connfd, args, "create thread failed", 0, 530);
        return 0;
    }
    pthread_detach(pid);
    return 0;
}

int mkd_handler(ConnectArg* args, char* cmd ,int cmdn)
{
    decodePathName(cmd, cmdn);
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(&args->connfd, args, "a parameter is expected", 0, 530);
        return 0;
    }
    int p = getParam(cmd, 1);
    char pathname[1040];
    getFullPathName(args, &cmd[p], pathname);
    int tres = mkdir(pathname, S_IRWXU);
    if(tres < 0)
    {
        sendFmtMsg(&args->connfd, args, "create dir failed", 0, 550);
        return 0;
    }
    char msg[1080];
    sprintf(msg, "\"%s\"", args->dir);
    sendFmtMsg(&args->connfd, args, msg, 0, 250);
    return 0;
    /*
     * /a/b
     * MKD a/d/e
     * /a/b
     */
}

int rmd_handler(ConnectArg* args, char* cmd, int cmdn)
{
    decodePathName(cmd, cmdn);
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(&args->connfd, args, "a parameter is expected", 0, 530);
        return 0;
    }
    int p = getParam(cmd, 1);
    char pathname[1040];
    getFullPathName(args, &cmd[p], pathname);
    int tres = rmdir(pathname);
    if(tres < 0)
    {
        sendFmtMsg(&args->connfd, args, "remove dir failed", 0, 550);
        return 0;
    }
//    char msg[1080];
//    sprintf(msg, "\"%s\"", args->dir);
    sendFmtMsg(&args->connfd, args, "rm okey", 0, 250);
    return 0;
}

int rnfr_handler(ConnectArg* args, char* cmd, int cmdn)
{
    decodePathName(cmd, cmdn);
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(&args->connfd, args, "a parameter is required", 0, 530);
        return 0;
    }
    char pathName[1080];
    int p = getParam(cmd, 1);
    getFullPathName(args, &cmd[p], pathName);
    if(!file_exists(pathName))
    {
       sendFmtMsg(&args->connfd, args, "file do not exist", 0, 450);
        return 0;
    }
    sendFmtMsg(&args->connfd, args, "RNFR OK", 0, 350);
    int nlen = readMsg(&args->connfd, args, args->readbuffer, BUFFSIZE);
    if(nlen < 0)
        return 0;
    int hid = seek_handler(args, args->readbuffer);
    if(hid != RNTO)
        processMsg(args, args->readbuffer, nlen);
    else
        rnto_handler_accept(args, pathName, args->readbuffer, nlen);
    return 0;
}

int rnto_handler_accept(ConnectArg* args, char* oldpath, char* cmd, int cmdn)
{
    decodePathName(cmd, cmdn);
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(&args->connfd, args, "a parameter is required", 0, 550);
        return 0;
    }
    char newpath[1080];
    int p = getParam(cmd, 1);
    getFullPathName(args, &cmd[p], newpath);
    int tret = rename(oldpath, newpath);
    if(tret < 0)
    {
        sendFmtMsg(&args->connfd, args, "rename failed", 0, 553);
        return 0;
    }
    sendFmtMsg(&args->connfd, args, "rename ok", 0, 250);
    return 0;

}

int rnto_handler_refuse(ConnectArg* args, char* cmd, int cmdn)
{
    sendFmtMsg(&args->connfd, args, "RNTO must comes after RNFR", 0, 503);
    return 0;
}