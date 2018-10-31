//
// Created by shadowiterator on 18-10-27.
//

#ifndef SERVER_MSGHANDLER_H
#define SERVER_MSGHANDLER_H
#include "server.h"

int user_handler(ConnectArg*, char*, int);
int pass_handler(ConnectArg*, char*, int);
int list_handler(ConnectArg*, char*, int);
int syst_handler(ConnectArg*, char*, int);
int type_handler(ConnectArg*, char*, int);
int port_handler(ConnectArg*, char*, int);
int pasv_handler(ConnectArg*, char*, int);
int retr_handler(ConnectArg*, char*, int);
int stor_handler(ConnectArg*, char*, int);
int cwd_handler(ConnectArg*, char*, int);
int pwd_handler(ConnectArg*, char*, int);
int quit_handler(ConnectArg*, char*, int);
int mkd_handler(ConnectArg*, char*, int);
int rmd_handler(ConnectArg*, char*, int);
int rnfr_handler(ConnectArg*, char*, int);
int rnto_handler_refuse(ConnectArg*, char*, int);

int rnto_handler_accept(ConnectArg* args, char* oldpath, char* cmd, int cmdn);
int reducePath(char* path, int n);
#endif //SERVER_MSGHANDLER_H
