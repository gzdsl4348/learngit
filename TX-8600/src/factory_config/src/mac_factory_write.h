#ifndef __MAC_FACTOR_WRITE_H__
#define __MAC_FACTOR_WRITE_H__

#include <stdint.h>
#include "xtcp.h"

#ifndef null
#define null -1
#endif

typedef struct mac_facinfo_t{
	uint8_t mac[6];
}mac_facinfo_t;


// MAC地址 FLASH烧录程序，需定义
void mac_writeflash(uint8_t macadr[6]);


#if 0
// 出厂MAC烧录 主程序 
// t_mac_facinfo 中的MAC地址为 42 4C 45 00 00 00时 进入出厂烧录MAC模式。
// 出厂MAC烧录模式 IP地址自动设为192.168.168.168
void mac_factory_write(client xtcp_if i_xtcp, 
                          mac_facinfo_t t_mac_facinfo,
					      char rx_buffer[],
					      char tx_buffer[]);
#endif


int mac_factory_init(client xtcp_if i_xtcp);

void mac_fatctory_xtcp_event(client xtcp_if i_xtcp, xtcp_connection_t &conn, unsigned char rx_buffer[], unsigned int data_len);


#endif	//__MAC_FACTOR_WRITE_H__


