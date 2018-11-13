import socket
import threading
import time
import re
import argparse

fpid = 3
size = 200086
fexit = False
ipre = re.compile(r'\d+,\d+,\d+,\d+,\d+,\d+')


def try_close_sock(dic, name):
    if (dic[name] != None):
        print('close socket : ' + name)
        dic[name].close()
        dic[name] = None

class RecvThread(threading.Thread):
    def __init__(self, threadID, name, dic):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.dic = dic

    # def try_close_sock(self, name):
    #     if (self.dic[name] != None):
    #         print('close socket : '+name)
    #         self.dic[name].close()
    #         self.dic[name] = None

    def run(self):
        global fexit
        while(True):
            try:
                data = self.dic['sock'].recv(size)
            except:
                print('socket disconnected')
                return
            data = data.decode()
            paramL = data.split(' ')
            code = -1
            if(len(data) > 0):
                code = int(data[0:3])
                self.dic['recvcode'] = code
                print(self.name + " : " + data)
            if(code == 227):
                s, e = ipre.search(data).span()
                print(s, e)
                print(data[s: e])
                dataL = (data[s: e]).split(',')
                print(dataL)
                port = int(dataL[4]) * 256 + int(dataL[5])
                ip = dataL[0] + '.' + dataL[1] + '.' + dataL[2] + '.' + dataL[3]
                try_close_sock(self.dic, 'datasock')
                try_close_sock(self.dic, 'listensock')
                try:
                    print(ip, port)
                    datasock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    datasock.connect((ip, port))
                    self.dic['datasock'] = datasock
                    print('datasock bind')
                except:
                    print('unable to connect server')

class SendFileThread(threading.Thread):
    def __init__(self, threadID, name, dic, filename):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.dic = dic
        self.filename = filename

    def run(self):
        filename = self.filename
        try:
            f = open(filename, 'rb')
            fdata = f.read(5)
            while(len(fdata) > 0):
                self.dic['datasock'].send(fdata)
                print('send data : ', fdata)
                time.sleep(0.5)
                fdata = f.read(5)
            f.close()
        except:
            print('read file failed or connection failed')
        # try:
        #     self.dic['datasock'].send(fdata)
        # except:
        #     print('trans data failed')
        try_close_sock(self.dic, 'datasock')
        try_close_sock(self.dic, 'listensock')


class RecvFileThread(threading.Thread):
    def __init__(self, threadID, name, dic, filename):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.dic = dic
        self.filename = filename

    def run(self):
        filename = self.filename
        try:
            f = open(filename, 'wb')
        except:
            print('RETR: open file failed')

        try:
            fdata = self.dic['datasock'].recv(5)
            while (len(fdata) > 0):
                print('fdata = ', fdata)
                f.write(fdata)
                fdata = self.dic['datasock'].recv(5)
        except:
            print('RETR: trans data failed or write file failed')
        try:
            f.close()
        except:
            print('RETR: close file failed')
        try_close_sock(self.dic, 'datasock')
        try_close_sock(self.dic, 'listensock')
        print(self.name + ' done')


class DataThread(threading.Thread):
    def __init__(self, threadID, name, dic):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.dic = dic

    def run(self):
        while(True):
            try:
                data = self.dic['datasock'].recv(size)
                data = data.decode()
                if(len(data) > 0):
                    print(self.name + " : " + data)
            except:
                pass

class SendThread(threading.Thread):
    def __init__(self, threadID, name, dic):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.dic = dic

    def run(self):
        global fexit
        while (True):
            if(self.dic['sock'] == None):
                return
            msg = input()
            print(self.name + " : " + msg)
            paramL = msg.split(' ')
            if(paramL[0] == 'PORT'):
                try:
                    dataL = paramL[1].split(',')
                    ip = dataL[0] + '.' + dataL[1] + '.' + dataL[2] + '.' + dataL[3]
                    port = int(dataL[4]) * 256 + int(dataL[5])
                    print(ip, port)
                    try_close_sock(self.dic, 'datasock')
                    try_close_sock(self.dic, 'listensock')
                    self.dic['listensock'] = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    self.dic['listensock'].bind((ip, port))
                    print("PORT: bind done")
                    self.dic['listensock'].listen(1)
                    self.dic['sock'].send((msg + '\n').encode())
                    print("PORT: send done")
                    connection, address = self.dic['listensock'].accept()
                    print("PORT: accpted")
                    self.dic['datasock'] = connection
                except:
                    print("PORT: cannot establish connect")
            elif(paramL[0] == 'LIST'):
                self.dic['sock'].send((msg + '\n').encode())
                try:
                    print("geting list data")
                    while(True):
                        data = self.dic['datasock'].recv(200086)
                        if not data:
                            break
                        data = data.decode()
                        if(len(data) > 0):
                            print(data)
                except:
                    print("list: data connect not established")
                try_close_sock(self.dic, 'datasock')
                try_close_sock(self.dic, 'listensock')
            elif(paramL[0] == 'PASV'):
                try_close_sock(self.dic, 'datasock')
                try_close_sock(self.dic, 'listensock')
                self.dic['sock'].send((msg + '\n').encode())
            elif(paramL[0] == 'STOR'):
                filename = input('specify upload file name:')
                self.dic['sock'].send((msg + '\n').encode())
                tsfile = SendFileThread(4, 'sFile', self.dic, filename)
                tsfile.start()
            elif(paramL[0] == 'RETR'):
                # self.dic['recvcode'] = -1
                self.dic['sock'].send((msg + '\n').encode())
                filename = input('specify upload file name:')
                trfile = RecvFileThread(3, 'rFile', self.dic, filename)
                trfile.start()
            elif(paramL[0] == 'QUIT'):
                self.dic['sock'].send((msg + '\n').encode())
                # self.try_close_sock('sock')
                try_close_sock(self.dic, 'sock')
                try_close_sock(self.dic, 'datasock')
                try_close_sock(self.dic, 'listensock')
                return
            else:
                self.dic['sock'].send((msg + '\n').encode())

    def wait_for_sock_recv(self):
        while(True):
            # time.sleep(0.2)
            if(self.dic['recvcode'] > 0):
                return self.dic['recvcode']

    # def try_close_sock(self, name):
    #     if (self.dic[name] != None):
    #         print('close socket : '+name)
    #         self.dic[name].close()
    #         self.dic[name] = None

try:
    parser = argparse.ArgumentParser()
    parser.add_argument('--address', '-a', required = True, type = str, dest = 'addr')
    parser.add_argument('--port', '-p', required = True, type = int, dest = 'port')
    args = parser.parse_args()

    print((args.addr, args.port))

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #sock.connect(('ftp.dlptest.com', 21))
    #sock.connect(('localhost', 21))
    # sock.connect(('47.95.120.180', 21))
    sock.connect((args.addr, args.port))
    dic = {}
    dic['sock'] = sock
    dic['datasock'] = None
    dic['listensock'] = None
    dic['recvcode'] = -1
    trecv = RecvThread(0, 'recv', dic)
    tsend = SendThread(1, 'send', dic)
    # tdata = DataThread(2, 'data', dic)
    tsend.start()
    trecv.start()
    # tdata.start()

    tsend.join()
    trecv.join()

    print('socket close')
        # sock.close()
 
except:
    print("cannot reach the server")
