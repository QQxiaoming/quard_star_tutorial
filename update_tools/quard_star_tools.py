import os
import sys
import string
import logging
import time
import telnetlib
from modem.protocol.xmodem import XMODEM

class QuarUpdate:
    def __init__(self):
        self.tn = None
        self.recv_data = bytes()
        self.xmodem = XMODEM(self.getc, self.putc)

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
        print(data)

    def putc(self, data, timeout=1):
        return self.tn.write(data)

    def run(self):
        self.tn = telnetlib.Telnet(host='localhost', port=3441, timeout=30000)
        self.tn.read_until(b'UPDATE...\r\n', 30000)
        self.tn.write(b'Q')
        logging.info('Connect Quard Star MaskROM Success! Please wait...')
        time.sleep(0.01)
        with open('../output/lowlevelboot/lowlevelboot.bin', 'rb') as (stream):
            self.xmodem.send(stream)
        self.tn.close()
        logging.info('update firmware done!')

if __name__ == '__main__':
    logging.getLogger("modem").setLevel(logging.WARNING)
    update = QuarUpdate()
    update.run()
    