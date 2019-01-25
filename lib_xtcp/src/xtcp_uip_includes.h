// Copyright (c) 2016, XMOS Ltd, All rights reserved
#ifndef __xtcp_uip_includes_h__
#define __xtcp_uip_includes_h__

/* Work around xC keywords used as variable names in uIP */
#define in _in
#define module _module
#define forward _forward
#define interface _interface
#define port _port
#define timer _timer
#ifdef __XC__
extern "C" {
#endif

/* The include files */
#include "uip.h"
#include "autoip.h"
#include "igmp.h"
#include "dhcpc.h"
#include "uip_arp.h"
#include "uip-split.h"

#ifdef __XC__
}
#endif
#undef in
#undef module
#undef forward
#undef interface
#undef port
#undef timer

#endif /* __xtcp_uip_includes_h__ */