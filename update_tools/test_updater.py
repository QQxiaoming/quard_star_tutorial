import os
import sys
import logging
import telnetlib
from quard_updater.base import QuardUpdate

class Test(QuardUpdate):
    def __init__(self,filePath):
        super().__init__()
        self.set_file(filePath)
        self.recv_data = bytes()
        self.send = 0

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

if __name__ == '__main__':
    filepath = './lowlevelboot.bin'
    if len(sys.argv) >= 1:
        filepath = sys.argv[1]
    update = Test(filepath)
    update.update_run()
    