#include "global.h"
#include "sistring.h"
#include "msghandler.h"
#include "server.h"

#define BUFFSIZE (8192)



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
}

int seek_handler(ConnectArg* args, char* cmd)
{
    int p = 0;
    while(cmd[p] && cmd[p] != ' ') ++p;
//    --p;
    int hid = 0;
    for(; hid < CMD_N; ++hid)
    {
        if(sistrcmp(cmd, cmd_list[hid].CMD, 0, 0, p) == 0)
            return ((args->cmdflag >> hid) & 1)? hid : -1;
    }
    return -1;
}

int readMsg(ConnectArg* args, char* buffer)
{
//    char* buffer = args->buffer;
    int connfd = args->connfd;
    int len = 0;
    int p;
        //榨干socket传来的内容
    p = 0;
    while (1) {
        int n = read(connfd, buffer + p, 8191 - p);
        if (n < 0) {
            printf("connection-%d : Error read(): %s(%d)\n", args->connfd, strerror(errno), errno);
//            close(connfd);
//            clear_connect_arg(args);
            args->closed = 1;
            return -1;
        } else if (n == 0) {
            break;
        } else {
            p += n;
            if (buffer[p - 1] == '\n') {
                break;
            }
        }
    }
    //连接断开
    if (!p) {
        printf("%d client quit\n", args->connfd);
//        clear_connect_arg(args);
//        close(connfd);
        args->closed = 1;
        return -1;
    }
    //socket接收到的字符串并不会添加'\0'
    buffer[p - 1] = '\0';
    len = p - 1;

    // print debug info
    printf("%d recv%8x %s\n",args->connfd , len, buffer);

//  handle exit _temp.
    if(len == 1 && buffer[0] == 'Q')
    {
//        write(connfd, "squit", 5);
        sendMsg(args, "squit", 5);
    }
    return len;
}

int sendMsg(ConnectArg* args, char* buffer, int len)
{
//    char* wbuf[BUFFSIZE];
    if(!len)
        len = strlen(buffer);
    int connfd = args->connfd;
    int p = 0;
    while (p < len)
    {
        int n = write(connfd, buffer + p, len + 1 - p);
        if (n < 0)
        {
            args->closed = 1;
            printf("Error write(): %s(%d)\n", strerror(errno), errno);
            return -1;
        }
        else
        {
            p += n;
        }
    }
    printf("%d send%8x ", args->connfd, len);
    for(int i = 0; i < len; ++i)
        putchar(buffer[i]);
    putchar('\n');
    return 0;
}

int sendFmtMsg(ConnectArg* args, char* buffer, int len, int code)
{
    char wbuf[BUFFSIZE + 10];
    char fmt[20];
    if(len <= 0) len = strlen(buffer);
    sprintf(fmt, "%%d %%.%ds", len);
    sprintf(wbuf, fmt, code, buffer);
    return sendMsg(args, wbuf, strlen(wbuf));
}

int processMsg(ConnectArg* args, char* cmd, int len)
{
    int hid = seek_handler(args, cmd);
    if(hid < 0)
    {
        sendMsg(args, "invalid command", 0);
        return -1;
    }
    (*(cmd_list[hid].hdr))(args, cmd);
//    strcpy(args->writebuffer, args->readbuffer);
//    sendMsg(args, args->writebuffer, len);
}

int aserver(ConnectArg* args)
{
    args->readbuffer = malloc(BUFFSIZE * sizeof(char));
    args->writebuffer = malloc(BUFFSIZE * sizeof(char));
    args->dir = NULL;
    set_cmd_status(args, USER, CMD_ENABLE);
    int len;
    while(1)
    {

        if(args->closed)
            break;
        len = readMsg(args, args->readbuffer);
        if(len > 0)
            processMsg(args, args->readbuffer, len);
//        sendMsg(args, args->buffer, len);
    }
    close(args->connfd);
    clear_connect_arg(args);
    return 0;
}


int main(int argc, char **argv) {
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
    addr.sin_port = htons(6789);
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
        sendFmtMsg(parg, "Anonymous FTP server ready.", 0, 200);
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


//import socket
//
//size = 8192
//
//try:
//#  msg = raw_input()
//        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
//sock.connect(('localhost',6789))
//#  sock.sendto(msg, ('localhost', 9876))
//#  print sock.recv(size)
//for i in range(20, 21):
//print "send i"
//sock.send(str(i)+'\n')
//print sock.recv(size)
//sock.close()
//
//except:
//        print "cannot reach the server"