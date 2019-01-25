// Copyright (c) 2011-2017, XMOS Ltd, All rights reserved

#include <print.h>
#include <xs1.h>
#include <ethernet.h>
#include <mii.h>
#include <string.h>

extern unsigned short uip_len;
extern unsigned int uip_buf32[];
extern unsigned char * unsafe uip_buf;
extern void * unsafe uip_sappdata;

client interface ethernet_tx_if  * unsafe xtcp_i_eth_tx = NULL;
client interface mii_if * unsafe xtcp_i_mii = NULL;
mii_info_t xtcp_mii_info;

#ifndef UIP_MAX_TRANSMIT_SIZE
#define UIP_MAX_TRANSMIT_SIZE 1520 /* bytes */
#endif

unsafe static void 
mii_send(void)
{
  static int txbuf[(UIP_MAX_TRANSMIT_SIZE+3)/4];
  static int first_packet_sent = 0;
  int len = uip_len;
  
  if (first_packet_sent) {
    select {
    case mii_packet_sent(xtcp_mii_info):
      break;
    }
  }
  
  if (len < 60) {
    for (int i=len; i < 60; i++) {
      (uip_buf32, unsigned char[])[i] = 0;
    }
    len=60;
  }

  memcpy(txbuf, uip_buf32, len);
  xtcp_i_mii->send_packet(txbuf, len);
  first_packet_sent=1;
}

void
xcoredev_send(void)
{
  unsafe {
    int len = uip_len;
    if (len != 0) {
      if (xtcp_i_eth_tx != NULL) {
        if (len < 60) {
          for (int i=len; i<60; i++) {
            (uip_buf32, unsigned char[])[i] = 0;
          }
          len=60;
        }
        xtcp_i_eth_tx->send_packet((char *) uip_buf32, len, ETHERNET_ALL_INTERFACES);
      } else {
        mii_send();
      }
    }
  }
}