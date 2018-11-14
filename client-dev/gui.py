import sys
import socket
from PyQt5.QtWidgets import (QWidget, QGridLayout,
    QPushButton, QApplication)


class Example(QWidget):

    def __init__(self, rmt_sock):
        super().__init__()
        self.rmt_sock = rmt_sock
        self.initUI()

    def __del__(self):
        self.rmt_sock.send(b'quit')
        self.rmt_sock.send(b'disconnect')
        self.rmt_sock.close()
        # super().__del__()

    def initUI(self):
        self.rmt_sock.send(b'list_dir')
        print(self.rmt_sock.recv(10086).decode())

        # self.rmt_sock.send(b'disconnect')

        # grid = QGridLayout()
        # self.setLayout(grid)
        #
        # names = ['Cls', 'Bck', '', 'Close',f
        #          '7', '8', '9', '/',
        #         '4', '5', '6', '*',
        #          '1', '2', '3', '-',
        #         '0', '.', '=', '+']
        #
        # positions = [(i,j) for i in range(5) for j in range(4)]
        #
        # for position, name in zip(positions, names):
        #
        #     if name == '':
        #         continue
        #     button = QPushButton(name)
        #     grid.addWidget(button, *position)

        self.move(300, 150)
        self.resize(800, 500)
        self.setWindowTitle('Calculator')
        self.show()




if __name__ == '__main__':
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', 1108))

    app = QApplication(sys.argv)
    ex = Example(sock)
    sys.exit(app.exec_())