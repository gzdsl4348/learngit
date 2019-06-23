// Copyright (c) 2016, XMOS Ltd, All rights reserved
#include "xc2compat.h"
#include <string.h>
#include <xassert.h>
#include <print.h>
#include <debug_print.h>
#include "xtcp.h"
#include "xtcp_shared.h"
#if (XTCP_STACK == LWIP)
#include "xtcp_lwip_includes.h"
#endif

/* A 2D array of queue items */
static client_queue_t client_queue[MAX_XTCP_CLIENTS][CLIENT_QUEUE_SIZE] = {{{0}}};
static unsigned client_heads[MAX_XTCP_CLIENTS] = {0};
static unsigned client_num_events[MAX_XTCP_CLIENTS] = {0};

static server xtcp_if * unsafe i_xtcp; /* Used for notifying */
static unsigned ifstate = 0;           /* Connection state */
static unsigned n_xtcp;                /* Number of clients */

unsafe void xtcp_init_queue(static const unsigned n_xtcp_init, 
                            server xtcp_if i_xtcp_init[n_xtcp_init])
{
  xassert(n_xtcp <= MAX_XTCP_CLIENTS);
  i_xtcp = i_xtcp_init;
  memset(client_queue, 0, sizeof(client_queue));
  memset(client_heads, 0, sizeof(client_heads));
  memset(client_num_events, 0, sizeof(client_num_events));
  n_xtcp = n_xtcp_init;
}

unsafe void renotify(unsigned client_num)
{
  if(client_num_events[client_num] > 0) {
    i_xtcp[client_num].packet_ready();
  }
}
extern int uip_udp_connid_is_free(int connid);
static unsigned get_guid(void)
{
  static unsigned guid = 0;
  
  while(1) {
    if(guid++ > (0xfffffff)) {
      guid = 1;
    }
    debug_printf("\nxtcp build id %x\n\n",guid);
    //if(uip_udp_connid_is_free(guid) == 0) continue;
    break;
  }

  return guid;
}

unsafe xtcp_connection_t create_xtcp_state(int xtcp_num,
                                           xtcp_protocol_t protocol,
                                           unsigned char * unsafe remote_addr,
                                           int local_port,
                                           int remote_port,
                                           void * unsafe uip_lwip_conn)
{
  xtcp_connection_t xtcp_conn = {0};

  xtcp_conn.client_num = xtcp_num;
  xtcp_conn.id = get_guid();
  xtcp_conn.protocol = protocol;
  for (int i=0; i<4; i++)
    xtcp_conn.remote_addr[i] = remote_addr[i];
  xtcp_conn.remote_port = remote_port;
  xtcp_conn.local_port = local_port;
  if(protocol == XTCP_PROTOCOL_UDP)
    xtcp_conn.mss = MAX_PACKET_BYTES;

  xtcp_conn.stack_conn = (int) uip_lwip_conn;
  return xtcp_conn;
}

unsafe client_queue_t dequeue_event(unsigned client_num)
{
  client_num_events[client_num]--;
  xassert(client_num_events[client_num] >= 0);

  unsigned position = client_heads[client_num];
  client_heads[client_num] = (client_heads[client_num] + 1) % CLIENT_QUEUE_SIZE;
  return client_queue[client_num][position];
}

unsafe void enqueue_event_and_notify(unsigned client_num, 
                                     xtcp_event_type_t xtcp_event,
                                     xtcp_connection_t * unsafe xtcp_conn
#if (XTCP_STACK == LWIP)
                                     ,struct pbuf *unsafe pbuf
#endif
                                     )
{
  if(client_num!=0)
    return;
  unsigned position = (client_heads[client_num] + client_num_events[client_num]) % CLIENT_QUEUE_SIZE;
  client_queue[client_num][position].xtcp_event = xtcp_event;
  client_queue[client_num][position].xtcp_conn = xtcp_conn;
#if (XTCP_STACK == LWIP)
  client_queue[client_num][position].pbuf = pbuf;
#endif

  client_num_events[client_num]++;
  xassert(client_num_events[client_num] <= CLIENT_QUEUE_SIZE);

  /* Notify */
  i_xtcp[client_num].packet_ready();
}

unsafe void rm_recv_events(unsigned conn_id, unsigned client_num)
{
  for(unsigned i=0; i<client_num_events[client_num]; ++i) {
    unsigned place_in_queue = (client_heads[client_num] + i) % CLIENT_QUEUE_SIZE;
    client_queue_t current_queue_item = client_queue[client_num][place_in_queue];
    
    /* Found item */
    if(current_queue_item.xtcp_event == XTCP_RECV_DATA &&
       current_queue_item.xtcp_conn->id == conn_id) {

      client_num_events[client_num]--;

#if (XTCP_STACK == LWIP)
      if (current_queue_item.pbuf) {
        pbuf_free(current_queue_item.pbuf);
      }
#endif
      
      /* Move rest of events up queue */
      for(unsigned j=i; j<client_num_events[client_num]; ++j) {
        unsigned place = (client_heads[client_num] + j) % CLIENT_QUEUE_SIZE;
        unsigned next_place = ++place % CLIENT_QUEUE_SIZE;
        client_queue[client_num][place] = client_queue[client_num][next_place];
      }

      /* uIP can only have one packet in the buffer,
       * whereas LWIP can have many */
#if (XTCP_STACK == UIP)
      break;
#endif
    }
  }
}

unsigned get_if_state(void) 
{ 
  return ifstate;
}

xtcp_connection_t if_up_dummy = {{0}};
xtcp_connection_t if_down_dummy = {{0}};

unsafe void xtcp_if_up(void)
{
  ifstate = 1;
  // memset(&if_up_dummy, 0, sizeof(if_up_dummy));
  for(unsigned i=0; i<n_xtcp; ++i) {
#if (XTCP_STACK == LWIP)
    enqueue_event_and_notify(i, XTCP_IFUP, &if_up_dummy, NULL);
#else /* uIP */
    enqueue_event_and_notify(i, XTCP_IFUP, &if_up_dummy);
#endif
  }
}
extern void n_clear_autoip_lock(void);
extern void n_uip_clear_autoip_flag(void);
unsafe void xtcp_if_down(void)
{
  n_clear_autoip_lock();
  n_uip_clear_autoip_flag();
  ifstate = 0;
  for(unsigned i=0; i<n_xtcp; ++i) {
#if (XTCP_STACK == LWIP)
    enqueue_event_and_notify(i, XTCP_IFDOWN, &if_down_dummy, NULL);
#else /* uIP */
    enqueue_event_and_notify(i, XTCP_IFDOWN, &if_down_dummy);
#endif
  }
}