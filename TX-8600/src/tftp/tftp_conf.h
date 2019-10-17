// Copyright (c) 2011-2016, XMOS Ltd, All rights reserved
#ifndef TFTP_CONF_H_
#define TFTP_CONF_H_

#ifndef TFTP_DEBUG_PRINT
#define TFTP_DEBUG_PRINT      1
#endif

#ifndef TFTP_DEFAULT_PORT
#define TFTP_DEFAULT_PORT     69
#endif

#ifndef TFTP_BLOCK_SIZE
#define TFTP_BLOCK_SIZE       1428       /* 512 bytes */
#endif
#ifndef TFTP_MAX_FILE_SIZE
#define TFTP_MAX_FILE_SIZE      (512 * 1024 * 1024)  /* 512MB */
#endif

// The number of seconds after which the connection will close if no new data is received
#ifndef TFTP_TIMEOUT_SECONDS
#define TFTP_TIMEOUT_SECONDS    3
#endif

// The maximum length in bytes of the custom messages in error packets.
// Should always be >= the length of the longest error message.
#ifndef TFTP_ERROR_MSG_MAX_LENGTH
#define TFTP_ERROR_MSG_MAX_LENGTH 25
#endif

// By default, the TFTP client only accepts an image with the filename TFTP_IMAGE_FILENAME.
// Setting TFTP_ACCEPT_ANY_FILENAME to non-zero allows the client to accept any filename
#ifndef TFTP_ACCEPT_ANY_FILENAME
#define TFTP_ACCEPT_ANY_FILENAME  1
#endif

#if !TFTP_ACCEPT_ANY_FILENAME
#ifndef TFTP_IMAGE_FILENAME
#define TFTP_IMAGE_FILENAME   "upgrade.bin" /* Don't forget quotes around the string */
#endif
#endif

#define TFTP_READ_REQUEST_ENABLE  1

typedef unsigned short n16_t;

#define ntoh16(n) ((((n)&0xff)<<8)| (((n)&0xff00)>>8))

#define hton16(n) ntoh16(n)

#endif /* TFTP_CONF_H_ */
