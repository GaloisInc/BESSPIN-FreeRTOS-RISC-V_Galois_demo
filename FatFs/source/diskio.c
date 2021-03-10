/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "iceblk.h"
#include <string.h>

#define DEBUG_DISKIO 0

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
uint8_t diskio_buffer_rx[ICEBLK_SECTOR_SIZE] __attribute__ ((aligned(64)));
uint8_t diskio_buffer_tx[ICEBLK_SECTOR_SIZE] __attribute__ ((aligned(64)));


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive		  */
)
{
	(void)pdrv; /* always zero in our case */
	if (IceblkDevInstance.disk_present) {
		return STA_OK;
	} else {
		return STA_NODISK;
	}
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
/**
 * Always OK
 */
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	(void)pdrv; /* always zero in our case */
	return STA_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	(void)pdrv; /* always zero in our case */
	DRESULT res = RES_ERROR;
	#if DEBUG_DISKIO
	printf("disk_read: buff @ 0x%x, sector: %u, count: %u\r\n",
		buff, sector, count);
	#endif

	/* Assuming the buffer is 64-bit aligned, we can pass it freely to IceBlk */
	int devres = iceblk_queue_request(&IceblkDevInstance, ICEBLK_REQ_READ, buff, count, sector);
	switch (devres) {
		case 0:
			res = RES_OK;
			break;
		case 1:
			res = RES_ERROR;
			break;
		case -1:
			res = RES_NOTRDY;
			break;
	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	(void)pdrv; /* always zero in our case */
	DRESULT res = RES_ERROR;
	#if DEBUG_DISKIO
	printf("disk_write: buff @ 0x%x, sector: %u, count: %u\r\n",
		buff, sector, count);
	#endif

	/* Assuming the buffer is 64-bit aligned, we can pass it freely to IceBlk */
	int devres = iceblk_queue_request(&IceblkDevInstance, ICEBLK_REQ_WRITE, (BYTE*)buff, count, sector);
	switch (devres) {
		case 0:
			res = RES_OK;
			break;
		case 1:
			res = RES_ERROR;
			break;
		case -1:
			res = RES_NOTRDY;
			break;
	}

	return res;
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
	(void)pdrv; /* always zero in our case */
	#if DEBUG_DISKIO
	printf("disk_ioctl: buff @ 0x%x, cmd: %u\r\n", buff, cmd);
	#else
	(void)buff;
	#endif

	switch (cmd) {
		case CTRL_SYNC:
			#if DEBUG_DISKIO
			printf("CTRL_SYNC: syncing... (nothing to do here)\r\n");
			#endif
			return RES_OK;
		default:
			printf("Unknown command!\r\n");
			return RES_PARERR;
	}
}

