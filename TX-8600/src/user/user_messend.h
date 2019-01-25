#ifndef __USER_MESSEND_H
#define __USER_MESSEND_H

#include <stdint.h>

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

void mes_send_begin();

void mes_send_decode();

void mes_send_listinfo(uint8_t type,uint8_t need_send);

void mes_send_acinfo(uint16_t id);

void mes_send_overtime();

#if defined(__cplusplus) || defined(__XC__)
}
#endif


#endif

