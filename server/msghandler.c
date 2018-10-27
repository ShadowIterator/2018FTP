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
    for(int p = 0;; ++p, ++rtn)
    {
        while(cmd[p] && cmd[p] != ' ')
            ++p;
        while(cmd[p] && cmd[p] == ' ')
            ++p;

        if(!cmd[p])
            return rtn;
    }
}

int getParam(char* cmd, int k, int* len)
{
    int p = 0;
    for(int i = 0; i < k; ++i)
    {
        while(cmd[p] && cmd[p] != ' ')
            ++p;
        while(cmd[p] && cmd[p] == ' ')
            ++p;
        if(!cmd[p])
            return -1;
    }
    *len = p;
    while(cmd[p] && cmd[p] != ' ') ++p;
    p -= (*len);
    (*len)^=p^=(*len)^=p;
    return p;
}

int checkParamterN(char* cmd, int n)
{
//    int len = 0;
//    int p;
//    return (p = getParam(cmd, n, &len)) > 0 && getParam(&cmd[p + len], 1, &len) == -1;
    return countParam(cmd) == n;
}

int user_handler(ConnectArg* args, char* cmd)
{
    int len = 0;
    int p;
    char* msg;
    int code;
    if(!checkParamterN(cmd, 1))
//        sendFmtMsg(args, "only support anonymous",0 ,530);
    {
        msg = " expected exact 1 parameter";
        code = 530;
        p = -1;
    }
    else
    {
        p = getParam(cmd, 1, &len);
        if(sistrcmp("anonymous", cmd, 0, p, len) != 0)
        {
            msg = " only support anonymous";
            code = 530;
            p = -1;
        }
    }
    if(p == -1)
        sendFmtMsg(args, msg, 0, code);
    else
    {
        set_cmd_status_all(args, CMD_DISABLE);
        set_cmd_status(args, PASS, CMD_ENABLE);
//        sendFmtMsg(args, &cmd[p], len, 100);
        sendFmtMsg(args, " password?", 0, 331);
    }
    return 0;
}

int pass_handler(ConnectArg* args, char* cmd)
{
    int len = 0;
    int p;
    char* msg;
    int code;
    if(!checkParamterN(cmd, 1))
    {
//        msg = "expected exact 1 parameter";
//        code = 530;
//        p = -1;
        sendFmtMsg(args, "expected exact 1 parameter", 0, 530);
        return 0;
    }
    else
    {
//        p = getParam(cmd, 1, &len);
//        valid!
        set_cmd_status_all(args, CMD_ENABLE);
        set_cmd_status(args, USER, CMD_DISABLE);
        set_cmd_status(args, PASS, CMD_DISABLE);
        sendFmtMsg(args, "-login!", 0, 230);
        sendFmtMsg(args, " welcome", 0, 230);
    }

    return 0;
}

int list_handler(ConnectArg* args, char* cmd)
{
    int p = countParam(cmd);
    if(p > 1)
    {
        sendFmtMsg(args, " too many args", 0, 530);
        return 0;
    }
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
        sendFmtMsg(args, msg, 0, 230);
    }
    return 0;
}

int syst_handler(ConnectArg* args, char* cmd)
{
    sendFmtMsg(args, " UNIX Type:L8", 0, 215);
    return 0;
}

int type_handler(ConnectArg* args, char* cmd)
{
    if(countParam(cmd) != 1)
    {
        sendFmtMsg(args, " expected 1 parameter", 0, 530);
        return 0;
    }
    int p;
    int len;
    p = getParam(cmd, 1, &len);
    if(sistrcmp("I", cmd, 0, p, len) != 0)
    {
        sendFmtMsg(args, "expected I as parameter", 0, 530);
        return 0;
    }
    else
    {
        sendFmtMsg(args, " Type set to I", 0, 215);
        return 0;
    }
}