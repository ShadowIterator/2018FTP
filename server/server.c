#include "global.h"
#include "sistring.h"
#include "msghandler.h"
#include "server.h"




//TODO: cmd_stats diff from clients!

CMDList cmd_list[CMD_N];

void set_cmd_status(ConnectArg* args,int id, int status)
{
//    cmd_list[id].enable = status;
    args->cmdflag &= ~(1 << id);
    args->cmdflag |= (status << id);
}

void set_cmd_status_all(ConnectArg* args, int status)
{
//    for(int i = 0; i < CMD_N; ++i)
//        cmd_list[i].enable = status;
    args->cmdflag = status? ((1 << CMD_N) - 1) : 0;
}

void _init_cmd_list()
{
    memset(cmd_list, 0, CMD_N * sizeof(CMDList));

    register_cmd("USER", USER, user_handler);
    register_cmd("PASS", PASS, pass_handler);
    register_cmd("LIST", LIST, list_handler);
    register_cmd("SYST", SYST, syst_handler);
    register_cmd("TYPE", TYPE, type_handler);
    register_cmd("PORT", PORT, port_handler);
    register_cmd("PASV", PASV, pasv_handler);
    register_cmd("RETR", RETR, retr_handler);
    register_cmd("STOR", STOR, stor_handler);
    register_cmd("CWD", CWD, cwd_handler);
    register_cmd("PWD", PWD, pwd_handler);
    register_cmd("QUIT", QUIT, quit_handler);
    register_cmd("MKD", MKD, mkd_handler);
    register_cmd("RMD", RMD, rmd_handler);
    register_cmd("RNFR", RNFR, rnfr_handler);
    register_cmd("RNTO", RNTO, rnto_handler_refuse);
//    set_cmd_status(USER, CMD_ENABLE);
//    set_cmd_status(LIST, CMD_ENABLE);
//    set_cmd_status(SYST, CMD_ENABLE);
//    set_cmd_status(TYPE, CMD_ENABLE);
}

void _init_handler()
{
//    _init_cmd_list();
}

int register_cmd(char* cmd, int id, cmd_handler hdr)
{
    if(id < 0|| id >= CMD_N)
        return -1;
    if(cmd_list[id].CMD)
        free(cmd_list[id].CMD);
    cmd_list[id].CMD = malloc(strlen(cmd) + 5);
    strcpy(cmd_list[id].CMD, cmd);
    cmd_list[id].id = id;
    cmd_list[id].hdr = hdr;
    return 0;
}

void clear_connect_arg(ConnectArg* args)
{
    free(args->readbuffer);
    free(args->writebuffer);
    free(args->dir);
    args->readbuffer = NULL;
    args->writebuffer = NULL;
    args->dir = NULL;
    args->connfd = -1;
    free(args);
}

int seek_handler(ConnectArg* args, char* cmd)
{
    int p = 0;
    while(cmd[p] && cmd[p] != ' ') ++p;
//    --p;
    int hid = 0;
    for(; hid < CMD_N; ++hid)
    {
        if(cmd_list[hid].CMD && sistrcmp(cmd, cmd_list[hid].CMD, 0, 0, MAX(strlen(cmd_list[hid].CMD), p)) == 0)
            return ((args->cmdflag >> hid) & 1)? hid : -1;
    }
    return -1;
}

int readMsg(int* fd, ConnectArg* args, char* buffer, int bufferLen)
{
//    char* buffer = args->buffer;
//    int connfd = args->connfd;
    int len = 0;
    int p;
        //榨干socket传来的内容
    p = 0;
    while (1) {
        int n = read(*fd, buffer + p, bufferLen - 1 - p);
        if (n < 0) {
            printf("connection-%d : Error read(): %s(%d)\n", *fd, strerror(errno), errno);
            *fd = *fd > 0 ? -(*fd) : *fd;
            close(-(*fd));
            *fd = -1;
            return -1;
        } else if (n == 0) {
            break;
        } else {
            p += n;
            if (buffer[p - 1] == '\n' && (*fd) == args->connfd) {
                break;
            }
        }
    }
    //连接断开
    if (!p) {
        printf("%d client quit\n", args->connfd);
        *fd = *fd > 0 ? -(*fd) : *fd;
        close(-(*fd));
        *fd = -1;
        return -1;
    }
    //socket接收到的字符串并不会添加'\0'
    if((*fd) == args->connfd)
    {
        buffer[p - 1] = '\0';
        len = p - 1;
        if (buffer[len - 1] == '\r') {
            buffer[--len] = '\0';
        }
    }
    else
        len = p;
    // print debug info
    if((*fd) == args->connfd)
    {
        printf("%d recv%8x %s\n", *fd, len, buffer);
    }
    else
    {
        printf("%d recv%8x Bytes\n", *fd, len);
    }
//  handle exit _temp.
//    if(len == 1 && buffer[0] == 'Q')
//    {
////        write(connfd, "squit", 5);
//        sendMsg(fd, args, "squit", 5);
//    }
    return len;
}

