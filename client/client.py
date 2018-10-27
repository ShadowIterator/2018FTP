import socket
import threading

size = 8192
fexit = False

class RecvThread(threading.Thread):
    def __init__(self, threadID, name, sock):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.sock = sock

    def run(self):
        global fexit
        while(True):
            data = self.sock.recv(size)
            data = data.decode()
            # print(type(data), data)
            # print(data == 'squit')
            if(len(data) > 0):
                print(self.name + " : " + data)

            if(data == 'squit'):
                print("quit!")
                # fexit = True
                return

class SendThread(threading.Thread):
    def __init__(self, threadID, name, sock):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.sock = sock

    def run(self):
        global fexit
        while (True):
            msg = input()
            # print(msg)
            if (msg == 'exit'):
                self.sock.send('Q\n'.encode())
                print('exit!')
                return
                # return
            print(self.name + " : " + msg)
            self.sock.send((msg + '\n').encode())


try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', 6789))
    trecv = RecvThread(0, 'recv', sock)
    tsend = SendThread(1, 'send', sock)
    tsend.start()
    trecv.start()

    tsend.join()
    trecv.join()
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
