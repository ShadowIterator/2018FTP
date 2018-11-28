# Client
## 概述
使用python编写,可以向服务器发送原始指令,也可以使用客户端封装的指令,或者使用gui
## 使用方法
- 向服务器发送原始指令
    - 使用以下两个命令之一
        - ./bin/client -a \<addr\> -p \<port\> -m raw
        - python3 ./src/client.py -a \<addr\> -p \<port\> -m raw
    - 然后在控制台中输入相应指令即可,服务器的返回值会被显示到控制台中
-  客户端封装的指令
    - 使用以下两个命令之一
        - ./bin/client -a \<addr\> -p \<port\> -m console
        - python3 ./src/client.py -a \<addr\> -p \<port\> -m console
    - 然后在控制台中输入相应指令即可,服务器的返回值会被显示到控制台中
    - 封装的指令集有
        - mkd \<dir\>
        - login \<user> \<password>
        - cd \<dir>
        - pwd
        - set_type \<type>
        - upload \<serverpath> \<localpath>
        - upload_append \<serverpath> \<localpath> \<startpoint>
        - download \<serverpath> \<localpath>
        - download_append \<serverpath> \<localpath> 
        \<startpoint>
        - rmd \<dir>
        - list_dir \<dir(省缺为空)>
        - quit
- gui
    - 使用以下两个命令之一
        - ./bin/gui
        - python3 ./src/gui.py 
    - 按照gui中的提示操作即可
    - 双击文件列表中的项,可以下载文件或进入目录

