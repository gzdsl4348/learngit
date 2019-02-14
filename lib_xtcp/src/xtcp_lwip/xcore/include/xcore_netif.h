// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#ifndef __XCORE_NETIF_H__
#define __XCORE_NETIF_H__
#include <xccompat.h>
#include <xc2compat.h>

unsafe err_t xcore_igmp_mac_filter(struct netif *unsafe netif,
                                   const ip4_addr_t *unsafe group,
                                   u8_t action);


/** Function prototype for netif->linkoutput functions. Only used for ethernet
 * netifs. This function is called by ARP when a packet shall be sent.
 *
 * @param netif The netif which shall send a packet
 * @param p The packet to send (raw ethernet packet)
 */
unsafe err_t xcore_linkoutput(struct netif *unsafe netif, struct pbuf *unsafe p);

#endif