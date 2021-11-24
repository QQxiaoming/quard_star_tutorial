#!/usr/bin/env python
# -*- coding:utf-8 -*-

import os
from PyQt5 import QtCore, QtWidgets
from PyQt5.QtCore import Qt, QThread, pyqtSignal
from PyQt5.QtWidgets import QFileDialog, QMessageBox, QToolTip
from PyQt5.QtGui import QGuiApplication
from ui_mainwindow import Ui_MainWindow
from quard_updater.base import QuardUpdate
from quard_updater.base import VERSION
import telnetlib

class QSUpdaterThread(QThread,QuardUpdate):
    info = pyqtSignal(str,list)

    def __init__(self,filePath):
        super().__init__()
        self.set_file(filePath)
        self.recv_data = bytes()

    def hw_port_io_init(self):
        self.tn = telnetlib.Telnet(host='localhost', port=3441, timeout=30000)

    def hw_port_io_send(self,data):
        self.tn.write(data)
        return len(data)

    def hw_port_io_recv(self,size,timeout):
        timeout = timeout * 100
        while(timeout):
            recv = self.tn.read_some()
            if recv != b'':
                self.recv_data = self.recv_data + recv
            timeout -= 1
            if(len(self.recv_data) >= size):
                data = self.recv_data[:size]
                self.recv_data=self.recv_data[size:]
                return data

    def hw_port_io_recv_until(self,data,timeout):
        self.tn.read_until(data,timeout)

    def hw_port_io_close(self):
        self.tn.close()

    def report(self, cmd, arg):
        self.info.emit(cmd,[arg])

    def run(self):
        self.update_run()

class MainWindow(QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.setWindowTitle('Quard Star Update Tools ' + VERSION)
        screen = QGuiApplication.primaryScreen().geometry()
        size = self.geometry()
        self.move((screen.width() - size.width()) / 2, (screen.height() - size.height()) / 2)
        self.updaterThread = QSUpdaterThread('../output/lowlevelboot/lowlevelboot.bin')
        self.updaterThread.info.connect(self.info_report)
        self.ui.progressBar.setValue(0)
        self.ui.pushButton.clicked.connect(self.clicked_run)

    def clicked_run(self):
        openfile_name = QFileDialog.getOpenFileName(self, '选择文件', '', 'bin files(*.bin)')
        if os.path.exists(openfile_name[0]):
            self.updaterThread.set_file(openfile_name[0])
            self.fsize = os.path.getsize(openfile_name[0])
            self.sendsize = 0
            self.ui.lineEdit.setText(openfile_name[0])
            self.updaterThread.start()

    def info_report(self,cmd, arg):
        if cmd == 'send':
            self.sendsize += arg[0]
            self.ui.progressBar.setValue(self.sendsize*100/self.fsize)
        elif cmd == 'end':
            self.ui.progressBar.setValue(100)