int sendMsg(int* fd, ConnectArg* args, char* buffer, int len) {
//    char* wbuf[BUFFSIZE];
    if (!len)
        len = strlen(buffer);
//    int connfd = args->connfd;
    int p = 0;
    while (p < len) {
        int n = write(*fd, buffer + p, len - p);
        if (n < 0) {
//            args->closed = 1;
//            *fd = *fd > 0 ? -(*fd) : *fd;
            *fd = *fd > 0 ? -(*fd) : *fd;
            close(-(*fd));
            *fd = -1;
            printf("fd : %d Error write(): %s(%d)\n", *fd, strerror(errno), errno);
            return -1;
        } else {
            p += n;
        }
    }
    if ((*fd) == args->connfd)
    {
        printf("%d send%8x ", *fd, len);
            for (int i = 0; i < len; ++i)
                putchar(buffer[i]);
        putchar('\n');
    }
    else
    {
        printf("%d send%8x Bytes\n", *fd, len);
    }

    return 0;
}

int sendFmtMsg(int* fd, ConnectArg* args, char* buffer, int len, int code)
{
    char wbuf[BUFFSIZE + 10];
    char fmt[20];
    if(len <= 0) len = strlen(buffer);
    sprintf(fmt, "%%d %%.%ds\r\n", len);
    sprintf(wbuf, fmt, code, buffer);
    return sendMsg(fd, args, wbuf, strlen(wbuf));
}

int processMsg(ConnectArg* args, char* cmd, int len)
{
    int hid = seek_handler(args, cmd);
    if(hid < 0)
    {
        sendFmtMsg(&args->connfd, args, "invalid command", 0, 555);
        return -1;
    }
    (*(cmd_list[hid].hdr))(args, cmd, len);
    return 0;
}

int aserver(ConnectArg* args)
{
    args->datafd = -1;
    args->psvlistenfd = -1;
    args->readbuffer = malloc(BUFFSIZE * sizeof(char));
    args->writebuffer = malloc(BUFFSIZE * sizeof(char));
    args->dir = malloc(DIRSIZE * sizeof(char));
    strcpy(args->dir, "");
    set_cmd_status(args, USER, CMD_ENABLE);
    //------------------------------------
//    set_cmd_status(args, PASV, CMD_ENABLE);
//    set_cmd_status(args, PORT, CMD_ENABLE);
//    set_cmd_status(args, RETR, CMD_ENABLE);
//    set_cmd_status(args, CWD, CMD_ENABLE);
//    set_cmd_status(args, PWD, CMD_ENABLE);
//    set_cmd_status(args, MKD, CMD_ENABLE);
//    set_cmd_status(args, RMD, CMD_ENABLE);
//    set_cmd_status(args, RNFR, CMD_ENABLE);
//    set_cmd_status(args, RNTO, CMD_ENABLE);
    //------------------------------------
    int len;
    while(1)
    {

        if(args->connfd < 0)
            break;
        if(args->datafd != -1 && args->datafd < 0)
        {
            printf("close data: %d\n", args->datafd);
//            close(-args->datafd);
            args->datafd = -1;
        }
        len = readMsg(&args->connfd, args, args->readbuffer, BUFFSIZE);
        if(len > 0)
            processMsg(args, args->readbuffer, len);
//        sendMsg(args, args->buffer, len);
    }
//    close(-args->connfd);
    if(args->datafd > 0)
    {
        printf("close datafd\n");
        close(args->datafd);
    }
    if(args->psvlistenfd > 0)
    {
        printf("close listenfd\n");
        close(args->psvlistenfd);
    }
    puts("close connfd");
    clear_connect_arg(args);
    return 0;
}

int getConsoleParam(char* cmd, int argc, char** argv)
{
    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(cmd, argv[i]) == 0 && i + 1 < argc)
            return i + 1;
    }
    return -1;
}

int main(int argc, char **argv) {

    //----------test reduce path--------------------
//    char* tpath = "/hello/xie/die/../d";
//    char tpath[1000];
//    strcpy(tpath, "/hello/../xie/..die/../../../d");
//    int reduceRes = reducePath(tpath, strlen(tpath));
//    printf("%d : %s\n", reduceRes, tpath);

    //-----------test done------------

    int dirID = getConsoleParam("-root", argc, argv);
    int portID = getConsoleParam("-port", argc, argv);
    if(dirID > 0)
        strcpy(SERVERDIR, argv[dirID]);
    else
        strcpy(SERVERDIR, "/tmp");
    if(portID > 0)
        SERVERPORT = atoi(argv[portID]);
    else
        SERVERPORT = 21;

    int listenfd, connfd;		//监听socket和连接socket不一样，后者用于数据传输
    struct sockaddr_in addr;

    //创建socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    //设置本机的ip和port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVERPORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"0.0.0.0"

    //将本机的ip和port与socket绑定
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    //开始监听socket
    if (listen(listenfd, 10) == -1) {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    _init_cmd_list();
    printf("init cmd list done\n");
    while(1)
    {
        if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            continue;
        }
//        write(connfd, buffer + p, len + 1 - p);
        printf("connection from %d\n", connfd);
//        write(connfd, "connection established\n", 23);
        ConnectArg* parg = malloc(sizeof(ConnectArg));
        parg->connfd = connfd;
        int tret = pthread_create(&(parg->tid), NULL, (void*)aserver, (void*)parg);
        sendFmtMsg(&parg->connfd, parg, "Anonymous FTP server ready.", 0, 220);
        if(tret)
        {
            write(connfd, "create thread failed\n", 21);
            continue;
        }

        pthread_detach(parg->tid);
    }

    return 0;
    close(listenfd);
}

