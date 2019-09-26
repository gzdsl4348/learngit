// Copyright (c) 2015, XMOS Ltd, All rights reserved

/*
 * SDCard Host Application Programming Interface(SDCARD Library)
 * This set of APIs are provided to interact with SD Card in 4 native or SPI mode.
 * */

#ifndef SDCARD_HOST_H_
#define SDCARD_HOST_H_

#include <xs1.h>
#include <xclib.h>
#include <platform.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __XC__

/** This interface allows clients to interact with SD HOST task.*/

typedef interface sd_host_if {

/**  Initialize the SD interface.
   *
   *  This function will initialize the SD Interface, communicates with the SD Card
   *  and make it ready for read/write transcation. The function returns the status
   *  of the SD Card.
   *  
   *   \returns  unsigned char    Media Status.
   */
unsigned char sd_initialize (void);

/**   Current Status of SD Card.
   *
   *  This function will return the current status of SD Card
   *  The possible return values are as:
   *  1. 0x00 - Media is initialized.
   *  2. 0x01 - Media is not initialized.
   *  3. 0x02 - No medium detected.
   *  4. 0x04 - Write-Protected.
   *
   *  \param      device_id          Drive / Device ID
   *
   *  \returns                      Media Status.
   */

unsigned char sd_status (unsigned char device_id);


/**  Read Data from SD Card.
   *
   *  This function will perform a single or multiple read from SD Card.
   *  Read is sector based (512 bytes of data). Sector is the starting
   *  sector address to read from and Count is the number of sectos to read.
   *
   *  It should be noted that due to the passing of data pointer into
   *  the function, the application and host need to reside on the same tile.
   *
   *  \param buff     Data Buffer to store the read data
   *  \param sector   Sector Number
   *  \param count    Data Count
   *
   *  \returns        Media Transcation Result.
   */

unsigned int sd_read ( unsigned char *buff, unsigned long sector , unsigned int count);
/**  Write Data to SD Card.
   *
   *  This function will perform a single or multiple write to SD Card.
   *  Write is sector based (512 bytes of data). Sector is the starting
   *  sector address to write from and Count is the number of sectos to write.
   *
   *  It should be noted that due to the passing of data pointer into
   *  the function, the application and host need to reside on the same tile.
   *
   *
   *  \param buff      Data Buffer to be written.
   *  \param sector    Sector Number
   *  \param count     Data Count
   *
   *   \returns        Media Transcation Result.
   */
unsigned int sd_write (const unsigned char *buff, unsigned long sector, unsigned int count);

/**  Miscellaneous Functions
   *
   *  This functions provide the miscellaneous options to perform few
   *  specific operations (IOCTL calls) with the SD Card.The command/
   *  functions supported are:
   *
   *   1. Control Code 0 - Flush disk cache (for write functions)
   *   2. Control Code 1 - Get Sector Count
   *   3. Control Code 3 - Get Block Size
   *   4. Control Code 10 - Get Card Type
   *   5. Control Code 11 - Get CSD
   *   6. Control Code 12 - Get CID
   *   7. Control Code 13 - Get OCR
   *   8. Control Code 14 - Get Card Status (CMD13)
   *
   *  \param buff      Data Buffer with return data.
   *  \param command   IOCTL command
   *
   *  \returns         Media Transaction Result.
   */
unsigned int sd_ioctl (unsigned char *buff,unsigned char command);



} sd_host_if;


/** Task that implements the SDCARD Host with Native proctocol.

    \param i             An array of interface connection to the
                         clients of the task.
    \param num_clients   The number of clients connected to the task.
    \param p_sdclk           The SD Clock port.
    \param p_sdcmd           The SD Command port.
    \param p_sddata          The SD Data port [DAT3..0].
    \param p_sdcarddetect    The SD Card Detection port.
    \param sdClkblk          A clock for the component to use.
    \param cb_sclk           A clock for the component to use.
*/

[[distributable]]
void sd_host_native(server interface sd_host_if i[num_clients],
                    static const size_t  num_clients,
                    buffered out port:32 p_sdclk,
  [[bidirectional]] buffered port:8 p_sdcmd,
  [[bidirectional]] buffered port:32 p_sddata,
                    clock  sdClkblk,
                    clock  cb_sclk
                    );
#endif //_XC_

/** Wrapper to SDCARD library when using the FATFS filesystem.

    \param sdhi          A client interface to SDCARD HOST.
*/
[[distributable]]
void sd_fs_wrapper(client interface sd_host_if sdhi);

#endif
