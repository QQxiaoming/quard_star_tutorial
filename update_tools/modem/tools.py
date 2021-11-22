import sys
import logging
from modem.const import CRC16_MAP, CRC32_MAP

# Configure logging
logging.basicConfig(format='%(asctime)s [%(levelname)s] %(message)s',
    datefmt='%m/%d/%Y %I:%M:%S %p',
    stream=sys.stdout,
    level=logging.DEBUG)

log = logging.getLogger('modem')

def crc16(data, crc=0):
    crc = (crc << 8) ^ CRC16_MAP[((crc >> 0x08) ^ (data)) & 0xff]
    return crc & 0xffff

def crc32(data, crc=0):
    crc = crc ^ 0xffffffff
    crc = (CRC32_MAP[(crc ^ data) & 0xff] ^ (crc >> 8)) ^ 0xffffffff
    return crc & 0xffffffff
