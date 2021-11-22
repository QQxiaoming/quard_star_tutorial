import os
import sys
import string
import logging
import time
import telnetlib
from modem.protocol.xmodem import XMODEM

class QuarUpdate:
    def __init__(self,filePath):
        self.tn = None
        self.recv_data = bytes()
        self.send = 0
        self.filePath = filePath
        self.fsize = os.path.getsize(self.filePath)
        self.xmodem = XMODEM(self.getc, self.putc, self.report)

    def getc(self, size, timeout=1):
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

    def putc(self, data, timeout=1):
        return self.tn.write(data)

    def report(self, cmd, arg):
        if cmd == 'send':
            self.send += arg
            num = int(self.send/int(self.fsize/64))
            if num >= 0:
                sys.stdout.write('#' * num)
                self.send -= num * (int(self.fsize/64))
        elif cmd == 'end':
            def hum_convert(value):
                 units = ["B", "KB", "MB", "GB", "TB", "PB"]
                 size = 1024.0
                 for i,item in enumerate(units):
                     if (value / size) < 1:
                         return "%.2f%s" % (value, item)
                     value = value / size
            sys.stdout.write(' '+hum_convert(self.fsize)+'\n')
        sys.stdout.flush()

    def run(self):
        self.tn = telnetlib.Telnet(host='localhost', port=3441, timeout=30000)
        self.tn.read_until(b'UPDATE...\r\n', 30000)
        self.tn.write(b'Q')
        logging.info('Connect Quard Star MaskROM Success! Please wait...')
        time.sleep(0.01)
        with open(self.filePath, 'rb') as (stream):
            self.xmodem.send(stream)
        self.tn.read_until(b'Quard Star Updater', 30000)
        self.tn.close()
        logging.info('update firmware done!')

if __name__ == '__main__':
    logging.getLogger("modem").setLevel(logging.WARNING)
    update = QuarUpdate('../output/lowlevelboot/lowlevelboot.bin')
    update.run()
    