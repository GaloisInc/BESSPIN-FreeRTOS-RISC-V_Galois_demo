/*-----------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs*/
/* RAMDisk implementation */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include <stdio.h>
#include <string.h>

#define DEBUG_DISKIO_RAM 0

#define SECTOR_SIZE 512

#ifndef RAMDISK_NUM_SECTORS
#error "Configure the RAM Disk size by defining RAMDISK_NUM_SECTORS (number of 512 byte sectors)"
#endif

static BYTE rawdisk[RAMDISK_NUM_SECTORS * SECTOR_SIZE];

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive		  */
)
{
  (void)pdrv; /* always zero in our case */
  return STA_OK;
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
  DWORD start_offset  = sector * SECTOR_SIZE;
  DWORD bytes_to_read = ((DWORD) count) * SECTOR_SIZE;

  (void)pdrv; /* always zero in our case */

#if DEBUG_DISKIO_RAM
  printf("diskio_ram/disk_read: buff @ %08x, sector: %08x, count: %u\n",
	 buff, sector, count);
  printf ("diskio_ram: returned %u bytes from sector at offset %08x is\n", bytes_to_read, start_offset);
  for (DWORD j = 0; j < bytes_to_read; j++)
    {
      printf ("%c ", rawdisk[start_offset + j]);
    }
  printf ("\n");
#endif

  for (DWORD j = 0; j < bytes_to_read; j++)
    {
      buff[j] = rawdisk[start_offset + j];
    }
  return RES_OK;
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
  DWORD start_offset   = sector * SECTOR_SIZE;
  DWORD bytes_to_write = ((DWORD) count) * SECTOR_SIZE;

  (void)pdrv; /* always zero in our case */

#if DEBUG_DISKIO_RAM
  printf("diskio_ram/disk_write: buff @ %08x, sector: %08x, count: %u\n",
	 buff, sector, count);
  printf ("diskio_ram: writing %u bytes to sector from offset %08x is\n", bytes_to_write, start_offset);
  for (DWORD j = 0; j < bytes_to_write; j++)
    {
      printf ("%c ", buff[j]);
    }
  printf ("\n");
#endif

  for (DWORD j = 0; j < bytes_to_write; j++)
    {
      rawdisk[start_offset + j] = buff[j];
    }
  return RES_OK;
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
  
  DWORD *dwptr = (DWORD *) buff;
  WORD   *wptr = (WORD *) buff;

  switch (cmd) {
  case CTRL_SYNC:
    return RES_OK;

  case GET_SECTOR_COUNT:
    *dwptr = RAMDISK_NUM_SECTORS;
    return RES_OK;

  case GET_SECTOR_SIZE:
    *wptr = SECTOR_SIZE;
    return RES_OK;

  case GET_BLOCK_SIZE:
    *dwptr = 1;
    return RES_OK;

  default:
#if DEBUG_DISKIO_RAM
    printf("diskio_ram/ioctl: Unknown command %d\n", (int) cmd);
#endif
    return RES_PARERR;
  }
}

