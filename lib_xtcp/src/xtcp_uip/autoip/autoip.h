// Copyright (c) 2011-2016, XMOS Ltd, All rights reserved

#ifndef _autoip_h_
#define _autoip_h_

void uip_autoip_init(int seed);
void uip_autoip_arp_in();
void uip_autoip_start();
void uip_autoip_stop();
void uip_autoip_configured(uip_ipaddr_t ipaddr);
void uip_autoip_periodic();

void uip_autoip_en_checkip(u16_t ipaddr[]);
u8_t uip_autoip_read_conflict_flag(void);


#endif //_autoip_h_
