# FTP

## 环境
- Ubuntu 16.04 LTS
- gcc 5.4.0
- python3.6
    - PyQt5==5.11.3
## Server
### 概述
实现了一个简易FTP服务器,该服务器支持匿名用户.可以对一些基本指令进行正确响应
### 实现指令
实验文档中规定的指令和`REST`,`APPE`两个指令
### 功能
- 不阻塞服务器的大文件传输
- 断点续传
- 多客户端
### 实现
- 主要采用多线程方式,每一个客户端链接对应一个线程,传输文件时新开一个线程,每个客户端链接的多个线程共享同一套文件描述符,通过文件描述符控制线程生命周期.
- 对于每一个客户端链接,循环读取指令,在指令列表里面找到相应的响应函数对客户端进行响应
- 对socket的fd加锁防止多个线程对同一个socket进行读写
### 使用方法
- make
- sudo ./server -port <port> -root <root-dir>
    - 缺少参数时,默认<port> = 21, <root-dir> = \tmp

## Client
### 概述
使用python编写,可以向服务器发送原始指令,也可以使用客户端封装的指令,或者使用gui
支持下面四种模式
- 在控制台中输入原始指令
- 在控制台中输入封装指令
- 通过socket输入封装指令
- gui

### gui的实现
使用命令
` ./bin/client -a \<addr\> -p \<port\> -m remote -lp \<clientport>`
可以使得客户端监听\<clientport>端口,然后gui可以通过链接这个端口操作客户端,并把结果显示在图形界面上

## 实验小结
### Server部分
主要是当server向client发送文件时,如果client主动断开连接,server在对socket进行写操作时,会产生SIGPIPE信号,然后被系统杀死.通过在程序里捕获这个信号,可以解决这个问题.
### Client部分
client的gui时,由于使用了gui与client-backend使用网络交互的模式,遇到了一些线程生命周期控制的问题,另外如果要操作图形界面,必须使用PyQt自带的多线程机制而不能使用pyton的threading库
