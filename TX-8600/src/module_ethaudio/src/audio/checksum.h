#ifndef  __CHECKSUM_H
#define  __CHECKSUM_H

#include <stdint.h>
#include "xclib.h"

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

uint16_t chksum_16bit(uint16_t sum, uint8_t *byte_data, uint16_t lengthInBytes);
uint16_t chksum_8bit(uint16_t sum, uint8_t *byte_data, uint16_t lengthInBytes);


#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__CHECKWUM_H

