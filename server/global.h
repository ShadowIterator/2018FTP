//
// Created by shadowiterator on 18-10-27.
//

#ifndef SERVER_GLOBAL_H
#define SERVER_GLOBAL_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include<arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>

#define MAX(a,b) ((a)>(b)? (a) : (b))
#define MIN(a,b) ((a)<(b)? (a) : (b))

#endif //SERVER_GLOBAL_H
