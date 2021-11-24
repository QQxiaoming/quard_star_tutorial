#!/usr/bin/env python
# -*- coding:utf-8 -*-
import sys
import logging
from PyQt5 import QtWidgets
from mainwindow import MainWindow
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.WARNING)
    app = QtWidgets.QApplication(sys.argv)
    show = MainWindow()
    show.show()
    sys.exit(app.exec_())
