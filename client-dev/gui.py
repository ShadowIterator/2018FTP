import sys
import socket
import time
import subprocess
import re
import os
import signal
# from PyQt5.QtWidgets import (QWidget, QGridLayout, QPushButton, QApplication)
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import threading
import random


remote_port = 1159
idir = re.compile(r'\".*\"')

# TODO: need another socket to send directory data

class RecvDataThread(QRunnable):
    def __init__(self, threadID, name, port, widgegt):
        # threading.Thread.__init__(self)
        super(RecvDataThread, self).__init__()
        self.threadID = threadID
        self.name = name
        self.port = port
        self.widget = widgegt

    @pyqtSlot()
    def run(self):
        try:
            datasock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            datasock.connect(('localhost', self.port))
            print('gui: console data connection established')
            try:
                data = b''
                fdata = datasock.recv(5)
                while (len(fdata) > 0):
                    data = data + fdata
                    fdata = datasock.recv(5)
                pass
            except:
                print("recvdata failed")
            datasock.close()
        except:
            print('recv data failed')
        print('gui: recv data: ', data)
        data = data.decode(encoding = 'utf-8')
        # cnt = 0
        try:
            self.widget.ls.clear()
            files = data
            # files.strip('\r\n')
            print('gui: before split: ', files)
            files = files.split('\n')
            print('gui: files =', files)
            for file in files:
                if (not file):
                    continue
                # cnt = cnt + 1
                # if(cnt > 20):
                #     break
                # print('file: ', file)
                print('gui: file ', file)
                file = re.split(r'\s+', file)
                self.widget.appendFile(filename=file[8], permission=file[0], size=file[4])
            # pass
        except:
            print('list index out of range')
        print('recv console data done')


class RecvThread(QRunnable):
    def __init__(self, threadID, name, widget):
        # threading.Thread.__init__(self)
        super(RecvThread, self).__init__()
        self.threadID = threadID
        self.name = name
        self.widget = widget

    @pyqtSlot()
    def run(self):
        print('recvthread start')
        global fexit
        while(True):
            try:
                data = self.widget.rmt_sock.recv(10086)
            except:
                print('recv_thread: socket disconnected')
                break
            data = data.decode()
            if('$stoplisten$' in data):
                break
            paramL = data.split('$')
            if(paramL[0] == 'console'):
                self.widget.consoleInfoEdit.append(data)
            elif(paramL[0] == 'linkto'):
                try:
                    print('linkto$ ', data[7:])
                    port = int(data[7:])
                    # tdata = RecvDataThread(3, 'recv dir', port, self.widget)
                    # tdata.start()
                    tdata = RecvDataThread(3, 'recv dir', port, self.widget)
                    self.widget.threadpool.start(tdata)
                except:
                    print('failed to connect')
            elif(paramL[0] == 'dir'):
                pass
            elif(paramL[0] == 'path'):
                # print('enter path')
                s, e = idir.search(data).span()
                # self.widget.pathLineEdit.setDisabled(False)
                self.widget.pathLineEdit.setText(data[s + 1: e - 1])
                # self.widget.pathLineEdit.setDisabled(True)

            # print(self.name + ' : ' + data)
        print('recv thread quit')

# set


