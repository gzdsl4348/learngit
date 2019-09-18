/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "sdcard_host.h"/* SDCARD HOST drive control */
#include "diskio.h"     /* FatFs lower layer API */
#include "debug_print.h"

/* Definitions of physical drive number for each drive */

#define SD          0   /* Example: Map MMC/SD card to drive number 0 */
#define FLASH       1   /* Example: Map Flash drive to drive number 1*/
#define USB         2   /* Example: Map USB drive to drive number 2 */

client interface sd_host_if  * unsafe sd_hi = NULL;
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv       /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat = RES_PARERR;
    BYTE result;

    switch (pdrv) {
    case SD :
        unsafe{
        result = sd_hi->sd_status(pdrv);
        }

        if (result){
            stat = STA_NOINIT;
        }
        else{
            stat = RES_OK;
          }
        break;

    case FLASH :
      break;
    case USB :
       break;
    }

    return stat;
}


/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv               /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat=RES_PARERR;
    BYTE result;

    switch (pdrv) {
    case SD :

        unsafe{
        result = sd_hi->sd_initialize();
        }
        if (result){
            stat = RES_ERROR;
        }
        else{
            stat = RES_OK;
        }
        break;

    case FLASH :
        break;
    case USB :
        break;
    }
    return stat ;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,      /* Physical drive nmuber to identify the drive */
    BYTE buff[],        /* Data buffer to store read data */
    DWORD sector,   /* Sector address in LBA */
    UINT count      /* Number of sectors to read */
)
{
    DRESULT res = RES_PARERR;
   text_debug("sd read\n");
   switch (pdrv) {
    case SD :
        unsafe{
        res = sd_hi->sd_read(buff, sector, count);
        }
        break;
    case FLASH :
        break;
    case USB :
        break;
    }
    return res;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
    BYTE pdrv,          /* Physical drive nmuber to identify the drive */
    const BYTE buff[],  /* Data to be written */
    DWORD sector,       /* Sector address in LBA */
    UINT count          /* Number of sectors to write */
)
{
    DRESULT res = RES_PARERR;
    text_debug("sd write\n");
    switch (pdrv) {
    case SD :
        unsafe{
        res = sd_hi->sd_write(buff, sector, count);
        }
        break;
    case FLASH :
        break;
    case USB :
        break;
    }


  return res;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    BYTE buff[]     /* Buffer to send/receive control data */
)
{
    DRESULT result = RES_PARERR;
  switch (pdrv) {
    case SD :
        unsafe{
         result = sd_hi->sd_ioctl(buff,cmd);
        }
         break;
    case FLASH :
        break;
    case USB :
        break;

    }
  return result;

}

/*-----------------------------------------------------------------------*/
/* FileSystem Wrapper to provide SD Host Client Interface                */
/*-----------------------------------------------------------------------*/
[[distributable]]
void sd_fs_wrapper(client interface sd_host_if sdhi) {

    unsafe {
    sd_hi = (client sd_host_if * unsafe) &sdhi;
    }
    return;
    while (1) { select{ }}

}
#endif
