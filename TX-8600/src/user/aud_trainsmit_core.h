#ifndef __AUD_TRAINSMIT_CORE_H
#define __AUD_TRAINSMIT_CORE_H
#include "ethernet.h"
#include "stdint.h"
#include "user_unti.h"

typedef interface aud_trainsmit_if{
    uint8_t aud_ch_chk();
    void divlist_set(audts_divlist_s audts_divlist,xtcp_ipconfig_t set_host_ip,uint8_t set_host_mac[6]);
}aud_trainsmit_if;

void aud_trainsmit_core(streaming chanend c_rx_hp,server aud_trainsmit_if if_aud_trainsmit,client ethernet_tx_if i_eth_tx);

#endif

