import os
import sys
import string
import logging
import time
import telnetlib
from modem.protocol.xmodem import XMODEM

tn = None
recv_data = bytes()

def getc(size, timeout=1):
    global tn
    global recv_data
    timeout = timeout * 100
    while(timeout):
        recv = tn.read_some()
        if recv != b'':
            recv_data = recv_data + recv
        timeout -= 1
        if(len(recv_data) >= size):
            data = recv_data[:size]
            recv_data=recv_data[size:]
            return data
    print(data)

def putc(data, timeout=1):
    global tn
    return tn.write(data)

if __name__ == '__main__':
    logging.getLogger("modem").setLevel(logging.WARNING)
    xmodem = XMODEM(getc, putc)
    tn = telnetlib.Telnet(host='localhost', port=3441, timeout=30000)
    tn.read_until(b'UPDATE...\r\n', 30000)
    tn.write(b'Q')
    logging.info('Connect Quard Star MaskROM Success! Please wait...')
    time.sleep(0.01)
    with open('../output/lowlevelboot/lowlevelboot.bin', 'rb') as (stream):
        xmodem.send(stream)
    tn.close()
    logging.info('update firmware done!')
