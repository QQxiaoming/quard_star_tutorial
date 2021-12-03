import os
import sys
import string
import logging
import time
from quard_updater.modem.protocol.xmodem import XMODEM
from abc import abstractmethod

VERSION = 'V0.1.0'

class QuardUpdate(object):
    def __init__(self):
        super().__init__()
        self.xmodem = XMODEM(self.__getc, self.__putc, self.__report)
        self.filePath = None
        self.fsize = 0

    def __getc(self, size, timeout=1):
        return self.hw_port_io_recv(size,timeout)

    def __putc(self, data, timeout=1):
        size = self.hw_port_io_send(data)
        return data[:size]

    def __report(self, cmd, arg):
        self.report(cmd, arg)

    def set_file(self,filePath):
        self.filePath = filePath
        if os.path.exists(self.filePath):
            self.fsize = os.path.getsize(self.filePath)

    def update_run(self):
        if self.fsize:
            self.hw_port_io_init()
            self.hw_port_io_recv_until(b'UPDATE...\r\n',30000)
            self.hw_port_io_send(b'Q')
            logging.info('Connect Quard Star MaskROM Success! Please wait...')
            time.sleep(0.01)
            with open(self.filePath, 'rb') as (stream):
                self.xmodem.send(stream)
            self.hw_port_io_recv_until(b'Quard Star Updater', 30000)
            self.hw_port_io_close()
            logging.info('update firmware done!')
            return True
        return False

    @abstractmethod
    def hw_port_io_init(self):
        pass

    @abstractmethod
    def hw_port_io_send(self,data):
        pass

    @abstractmethod
    def hw_port_io_recv(self,size,timeout):
        pass

    @abstractmethod
    def hw_port_io_recv_until(self,data,timeout):
        pass

    @abstractmethod
    def hw_port_io_close(self):
        pass

    def report(self, cmd, arg):
        pass
