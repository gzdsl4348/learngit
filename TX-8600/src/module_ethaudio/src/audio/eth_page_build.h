#ifndef  __ETH_PAGE_BUILD_H
#define  __ETH_PAGE_BUILD_H

#include <xs1.h>
#include <platform.h>
#include <stdint.h>

#include "eth_audio_config.h"
#include "tcpip_base_adr.h"
#include "audio_tx.h"

void aud_udpdata_init(uint8_t txbuff[],uint8_t mac_address[]);

uint16_t audio_page_build(uint8_t txbuff[],
                            uint8_t ipaddress[],
                            uint8_t &audio_format,
                            uint8_t audio_type,
                            uint8_t &volume,
                            unsigned timestamp,
                            uint16_t &iptmp,
                            uint16_t &udptmp,
                            uint32_t mp3_frame_size,      
                            uint8_t ch);

void audpage_sum_build(uint8_t txbuff[],uint8_t des_ip[],uint8_t des_mac[],uint8_t area_contorl);


#endif // __ETH_PAGE_BUILD_H


