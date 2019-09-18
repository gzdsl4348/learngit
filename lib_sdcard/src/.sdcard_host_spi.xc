// Copyright (c) 2015, XMOS Ltd, All rights reserved
/*-----------------------------------------------------------------------
* MMCv3/SDv1/SDv2 (in SPI mode) control module
*
* Features and Limitations:
* -------------------------
* No Media Change Detection- Application program must re-mount the
* volume after media change or it results a hard error.
*
*-----------------------------------------------------------------------*/
#include <xs1.h>
#include <xclib.h>
#include <string.h>
#include <print.h>
#include <platform.h>

#include "spi.h"
#include "sdcard_host.h"


///** Status of SD Interface Functions **/
typedef unsigned char   MDSTATUS;

unsigned char cardType; /* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */
MDSTATUS stat; /* Disk status */


/** Return values from SD Transcations **/
typedef enum {
        MD_OK = 0,             /* 0: Successful */
        MD_ERROR,              /* 1: R/W Error */
        MD_WRPRT,              /* 2: Write Protected */
        MD_NOTRDY,             /* 3: Not Ready */
        MD_PARERR              /* 4: Invalid Parameter */
} MDRESULT;



#define DEVICE_ID 0
#define LOW_SPEED_IN_KHZ 250000
#define HIGH_SPEED_IN_KHZ 500000
#define IF_GAP 1000
#define MODE SPI_MODE_3

/** Status of SD Interface Functions **/


#define ST_INIT                0x00
#define ST_NOINIT              0x01    /* Drive not initialized */
#define ST_NODISK              0x02    /* No medium in the drive */
#define ST_PROTECT             0x04    /* Write protected */

/** Miscellaneous Functions - Control Code  **/
#define CL_SYNC             0       /* Flush disk cache (for write functions) */
#define SECTOR_COUNT        1       /* Get media size (for only f_mkfs()) */
#define SECTOR_SIZE         2       /* Get sector size (for multiple sector size (_MAX_SS >= 1024)) */
#define BLOCK_SIZE          3       /* Get erase block size (for only f_mkfs()) */
#define ERASE_SECTOR        4       /* Force erased a block of sectors (for only _USE_ERASE) */
#define TOT_SIZE            5



client interface spi_master_if  * unsafe sd_spi = NULL;

unsigned int ss_speed;
void dly_us(int n)
{
  timer tmr;
  unsigned t;

  tmr :> t;
  t += 100*n;
  tmr when timerafter(t) :> void;
}

#define DLY_US(n) { dly_us(n); }    /* Delay n microseconds */

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* MMC/SD command (SPI mode) */
#define CMD0   (0)     /* GO_IDLE_STATE */
#define CMD1   (1)     /* SEND_OP_COND */
#define ACMD41 (0x80+41) /* SEND_OP_COND (SDC) */
#define CMD8   (8)     /* SEND_IF_COND */
#define CMD9   (9)     /* SEND_CSD */
#define CMD10  (10)    /* SEND_CID */
#define CMD12  (12)    /* STOP_TRANSMISSION */
#define CMD13  (13)    /* SEND_STATUS */
#define ACMD13 (0x80+13) /* SD_STATUS (SDC) */
#define CMD16  (16)    /* SET_BLOCKLEN */
#define CMD17  (17)    /* READ_SINGLE_BLOCK */
#define CMD18  (18)    /* READ_MULTIPLE_BLOCK */
#define CMD23  (23)    /* SET_BLOCK_COUNT */
#define ACMD23 (0x80+23)  /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)    /* WRITE_BLOCK */
#define CMD25  (25)    /* WRITE_MULTIPLE_BLOCK */
#define CMD41  (41)    /* SEND_OP_COND (ACMD) */
#define CMD55  (55)    /* APP_CMD */
#define CMD58  (58)    /* READ_OCR */

/* Card type flags (CardType) */
#define CT_MMC    0x01    /* MMC ver 3 */
#define CT_SD1    0x02    /* SD ver 1 */
#define CT_SD2    0x04    /* SD ver 2 */
#define CT_SDC    (CT_SD1|CT_SD2)  /* SD */
#define CT_BLOCK  0x08    /* Block addressing */

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

