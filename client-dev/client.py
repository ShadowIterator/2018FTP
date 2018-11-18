import socket
import threading
import time
import re
import argparse

fpid = 3
size = 200086
fexit = False
ipre = re.compile(r'\d+,\d+,\d+,\d+,\d+,\d+')
idir = re.compile(r'\".*\"')

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
                print('recv_thread: socket disconnected')
                return
            if(self.dic['remotesock'] != None):
                self.dic['remotesock'].send(b'console$'+data)
            data = data.decode()
            paramL = data.split(' ')
            code = -1
            if(len(data) > 0):
                code = int(data[0:3])
                self.dic['recvcode'] = code
                print(self.name + " : " + data)
            if('227' in data):
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
            if('257' in data):
                s, e = idir.search(data).span()
                # print('dir: ', s, e, data[s: e])
                try:
                    self.dic['remotesock'].send(b'path$'+data[s: e].encode())
                except:
                    print('can not reach remote gui')

class SendFileThread(threading.Thread):
    def __init__(self, threadID, name, dic, filename, sp):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.dic = dic
        self.filename = filename
        self.sp = sp

    def run(self):
        filename = self.filename
        try:
            f = open(filename, 'rb')
            f.seek(self.sp)
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
    def __init__(self, threadID, name, dic, filename, method):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.dic = dic
        self.filename = filename
        self.method = method

    def run(self):
        filename = self.filename
        try:
            if(self.method == 'append'):
                f = open(filename, 'ab')
            else:
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
            elif(paramL[0] == 'APPE'):
                filename = input('specify upload file name:')
                self.dic['sock'].send((msg + '\n').encode())
                tsfile = SendFileThread(4, 'sFile', self.dic, filename)
                tsfile.start()
            elif(paramL[0] == 'RETR'):
                # self.dic['recvcode'] = -1
                self.dic['sock'].send((msg + '\n').encode())
                filename = input('specify upload file name:')
                trfile = RecvFileThread(3, 'rFile', self.dic, filename, 'append')
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


