//
// Created by shadowiterator on 18-10-27.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H
#include <pthread.h>

#define SERVERDIR ("/home/shadowiterator/2018FTP/For_Student/ftp_dev/ftp_root")
#define CMD_ENABLE 1
#define CMD_DISABLE 0
#define CMD_N (16)

enum CMD_TYPE
{
    USER = 0, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV,
    MKD, CWD, PWD, LIST, RMD, RNFR, RNTO
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
int readMsg(int* fd,ConnectArg* args, char* buffer);
int sendMsg(int* fd, ConnectArg* args, char* buffer, int len);
int sendFmtMsg(int* fd, ConnectArg* args, char* buffer, int len, int code);
#endif //SERVER_SERVER_H