unsafe static
int wait_ready (unsigned char drv)  /* 1:OK, 0:Timeout */
{
  unsigned char d[1];
  unsigned int tmr;

  for (tmr = 5000; tmr; tmr--)
  {  /* Wait for ready in timeout of 500ms */
      d[0] = sd_spi->transfer8(0xFF);
    if (d[0] == 0xFF) break;
    DLY_US(100);
  }
  return tmr ? 1 : 0;
}

/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

unsafe static
void deselect (unsigned char drv)
{
 /* Dummy clock (force DO hi-z for multiple slave SPI) */
  sd_spi->transfer8(0xFF);
  sd_spi->end_transaction(1000);
}

/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/

unsafe static
int Select (unsigned char drv)  /* 1:OK, 0:Timeout */
{

  sd_spi->begin_transaction(0, HIGH_SPEED_IN_KHZ, MODE);
  /* Dummy clock (force DO enabled) */
   sd_spi->transfer8(0xFF);


  if (wait_ready(drv))return 1;  /* OK */
  deselect(drv);
  return 0;      /* Failed */
}

/*-----------------------------------------------------------------------*/
/* Send a command packet to the card                                     */
/*-----------------------------------------------------------------------*/

unsafe static
unsigned char send_cmd (unsigned char drv,    /* Returns command response (bit7==1:Send failed)*/
  unsigned char cmd,    /* Command byte */
  unsigned long arg    /* Argument */
)
{
  unsigned char n, d[1], buf[6];

  if (cmd & 0x80)
  {  /* ACMD<n> is the command sequense of CMD55-CMD<n> */
    cmd &= 0x7F;
    n = send_cmd(drv, CMD55, 0);
    if (n > 1) return n;
  }

  /* Select the card and wait for ready */
  deselect(drv);
  if (!Select(drv)) return 0xFF;

  /* Send a command packet */
  buf[0] = 0x40 | cmd;      /* Start + Command index */
  buf[1] = arg >> 24;    /* Argument[31..24] */
  buf[2] = arg >> 16;    /* Argument[23..16] */
  buf[3] = arg >> 8;    /* Argument[15..8] */
  buf[4] = arg;        /* Argument[7..0] */
  n = 0x01;            /* Dummy CRC + Stop */
  if (cmd == CMD0) n = 0x95;    /* (valid CRC for CMD0(0)) */
  if (cmd == CMD8) n = 0x87;    /* (valid CRC for CMD8(0x1AA)) */
  buf[5] = n;

   for(unsigned j=0;j<6;j++){
     sd_spi->transfer8(buf[j]);}

  /* Receive command response */
  /* Skip a stuff byte when stop reading */
  if (cmd == CMD12)
     d[0] = sd_spi->transfer8(0xFF);

   n = 10;                /* Wait for a valid response in timeout of 10 attempts */

  do{
    d[0] = sd_spi->transfer8(0xFF);
  } while ((d[0] & 0x80) && --n);

  return d[0];      /* Return with the response value */
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from the card                                   */
/*-----------------------------------------------------------------------*/
#pragma unsafe arrays
static inline
int rcvr_datablock (unsigned char drv,  /* 1:OK, 0:Failed */
  unsigned char buff[],      /* Data buffer to store received data */
  unsigned int btr      /* Byte count */
)
{
  unsigned char d[2];
  unsigned int tmr;

unsafe {
  for (tmr = 1000; tmr; tmr--)
  {  /* Wait for data packet in timeout of 100ms */
    d[0] = sd_spi->transfer8(0xFF);
    if (d[0] != 0xFF) break;
    DLY_US(100);
  }

  if (d[0] != 0xFE) return 0;    /* If not valid data token, return with error */

   /* Receive the data block into buffer */
   for(unsigned j=0;j<btr;j++){
      buff[j]= sd_spi->transfer8(0xFF);}

  /* Discard CRC */
  d[0] = sd_spi->transfer8(0xFF);
  d[0] = sd_spi->transfer8(0xFF);

}
  return 1;            /* Return with success */
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to the card                                        */
/*-----------------------------------------------------------------------*/
#pragma unsafe arrays
static inline
int xmit_datablock (unsigned char drv,  /* 1:OK, 0:Failed */
  const unsigned char (&?buff)[],  /* 512 byte data block to be transmitted */
  unsigned char token      /* Data/Stop token */
)
{
  unsigned char d[2];

  unsafe{
  if (!wait_ready(drv)) return 0;

  d[0] = token;
  /* Xmit a token */
  sd_spi->transfer8(d[0]);

  /* Is it data token? */
  if (token != 0xFD)
  {
    /* Xmit the 512 byte data block to MMC/SD */
    for(unsigned j=0;j<512;j++){
         sd_spi->transfer8(buff[j]);}

    /* Xmit dummy CRC (0xFF,0xFF) */
    d[0] = sd_spi->transfer8(0xFF);
    d[0] = sd_spi->transfer8(0xFF);
    /* Receive data response */
    d[0] = sd_spi->transfer8(0xFF);

     if ((d[0] & 0x1F) != 0x05)  /* If not accepted, return with error */
      return 0;
  }
 }//unsafe
  return 1;
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

MDSTATUS sdspi_status (
  unsigned char drv      /* Drive number (always 0) */
)
{
  MDSTATUS s;
  unsigned char d[1];
unsafe{
  /* Check if the card is kept initialized */
  s = stat;
  if (!(s & ST_NOINIT))
  {
    if (send_cmd(drv, CMD13, 0))  /* Read card status */
      s = ST_NOINIT;
    /* Receive following half of R2 */
      d[0] = sd_spi->transfer8(0xFF);

    deselect(drv);
  }
  stat = s;
}
  return s;
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

MDSTATUS sdspi_initialize (
  unsigned char drv    /* Physical drive nmuber (0) */
)
{
  unsigned char n, ty, cmd, buf[4];
  unsigned int buffer, stat;
  unsigned int tmr;
  MDSTATUS s;

unsafe {


  ss_speed = LOW_SPEED_IN_KHZ;

  stat = ST_NOINIT;
  buf[0]=0xFF;
  for (n = 10; n; n--) /* 80 dummy clocks */
      sd_spi->transfer8(buf[0]);


  ty = 0;
  stat=0;
  if (send_cmd(drv, CMD0, 0) == 1) {      /* Enter Idle state */
    if (send_cmd(drv, CMD8, 0x1AA) == 1) {  /* SDv2? */
      /* Get trailing return value of R7 resp */

        buffer = sd_spi->transfer32(0xFF);

      if (buffer && 0x01AA) {    /* The card can work at vdd range of 2.7-3.6V */
        for (tmr = 1000; tmr; tmr--) {      /* Wait for leaving idle state (ACMD41 with HCS bit) */
          if (send_cmd(drv, ACMD41, 1UL << 30) == 0) break;
          DLY_US(1000);
        }
        if (tmr && send_cmd(drv, CMD58, 0) == 0) {  /* Check CCS bit in the OCR */
             buffer = sd_spi->transfer32(0xFF);
             ty = (buffer & 0x40000000) ? CT_SD2 | CT_BLOCK : CT_SD2;  /* SDv2 */
        }
      }
    } else {              /* SDv1 or MMCv3 */
      if (send_cmd(drv, ACMD41, 0) <= 1)   {
        ty = CT_SD1; cmd = ACMD41;  /* SDv1 */
      } else {
        ty = CT_MMC; cmd = CMD1;  /* MMCv3 */
      }
      for (tmr = 1000; tmr; tmr--) {      /* Wait for leaving idle state */
        if (send_cmd(drv, ACMD41, 0) == 0) break;
        DLY_US(1000);
      }
      if (!tmr || send_cmd(drv, CMD16, 512) != 0)  /* Set R/W block length to 512 */
        ty = 0;
    }
  }
  cardType = ty;

  s = ty ? 0 : ST_NOINIT;
  stat = s;

  deselect(drv);

  ss_speed = HIGH_SPEED_IN_KHZ;
}

  return s;
}


typedef unsigned char DATABLOCK[512];

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
#pragma unsafe arrays
MDRESULT sdspi_read (
  unsigned char drv,      /* Physical drive nmuber (0) */
  unsigned char buff[],      /* Pointer to the data buffer to store read data */
  unsigned long sector,    /* Start sector number (LBA) */
  unsigned int count      /* Sector count (1..128) */
)
{
  unsigned char BlockCount = 0;
unsafe{

  if (sdspi_status(drv) & ST_NOINIT) return MD_NOTRDY;
  if (!count) return MD_PARERR;
  if (!(cardType & CT_BLOCK)) sector *= 512;  /* Convert LBA to byte address if needed */

  if (count == 1) {  /* Single block read */
    if ((send_cmd(drv, CMD17, sector) == 0)  /* READ_SINGLE_BLOCK */
      && rcvr_datablock(drv, buff, 512))
      count = 0;
  }
  else {        /* Multiple block read */
    if (send_cmd(drv, CMD18, sector) == 0) {  /* READ_MULTIPLE_BLOCK */
      do {
        if (!rcvr_datablock(drv, (buff, DATABLOCK[])[BlockCount++], 512)) break;
      } while (--count);
      send_cmd(drv, CMD12, 0);        /* STOP_TRANSMISSION */
    }
  }
  deselect(drv);
}//unsafe
  return count ? MD_ERROR : MD_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#pragma unsafe arrays
MDRESULT sdspi_write (
  unsigned char drv,      /* Physical drive nmuber (0) */
  const unsigned char buff[],  /* Pointer to the data to be written */
  unsigned long sector,    /* Start sector number (LBA) */
  unsigned int count      /* Sector count (1..128) */
)
{
  unsigned char BlockCount = 0;
unsafe{
  if (sdspi_status(drv) & ST_NOINIT) return MD_NOTRDY;
  if (!count) return MD_PARERR;
  if (!(cardType & CT_BLOCK)) sector *= 512;  /* Convert LBA to byte address if needed */

  if (count == 1) {  /* Single block write */

    if ((send_cmd(drv, CMD24, sector) == 0)  /* WRITE_BLOCK */
      && xmit_datablock(drv, buff, 0xFE))
      count = 0;
  }
  else {        /* Multiple block write */

    if (cardType & CT_SDC) send_cmd(drv, ACMD23, count);
    if (send_cmd(drv, CMD25, sector) == 0) {  /* WRITE_MULTIPLE_BLOCK */
      do {
        if (!xmit_datablock(drv, (buff, DATABLOCK[])[BlockCount++], 0xFC)) break;
      } while (--count);
      if (!xmit_datablock(drv, null, 0xFD))  /* STOP_TRAN token */
        count = 1;
    }
  }
  deselect(drv);
}
  return count ? MD_ERROR : MD_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
#pragma unsafe arrays
MDRESULT sdspi_ioctl (
  unsigned char drv,    /* Physical drive nmuber (0) */
  unsigned char ctrl,    /* Control code */
  unsigned char buff[]    /* Buffer to send/receive control data */
)
{
  MDRESULT res;
  unsigned char n, csd[16];
  unsigned short cs;

  unsafe{
  if (sdspi_status(drv) & ST_NOINIT) return MD_NOTRDY;  /* Check if card is in the socket */

  res = MD_ERROR;
  switch (ctrl) {
    case CL_SYNC:    /* Make sure that no pending write process */
      if (Select(drv)) {
        deselect(drv);
        res = MD_OK;
      }
      break;

    case SECTOR_COUNT :  /* Get number of sectors on the disk (DWORD) */
      if ((send_cmd(drv, CMD9, 0) == 0) && rcvr_datablock(drv, csd, 16)) {
        if ((csd[0] >> 6) == 1) {  /* SDC ver 2.00 */
          cs= csd[9] + ((unsigned short)csd[8] << 8) + 1;
          //*(DWORD*)buff = (DWORD)cs << 10;
          for(unsigned long Val = cs << 10, i = 0; i < sizeof(unsigned long); i++)
            buff[i] = (Val, unsigned char[])[i];
        } else {          /* SDC ver 1.XX or MMC */
          n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
          cs = (csd[8] >> 6) + ((unsigned short)csd[7] << 2) + ((unsigned short)(csd[6] & 3) << 10) + 1;
          //*(DWORD*)buff = (DWORD)cs << (n - 9);
          for(unsigned long Val = (unsigned long)cs << (n - 9), i = 0; i < sizeof(unsigned long); i++)
            buff[i] = (Val, unsigned char[])[i];
        }
        res = MD_OK;
      }
      break;

    case BLOCK_SIZE :  /* Get erase block size in unit of sector (DWORD) */
      //*(DWORD*)buff = 128;
      for(unsigned long Val = 128, i = 0; i < sizeof(unsigned long); i++)
        buff[i] = (Val, unsigned char[])[i];
      res = MD_OK;
      break;

    case TOT_SIZE:
        if ((send_cmd(drv, CMD9, 0) == 0) && rcvr_datablock(drv, csd, 16)) {
               if ((csd[0] >> 6) == 1) {  /* SDC ver 2.00 */
                 cs= csd[9] + ((unsigned short)csd[8] << 8) + 1;
                 buff[0] = ((cs + 1)*1024)-1;

               } else {          /* SDC ver 1.XX or MMC */
                 n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                 cs = (csd[8] >> 6) + ((unsigned short)csd[7] << 2) + ((unsigned short)(csd[6] & 3) << 10) + 1;
                 buff[0] = ((cs + 1)*n)-1;
               }
               res = MD_OK;
             }
        break;

    default:
      res = MD_PARERR;
      break;
  }

  deselect(drv);
  }//unsafe
  return res;
}

unsigned long getcapacity (unsigned char drv)
{
    unsigned char n, csd[16];
    unsigned long cs, sdcap;

    unsafe{

      /* Check if card is in the socket */
    if (sdspi_status(drv) & ST_NOINIT) return 0;

    if ((send_cmd(drv, CMD9, 0) == 0) && rcvr_datablock(drv, csd, 16))
    {
       if ((csd[0] >> 6) == 1)
       {  /* SDC ver 2.00 */
        // sdcap = ((csd[9] + ((WORD)csd[8] << 8) + 1 )*1024)-1;
           cs = ((((unsigned long)csd[7] & 0x3F)<< 16) | ((unsigned short)csd[8] << 8) | csd[9]);
           sdcap = (((unsigned long)cs + 1 )* (unsigned short)(1024u))-1;
       }
       else
       {          /* SDC ver 1.XX or MMC */
         n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
         cs = (csd[8] >> 6) + ((unsigned short)csd[7] << 2) + ((unsigned short)(csd[6] & 3) << 10) + 1;
         sdcap = ((cs + 1)*n)-1;
       }

     }
    }//unsafe
    return sdcap;
}

//////////////////////////////////////////////////////////////////////////////////

[[distributable]]
void sd_host_spi(server interface sd_host_if i[num_clients],
        static const size_t num_clients,
        client spi_master_if i_spi,
        in port  p_sdcarddetect)
{

    unsigned int CardDetect;


    p_sdcarddetect:> CardDetect;
    if ((CardDetect & 0x01) != 0) {
            printstrln("Card Not Detected");
            stat= ST_NODISK;
     }

    /* SD Wrapper to provide SPI Master Client Interface   */
    unsafe {
    sd_spi = (client spi_master_if * unsafe) &i_spi;
    }



    while(1){
               select {
               case i[int x].sd_initialize(void)->unsigned char status:{

                   status = sdspi_initialize(0);
               }
                break;
               case i[int x].sd_status(unsigned char drv)->unsigned char status:{

                   p_sdcarddetect:> CardDetect;
                   if ((CardDetect & 0x1) != 0)
                    {
                       stat |= (ST_NODISK | ST_NOINIT);
                    }
                     else
                    {
                        stat &= ~ST_NODISK;
                     }
                    status = stat;
                  }

                break;
               case i[int x].sd_read(unsigned char *buff, unsigned long sector, unsigned int count)->unsigned int result:{
                         result = sdspi_read(0,buff,sector,count);
               }
               break;
               case i[int x].sd_write(const unsigned char *buff, unsigned long sector, unsigned int count)->unsigned int result:{
                         result = sdspi_write(0,buff,sector,count);
                }
               break;
               case i[int x].sd_ioctl(unsigned char *buff,unsigned char ctrl)->unsigned int result:{

                   result = sdspi_ioctl(0,ctrl,buff);


               }
               break;

              } //select

         }//while loop

}
