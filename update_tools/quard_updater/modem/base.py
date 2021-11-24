from quard_updater.modem.const import CRC16_MAP, CRC32_MAP
from quard_updater.modem.tools import crc16, crc32
from zlib import crc32 as _crc32


class Modem(object):
    '''
    Base modem class.
    '''

    def __init__(self, getc, putc, report=None):
        self.getc = getc
        self.putc = putc
        self.report = report

    def calc_checksum(self, data, checksum=0):
        '''
        Calculate the checksum for a given block of data, can also be used to
        update a checksum.

            >>> csum = modem.calc_checksum(b'hello')
            >>> csum = modem.calc_checksum(b'world', csum)
            >>> hex(csum)
            '0x3c'

        '''
        return (sum(data) + checksum) % 256

    def calc_crc16(self, data, crc=0):
        '''
        Calculate the 16 bit Cyclic Redundancy Check for a given block of data,
        can also be used to update a CRC.

            >>> crc = modem.calc_crc16(b'hello')
            >>> crc = modem.calc_crc16(b'world', crc)
            >>> hex(crc)
            '0x4ab3'

        '''
        for char in data:
            crc = crc16(char, crc)
        return crc & 0xffff

    def calc_crc32(self, data, crc=0):
        '''
        Calculate the 32 bit Cyclic Redundancy Check for a given block of data,
        can also be used to update a CRC.

            >>> crc = modem.calc_crc32(b'hello')
            >>> crc = modem.calc_crc32(b'world', crc)
            >>> hex(crc)
            '0xf9eb20ad'

        '''
        for char in data:
            crc = crc32(char, crc)
        return crc & 0xffffffff
        #return _crc32(data, crc) & 0xffffffff

    def _check_crc(self, data, crc_mode):
        '''
        Depending on crc_mode check CRC or checksum on data.

            >>> data = self._check_crc(data,crc_mode)
            >>> if data:
            >>>    income_size += len(data)
            >>>    stream.write(data)
            ...

        In case the control code is valid returns data without checksum/CRC,
        or returns False in case of invalid checksum/CRC
        '''
        if crc_mode:
            csum = (data[-2] << 8) + data[-1]
            data = data[:-2]
            mine = self.calc_crc16(data)
            if csum == mine:
                return data
        else:
            csum = data[-3]
            data = data[:-1]
            mine = self.calc_checksum(data)
            if csum == mine:
                return data
        return False
