import socket
import threading
import time

size = 8192
fexit = False

class RecvThread(threading.Thread):
    def __init__(self, threadID, name, dic):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.dic = dic

    def run(self):
        global fexit
        while(True):
            data = self.dic['sock'].recv(size)
            data = data.decode()

            if(len(data) > 0):
                print(self.name + " : " + data)
            paramL = data.split(' ')
            if(paramL[0] == '227'):
                print(paramL)
                dataL = paramL[1].split('=')
                print(dataL)
                dataL = dataL[1].split(',')
                print(dataL)

                port = int(dataL[4]) * 256 + int(dataL[5][:-1])
                ip = dataL[0] + '.' + dataL[1] + '.' + dataL[2] + '.' + dataL[3]
                if (self.dic['datasock'] != None):
                    print("close datasock")
                    self.dic['datasock'].close()
                    # time.sleep(3)
                if (self.dic['listensock'] != None):
                    print("close listensock")
                    self.dic['listensock'].close()
                try:
                    print(ip, port)
                    datasock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    datasock.connect((ip, port))
                    self.dic['datasock'] = datasock
                    print('datasock bind')
                    # self.dic['datasock'].send('xxxxxxxxxxxxx'.decode())
                except:
                    print('unable to connect server')

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
            msg = input()
            print(self.name + " : " + msg)
            paramL = msg.split(' ')
            if(paramL[0] == 'PORT'):
                try:
                    # tdata = DataThread(2, 'data', dic)
                    # tdata.start()
                    dataL = paramL[1].split(',')
                    ip = dataL[0] + '.' + dataL[1] + '.' + dataL[2] + '.' + dataL[3]
                    port = int(dataL[4]) * 256 + int(dataL[5])
                    print(ip, port)
                    if(self.dic['datasock'] != None):
                        print("close datasock")
                        self.dic['datasock'].close()
                        # time.sleep(3)
                    if(self.dic['listensock'] != None):
                        print("close dataConn")
                        self.dic['listensock'].close()

                    # if(port == 5655):
                    #     port = port - 1
                    self.dic['listensock'] = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    self.dic['listensock'].bind((ip, port))
                    print("bind done")
                    self.dic['listensock'].listen(1)
                    self.dic['sock'].send((msg + '\n').encode())
                    print("send done")
                    # self.dic['datasock'].settimeout(5)
                    connection, address = self.dic['listensock'].accept()
                    # connection.settimeout(5)
                    print("accpted")
                    self.dic['datasock'] = connection
                    continue
                except:
                    print("cannot establish connect")
                    continue
            elif(paramL[0] == 'LIST'):
                self.dic['sock'].send((msg + '\n').encode())
            elif(paramL[0] == 'PASV'):
                if (self.dic['datasock'] != None):
                    print("close datasock")
                    self.dic['datasock'].close()
                    # time.sleep(3)
                if (self.dic['listensock'] != None):
                    print("close dataConn")
                    self.dic['listensock'].close()
                self.dic['sock'].send((msg + '\n').encode())
            elif(paramL[0] == 'STOR'):
                self.dic['sock'].send((msg + '\n').encode())
                try:
                    data = input('input some data')
                    self.dic['datasock'].send(data.encode())
                    self.dic['datasock'].close()
                except:
                    print('trans data failed')
                # self.dic['datasock'].close()
            else:
                self.dic['sock'].send((msg + '\n').encode())


try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('127.0.0.1', 6789))
    dic = {}
    dic['sock'] = sock
    dic['datasock'] = None
    dic['listensock'] = None
    trecv = RecvThread(0, 'recv', dic)
    tsend = SendThread(1, 'send', dic)
    tdata = DataThread(2, 'data', dic)
    tsend.start()
    trecv.start()
    # tdata.start()

    tsend.join()
    trecv.join()
    # tdata.join()
    # while(True):
    #     # data = sock.recv(size)
    #     # while(len(data) > 0):
    #     #     print(data)
    #     #     data = sock.recv(size)
    #     #     print(len(data))
    #     msg = input('send msg: ')
    #     print(msg)
    #     if(msg == 'exit'):
    #         break
    #     if(msg != 'pass'):
    #         sock.send((msg+'\n').encode('utf-8'))
            # sock.send(b'hello\n')
        # sock.send(b'hello\n')
        # data = sock.recv(size)
        # if(len(data)):
        #     print(data)
    print('socket close')
    sock.close()
 
except:
    print("cannot reach the server")
