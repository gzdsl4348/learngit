// Copyright (c) 2011-2016, XMOS Ltd, All rights reserved

#include "mdns.h"

mdns_event mdns_handle_event(chanend tcp_svr,
                       xtcp_connection_t &conn,
                       unsigned int t);

mdns_event mdns_xtcp_handler(chanend tcp_svr,
                       xtcp_connection_t &conn)
{
  timer tmr;
  unsigned t;
  tmr :> t;
  return mdns_handle_event(tcp_svr, conn, t);
}