class SIFtp:
    def __init__(self):

        self.dic = {
            'sock': None,
            'datasock': None,
            'listensock': None,
            'remotesock': None,
            'recvcode': -1
        }
        self.mode = 'console'

    def mkd(self, dirPath):
        self.sendMsg('MKD ' + dirPath)

    def connect(self, addr, port):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((addr, port))
        self.dic['sock'] = sock
        print('ftp: connection established')

    def listen_raw(self):
        # trecv = RecvThread(0, 'recv', self.dic)
        tsend = SendThread(1, 'send', self.dic)
        tsend.start()
        # trecv.start()

        tsend.join()
        # trecv.join()

    def show_response(self):
        trecv = RecvThread(0, 'recv', self.dic)
        trecv.start()
        # trecv.join()

    def sendMsg(self, msg):
        self.dic['sock'].send((msg + '\n').encode())

    def login(self, usr, pwd):
        self.sendMsg('USER ' + usr)
        self.sendMsg('PASS ' + pwd)

    def pwd(self):
        self.sendMsg('PWD')

    def request_data_connection(self):
        try_close_sock(self.dic, 'datasock')
        try_close_sock(self.dic, 'listensock')
        self.sendMsg('PASV')
        # time.sleep(0.1)
        while(not self.dic['datasock']):
            time.sleep(0.1)


    def cd(self, dir):
        self.sendMsg('CWD ' + dir)

    def upload(self, spath, filename):
        self.set_type('I')
        self.request_data_connection()
        # filename = input('specify upload file name:')
        # self.dic['sock'].send((msg + '\n').encode())
        self.sendMsg('STOR ' + spath)
        tsfile = SendFileThread(4, 'sFile', self.dic, filename, 0)
        tsfile.start()

    def set_type(self, tp):
        self.sendMsg('TYPE ' + tp)

    def upload_append(self, spath, filename, sp):
        self.set_type('I')
        self.request_data_connection()
        # filename = input('specify upload file name:')
        # self.dic['sock'].send((msg + '\n').encode())
        self.sendMsg('APPE ' + spath)
        tsfile = SendFileThread(4, 'sFile', self.dic, filename, int(sp))
        tsfile.start()

    def download(self, spath, filename, sp):
        self.set_type('I')
        self.request_data_connection()
        # self.dic['sock'].send((msg + '\n').encode())
        # filename = input('specify upload file name:')
        self.sendMsg('REST ' + sp)
        self.sendMsg('RETR ' + spath)
        trfile = RecvFileThread(3, 'rFile', self.dic, filename, 'rewrite')
        trfile.start()

    def download_append(self, spath, filename, sp):
        self.set_type('I')
        self.request_data_connection()
        # self.dic['sock'].send((msg + '\n').encode())
        # filename = input('specify upload file name:')

        self.sendMsg('REST ' + sp)
        self.sendMsg('RETR ' + spath)
        trfile = RecvFileThread(3, 'rFile', self.dic, filename, 'append')
        trfile.start()

    def list_dir(self, spath = ''):
        # self.dic['sock'].send((msg + '\n').encode())
        self.request_data_connection()
        self.sendMsg('LIST ' + spath)
        try:
            # print("geting list data")
            # while (True):
            #     data = self.dic['datasock'].recv(200086)
            #     if not data:
            #         break
            #     data = data.decode()
            #     if (len(data) > 0):
            #         print(data)
            data = b''
            fdata = self.dic['datasock'].recv(5)
            while (len(fdata) > 0):
                # print('fdata = ', fdata)
                data = data + fdata
                fdata = self.dic['datasock'].recv(5)
            pass
        except:
            print("list: data connect not established")
        print(data.decode())
        if(self.mode == 'remote'):
            self.dic['remotesock'].send(b'dir$'+data)
        try_close_sock(self.dic, 'datasock')
        try_close_sock(self.dic, 'listensock')

    def quit(self):
        self.sendMsg('QUIT')
        try_close_sock(self.dic, 'datasock')
        try_close_sock(self.dic, 'listensock')
        try_close_sock(self.dic, 'sock')

    def depatch(self, cmd):
        params = cmd.split(' ')
        func = getattr(self, params[0], None)
        if(not callable(func)):
            print('invalid method')
            return
        func(*params[1:])

    def listen_cmd(self):
        while(self.dic['sock'] != None):
            cmd = input()
            self.depatch(cmd)

    def close_data_connection(self):
        try_close_sock(self.dic, 'datasock')
        # self.quit()

    def listen_remote(self, listen_port):
        print('waiting for remote connection...')
        rmt_listensock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        rmt_listensock.bind(('localhost', listen_port))
        rmt_listensock.listen(1)
        connection, address = rmt_listensock.accept()
        self.dic['remotesock'] = connection
        print('remote: connection established')
        try:
            # while(True):
            #     cmd = connection.recv(10086).decode()
            #     print('remote_recv: ' + cmd)
            #     if(cmd.decode() == 'disconnect'):
            #         connection.close()
            #         rmt_listensock.close()
            #         break
            #     else:
            #         self.depatch(cmd)
            pass
        except:
            print('abnormal disconnect')
        while(True):
            cmd = connection.recv(10086)

            print(b'remote_recv: ' + cmd)
            cmd = cmd.decode()
            cmd = cmd.strip('\r\n')
            if(cmd == 'disconnect'):
                connection.send(b'$stoplisten$')
                while(True):
                    pass
                # connection.close()
                # rmt_listensock.close()
                # break
            elif(cmd == 'getdata'):
                connection.send(self.last_data)
            elif(cmd == 'closeconfirm'):
                connection.send(b'$closeGUI$')
            else:
                self.depatch(cmd)
        # self.quit()


    def run(self, mode, listen_port = 2324):
        self.mode = mode
        ftp.show_response()
        if(mode == 'console'):
            self.listen_cmd()
        elif(mode == 'raw'):
            self.listen_raw()
        elif(mode == 'remote'):
            self.listen_remote(listen_port)
        else:
            print('invalid mode')


# try:
parser = argparse.ArgumentParser()
parser.add_argument('--address', '-a', required = True, type = str, dest = 'addr')
parser.add_argument('--port', '-p', required = True, type = int, dest = 'port')
parser.add_argument('--mode', '-m', required = True, type = str, dest = 'mode')
parser.add_argument('--listen_port', '-lp', required = False, type = int, dest = 'listen_port')
args = parser.parse_args()

print((args.addr, args.port))

ftp = SIFtp()
# ftp.connect(args.addr, args.port)
# ftp.run_raw()
# ftp.show_response()
# ftp.listen_cmd()
# if(args.mode != 'remote'):
ftp.connect(args.addr, args.port)
ftp.run(args.mode, args.listen_port)
print('socket close')
#
# except:
#     print("cannot reach the server")


# python3 .\client.py -a ftp.dlptest.com -p 21
# login dlpuser@dlptest.com e73jzTRTNqCN9PYAAjjn