class Example(QWidget):

    def __init__(self):
        super().__init__()
        self.rmt_sock = None
        self.initUI()
        self.threadpool = QThreadPool()


    def connectTo(self, addr, port, remote_port):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        remote_port = random.randint(0, 65535)
        while (True):
            try:
                sock.bind(('localhost', remote_port))
                break
            except:
                remote_port = random.randint(0, 65535)
                pass
        sock.close()
        time.sleep(1)

        self.backend = subprocess.Popen(['python3', 'client.py', '-a', addr, '-p', port, '-m', 'remote', '-lp', str(remote_port)])
        time.sleep(1)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            sock.connect(('localhost', remote_port))
        except:
            print('gui: can not reach endback')
            self.backend.kill()
            self.backend = None
            return False
        self.rmt_sock = sock
        trecv = RecvThread(1, 'recv', self)
        # trecv.start()
        self.threadpool.start(trecv)
        time.sleep(1)
        return True

    def disconnect(self):
        self.sendToRemote('quit')
        time.sleep(1)
        self.sendToRemote('disconnect')
        time.sleep(1)

        self.rmt_sock.close()
        self.rmt_sock = None
        # self.backend.kill()
        # print(self.backend.pid)
        # os.kill(self.backendpid, signal.SIGKILL)
        # self.backend.kill()
        print('disconnect done')

    def __del__(self):
        print('in delete')
        self.disconnect()
        # self.rmt_sock.send(b'quit')
        # self.rmt_sock.send(b'disconnect')
        # print('send msg done')
        # if(self.rmt_sock):
        #     self.rmt_sock.close()
        #     self.rmt_sock = None
        # super().__del__()

    def sendMsg(self, data):
        print('sendMsg: ', data)

    def initUI(self):
        self.mainlayout = QVBoxLayout()
        self.upperlayout = QHBoxLayout()
        self.upperwidget = QWidget()
        self.btnlayout = QHBoxLayout()
        self.btnwidget = QWidget()
        self.pathlayout = QHBoxLayout()
        self.pathwidget = QWidget()

        self.transInfoEdit = QLineEdit()
        self.consoleInfoEdit = QTextEdit()

        self.hostLineEdit = QLineEdit('166.111.80.237')
        self.portLineEdit = QLineEdit('8279')
        self.userLineEdit = QLineEdit('cn2018')
        self.passLineEdit = QLineEdit('ftp')

        # self.hostLineEdit = QLineEdit('ftp.dlptest.com')
        # self.portLineEdit = QLineEdit('21')
        # self.userLineEdit = QLineEdit('dlpuser@dlptest.com')
        # self.passLineEdit = QLineEdit('e73jzTRTNqCN9PYAAjjn')

        # self.hostLineEdit = QLineEdit('123.207.157.213')
        # self.portLineEdit = QLineEdit('8766')
        # self.userLineEdit = QLineEdit('anonymous')
        # self.passLineEdit = QLineEdit('e73jzTRT')


        self.hostLable = QLabel('address:')
        self.portLable = QLabel('port:')
        self.userLable = QLabel('username:')
        self.passLable = QLabel('password:')
        self.connectButton = QPushButton('connect')
        self.downloadButton = QPushButton('download')
        self.uploadButton = QPushButton('upload')
        self.pauseButton = QPushButton('stop')
        self.pathLineEdit = QLineEdit()
        self.enterDirButton = QPushButton('refresh')
        self.mkdLineEdit = QLineEdit()
        self.mkdButton = QPushButton('makedir')
        self.downloadAppendButton = QPushButton('download(append)')
        self.uploadAppendButton = QPushButton('upload(append)')
        self.rmdButton = QPushButton('rmdir')
        self.backButton = QPushButton('to upper dir')
        # self.pathLineEdit.setDisabled(True)
        self.transInfoEdit.setDisabled(True)
        self.consoleInfoEdit.setFocusPolicy(Qt.NoFocus)
        self.pathLineEdit.setFocusPolicy(Qt.NoFocus)
        self.upperwidget.setLayout(self.upperlayout)
        self.btnwidget.setLayout(self.btnlayout)
        self.pathwidget.setLayout(self.pathlayout)

        self.setLayout(self.mainlayout)
        self.mainlayout.addWidget(self.upperwidget)
        self.upperlayout.addWidget(self.hostLable)
        self.upperlayout.addWidget(self.hostLineEdit)
        self.upperlayout.addWidget(self.portLable)
        self.upperlayout.addWidget(self.portLineEdit)
        self.upperlayout.addWidget(self.userLable)
        self.upperlayout.addWidget(self.userLineEdit)
        self.upperlayout.addWidget(self.passLable)
        self.upperlayout.addWidget(self.passLineEdit)
        self.upperlayout.addWidget(self.connectButton)

        # inputText1 = QLineEdit()
        self.mainlayout.addWidget(self.consoleInfoEdit)

        self.mainlayout.addWidget(self.pathwidget)

        self.pathlayout.addWidget(self.pathLineEdit)
        self.pathlayout.addWidget(self.enterDirButton)
        self.pathlayout.addWidget(self.rmdButton)
        self.pathlayout.addWidget(self.backButton)
        self.pathlayout.addWidget(self.mkdLineEdit)
        self.pathlayout.addWidget(self.mkdButton)

        self.ls = QListWidget()
        self.mainlayout.addWidget(self.ls)

        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item1'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item2'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item3'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item4'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item5'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item6'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item7'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item8'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item9'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item0'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item10'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item11'))
        # self.ls.addItem(QListWidgetItem(QIcon('D:\\THU\\Python_dir\\2018FTP\\client-dev\\icon.png'), 'item12'))

        self.mainlayout.addWidget(self.btnwidget)





        self.btnlayout.addWidget(self.downloadButton)
        self.btnlayout.addWidget(self.uploadButton)
        self.btnlayout.addWidget(self.downloadAppendButton)
        self.btnlayout.addWidget(self.uploadAppendButton)
        self.btnlayout.addWidget(self.pauseButton)

        self.mainlayout.addWidget(self.transInfoEdit)

        # self.downloadButton.clicked.connect(self.si_mouse_clicked)
        # self.ls.clicked.connect(self.si_ls_clicked)

        self.connectButton.clicked.connect(self.connectButtonClicked)
        self.enterDirButton.clicked.connect(self.enterDirButtonClicked)
        self.downloadButton.clicked.connect(self.downloadButtonClicked)
        self.uploadButton.clicked.connect(self.uploadButtonClicked)
        self.pauseButton.clicked.connect(self.pauseButtonClicked)
        self.ls.doubleClicked.connect(self.lsDoubleClicked)
        self.mkdButton.clicked.connect(self.mkdButtonClicked)
        self.downloadAppendButton.clicked.connect(self.downloadAppendButtonClicked)
        self.uploadAppendButton.clicked.connect(self.uploadAppendButtonClicked)
        self.ls.clicked.connect(self.lsClicked)
        self.rmdButton.clicked.connect(self.rmdButtonClicked)
        self.backButton.clicked.connect(self.backButtonClicked)

        # fileselect = QFileDialog(self)
        # rtn = fileselect.show()
        # rtn = QFileDialog.getOpenFileName()
        # print(rtn)
        self.move(300, 150)
        self.resize(1000, 500)
        self.setWindowTitle('Calculator')
        self.show()

        # self.rmt_sock.send('''login {username} {password}'''.format(username='dlpuser@dlptest.com', password='e73jzTRTNqCN9PYAAjjn').encode())
        # self.rmt_sock.send(b'list_dir\n')
        # self.rmt_sock.send(b'list_dir\n')
        # self.sendToRemote('list_dir')
        # self.sendToRemote('list_dir')

    def sendToRemote(self, msg):
        try:
            self.rmt_sock.send(msg.encode() + b'\r\n')
            time.sleep(0.1)
        except:
            print('remote socket already closed')

    @pyqtSlot()
    def rmdButtonClicked(self):
        self.sendToRemote('rmd ' + re.split(r'\s+', self.selectFile)[1])
        self.enterDirButtonClicked()
    @pyqtSlot()
    def backButtonClicked(self):
        self.sendToRemote('cd ..\r\n')
        # self.sendToRemote('list_dir\r\n')
        self.enterDirButtonClicked()

    @pyqtSlot()
    def connectButtonClicked(self):
        print('connectB clicked', self.hostLineEdit.text())
        # self.connectButton.setText('disconnect')
        if(self.connectButton.text() == 'connect'):
            self.connectButton.setText('processing')
            addr = self.hostLineEdit.text()
            port = self.portLineEdit.text()
            usr = self.userLineEdit.text()
            pwd = self.passLineEdit.text()
            if(self.connectTo(addr, port, remote_port)):
                self.connectButton.setText('disconnect')
                self.sendToRemote('''login {username} {password}'''.format(username = usr, password = pwd))
                self.enterDirButtonClicked()
            else:
                self.connectButton.setText('connect')

            # self.sendToRemote('list_dir')
            # self.rmt_sock.send('''login {username} {password}'''.format(username = usr, password = pwd).encode())
            # self.rmt_sock.send(b'list_dir')
            # self.rmt_sock.send('''login {username} {password}'''.format(username='dlpuser@dlptest.com', password='e73jzTRTNqCN9PYAAjjn').encode())
            # self.rmt_sock.send(b'list_dir\n')
            # self.rmt_sock.send(b'list_dir\n')
            # self.sendToRemote('list_dir')
            # self.sendToRemote('list_dir')
        else:
            self.connectButton.setText('processing')
            # self.rmt_sock.send(b'quit')
            self.disconnect()
            self.consoleInfoEdit.setText('')
            self.pathLineEdit.setText('')
            self.ls.clear()
            self.connectButton.setText('connect')

    @pyqtSlot()
    def mkdButtonClicked(self):
        # print('enterDir clicked', self.hostLineEdit.text())
        # self.connectButton.setText('mkdButton')
        dirPath = self.mkdLineEdit.text()
        self.sendToRemote('''mkd {dirPath}'''.format(dirPath = dirPath))
        # self.sendToRemote('list_dir')
        self.enterDirButtonClicked()


    @pyqtSlot()
    def enterDirButtonClicked(self):
        # print('enterDir clicked', self.hostLineEdit.text())
        # self.connectButton.setText('disconnect')
        self.sendToRemote('pwd')
        self.sendToRemote('list_dir')

    @pyqtSlot()
    def downloadButtonClicked(self):
        print('download clicked', self.hostLineEdit.text())
        # self.connectButton.setText('disconnect')
        self.downloadFile(re.split(r'\s+', self.selectFile)[1])
    @pyqtSlot()
    def uploadButtonClicked(self):
        print('upload clicked', self.hostLineEdit.text())
        # self.connectButton.setText('disconnect')
        localFile = QFileDialog.getOpenFileName()
        if((not localFile[0]) and (not localFile[1])):
            return
        serverFile = QInputDialog.getText(self, "specify a file name", "file name:", QLineEdit.Normal, localFile[0].split('/')[-1])
        if(not serverFile[1]):
            return
        # print('''upload {server_file} {local_file}'''.format(server_file = serverFile[0], localFile = localFile[0]))
        self.sendToRemote('''upload {server_file} {local_file}'''.format(server_file = serverFile[0], local_file = localFile[0]))
        # self.sendToRemote('list_dir')
        # print('serverfile', serverFile)
        # print(localFile, serverFile)
        # print(localFile[], serverFile)

        # self.sendToRemote('''upload {}''')

    @pyqtSlot()
    def uploadAppendButtonClicked(self):
        print('upload_append clicked', self.hostLineEdit.text())
        # self.connectButton.setText('disconnect')
        localFile = QFileDialog.getOpenFileName()
        if((not localFile[0]) and (not localFile[1])):
            return
        # serverFile = QInputDialog.getText(self, "specify a file name", "file name:", QLineEdit.Normal, localFile[0].split('/')[-1])
        serverFile = QInputDialog.getText(self, "specify a file name", "file name:", QLineEdit.Normal, re.split(r'\s+', self.selectFile)[1])
        if(not serverFile[1]):
            return
        print(re.split(r'\s+', self.selectFile)[2])
        self.uploadFile_append(serverFile[0], localFile[0], re.split(r'\s+', self.selectFile)[2])
        # print('''upload {server_file} {local_file}'''.format(server_file = serverFile[0], localFile = localFile[0]))
        # self.sendToRemote('''upload {server_file} {local_file}'''.format(server_file = serverFile[0], local_file = localFile[0]))
        # self.sendToRemote('list_dir')
        # print('serverfile', serverFile)
        # print(localFile, serverFile)
        # print(localFile[], serverFile)

        # self.sendToRemote('''upload {}''')



    @pyqtSlot()
    def pauseButtonClicked(self):
        print('pause clicked', self.hostLineEdit.text())
        # self.connectButton.setText('disconnect')
        self.sendToRemote('close_data_connection')

    @pyqtSlot(QModelIndex)
    def lsDoubleClicked(self, index):
        print('ls double clicked', index.data())
        file = re.split(r'\s+', index.data())
        if('d' in file[0]):
            # self.rmt_sock.send('''cd {path}'''.format(path = file[1]).encode())
            # self.rmt_sock.send(b'pwd\n')
            # self.rmt_sock.send(b'list_dir\n')
            print('double clicked', file[1])
            self.sendToRemote('''cd {path}'''.format(path = file[1]))
            # self.sendToRemote('pwd')
            # self.sendToRemote('list_dir')
            self.enterDirButtonClicked()

        else:
            self.downloadFile(file[1])
        # print(index.data)
        # self.ls.

    @pyqtSlot(QModelIndex)
    def lsClicked(self, index):
        print('ls double clicked', index.data())
        self.selectFile = index.data()
        # file = re.split(r'\s+', index.data())
        # if('d' in file[0]):
        #     # self.rmt_sock.send('''cd {path}'''.format(path = file[1]).encode())
        #     # self.rmt_sock.send(b'pwd\n')
        #     # self.rmt_sock.send(b'list_dir\n')
        #     self.sendToRemote('''cd {path}'''.format(path = file[1]))
        #     self.sendToRemote('pwd')
        #     self.sendToRemote('list_dir')
        # else:
        #     self.downloadFile(file[1])
        # print(index.data)
        # self.ls.
    @pyqtSlot()
    def downloadAppendButtonClicked(self):
        self.downloadFile_append(re.split('\s+', self.selectFile)[1])

    def downloadFile_append(self, filename):
        try:
            filePath = QFileDialog.getSaveFileName()
            fsize = os.path.getsize(filePath[0])
            print(filePath, fsize)
            self.sendToRemote('''download_append {server_path} {local_path} {start_point}'''.format(server_path = filename, local_path = filePath[0], start_point = fsize))
        except:
            print('download aborted')

    def uploadFile_append(self, serverfile, localfile, sp):
        self.sendToRemote('''upload_append {server_path} {local_path} {start_point}'''.format(server_path = serverfile, local_path = localfile, start_point = sp))

    def downloadFile(self, filename):
        # fileselect = QFileDialog(self)
        # fileselect.show()
        # filePath = QFileDialog.getOpenFileName()
        try:
            filePath = QFileDialog.getSaveFileName()
            print(filePath)
            self.rmt_sock.send('''download {server_path} {local_path} {start_point}'''.format(server_path = filename, local_path = filePath[0], start_point = 0).encode())
        except:
            print('download aborted')

    def appendFile(self, filename, permission, size):
        iconPath = 'file.png'
        if('d' in permission):
            iconPath = 'dir.png'
        self.ls.addItem(QListWidgetItem(QIcon(iconPath), permission + '\t\t' + filename + '\t\t' + str(size)))


if __name__ == '__main__':
    # popen('python3 client.py', '-a' 'ftp.dlptest.com -p 21 -m remote -lp 1121')
    signal.signal(signal.SIGABRT, signal.SIG_IGN)
    app = QApplication(sys.argv)
    ex = Example()
    sys.exit(app.exec_())

# python3 .\client.py -a ftp.dlptest.com -p 21 -m remote -lp 1128
# login dlpuser@dlptest.com e73jzTRTNqCN9PYAAjjn
