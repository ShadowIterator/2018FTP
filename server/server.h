//
// Created by shadowiterator on 18-10-27.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H
#include <pthread.h>


#define CMD_ENABLE 1
#define CMD_DISABLE 0
#define CMD_N (18)
#define BUFFSIZE (8192)
#define DIRSIZE (1024)

char SERVERDIR[512];
int SERVERPORT;

enum CMD_TYPE
{
    USER = 0, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV,
    MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, REST, APPE
}cmd_id;

typedef struct
{
    pthread_t tid;
    char* readbuffer;
    char* writebuffer;
    int connfd;
    int datafd;
    char* dir;
    int closed;
    int cmdflag;
    int psvlistenfd;
    int sp;

}ConnectArg;

typedef int (*cmd_handler)(ConnectArg*, char*, int);

typedef struct
{
    char* CMD;
    int id;
    cmd_handler hdr;
//    int enable;
}CMDList;

void set_cmd_status(ConnectArg*, int, int);
void set_cmd_status_all(ConnectArg*, int);
//int enable_cmd(int);
int register_cmd(char* cmd, int id, cmd_handler hdr);
int readMsg(int* fd,ConnectArg* args, char* buffer, int bufferLen);
int sendMsg(int* fd, ConnectArg* args, char* buffer, int len);
int sendFmtMsg(int* fd, ConnectArg* args, char* buffer, int len, int code);
int seek_handler(ConnectArg* args, char* cmd);
int processMsg(ConnectArg* args, char* cmd, int len);
#endif //SERVER_SERVER_H
