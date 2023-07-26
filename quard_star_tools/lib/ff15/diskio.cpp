/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "stdlib.h"
#include "cstring"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
static uint8_t * ram_disk_data = nullptr;
static uint64_t ram_disk_size = 0;

void ff_init(uint8_t * data, uint64_t data_size) {
	ram_disk_data = data;
	ram_disk_size = data_size;
}

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv) {
	case DEV_RAM :
		if (ram_disk_data && ram_disk_size) {
			stat = RES_OK;
		} else {
			stat = STA_NOINIT;
		}
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv) {
	case DEV_RAM :
		if (ram_disk_data && ram_disk_size) {
			stat = RES_OK;
		} else {
			stat = STA_NOINIT;
		}

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;

	switch (pdrv) {
	case DEV_RAM :
		memcpy(buff, ram_disk_data + sector * 512, count * 512);
		res = RES_OK;
		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;

	switch (pdrv) {
	case DEV_RAM :
		memcpy(ram_disk_data + sector * 512, buff, count * 512);
		res = RES_OK;
		return res;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch (pdrv) {
	case DEV_RAM :
		switch(cmd) {
			case GET_SECTOR_SIZE:
				*(DWORD*)buff = 512;
				return RES_OK;
			case GET_SECTOR_COUNT:
				*(DWORD*)buff = ram_disk_size / 512;
				return RES_OK;
			case GET_BLOCK_SIZE:
				*(DWORD*)buff = 1;
				return RES_OK;
			case CTRL_SYNC:
				return RES_OK;
		}
		return RES_PARERR;
	}

	return RES_PARERR;
}

#include "QDate"
#include "QTime"

DWORD get_fattime (void)
{
    QDate date = QDate::currentDate();
	QTime time = QTime::currentTime();
    int year   = date.year();
	int month  = date.month();
	int day    = date.day();
	int hour   = time.hour();
	int minute = time.minute();
	int second = time.second();
	return ((year - 1980) << 25) | (month << 21) | (day << 16) | (hour << 11) | (minute << 5) | (second >> 1);
}
