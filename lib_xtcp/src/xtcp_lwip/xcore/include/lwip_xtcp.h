// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved

#ifndef _LWIP_XTCP_H_
#define _LWIP_XTCP_H_

#include <xccompat.h>

void lwip_xtcp_checkstate();
void lwip_xtcp_up();
void lwip_xtcp_down();
void lwip_xtcp_checklink(chanend connect_status);
int get_lwip_xtcp_ifstate();
void lwip_linkdown();
void lwip_linkup();
void lwip_xtcp_null_events();

#endif // _LWIP_XTCP_H_
