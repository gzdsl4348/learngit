// Copyright (c) 2016, XMOS Ltd, All rights reserved
#ifndef __xtcp_shared_includes_h__
#define __xtcp_shared_includes_h__

#include "xtcp.h"

typedef struct client_queue_t {
  xtcp_event_type_t xtcp_event;
  /* Pointer to connection in uIP or LWIP */
  xtcp_connection_t *unsafe xtcp_conn;

#if (XTCP_STACK == LWIP)
  struct pbuf *unsafe pbuf;
#endif
} client_queue_t;

unsigned get_if_state(void);
unsafe void renotify(unsigned client_num);
unsafe void xtcp_init_queue(static const unsigned n_xtcp, server xtcp_if i_xtcp_init[n_xtcp]);
unsafe xtcp_connection_t create_xtcp_state(int xtcp_num, xtcp_protocol_t protocol,
                                           unsigned char * unsafe remote_addr,
                                           int local_port, int remote_port,
                                           void * unsafe uip_lwip_conn);

unsafe client_queue_t dequeue_event(unsigned client_num);
unsafe void enqueue_event_and_notify(unsigned client_num, 
                                     xtcp_event_type_t xtcp_event,
                                     xtcp_connection_t * unsafe xtcp_conn
#if (XTCP_STACK == LWIP)
                                     ,struct pbuf *unsafe pbuf
#endif
                                     );

unsafe void rm_recv_events(unsigned conn_id, unsigned client_num);

unsafe void xtcp_if_up(void);
unsafe void xtcp_if_down(void);

#endif /* __xtcp_shared_includes_h__ */