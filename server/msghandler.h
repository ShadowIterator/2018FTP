//
// Created by shadowiterator on 18-10-27.
//

#ifndef SERVER_MSGHANDLER_H
#define SERVER_MSGHANDLER_H
#include "server.h"

int user_handler(ConnectArg*, char*);
int pass_handler(ConnectArg*, char*);
int list_handler(ConnectArg*, char*);
int syst_handler(ConnectArg*, char*);
int type_handler(ConnectArg*, char*);

#endif //SERVER_MSGHANDLER_H
