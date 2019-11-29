#ifndef __USER_WIFICONTORL_H_
#define __USER_WIFICONTORL_H_

#include <stdint.h>
#include "sys_config_dat.h"
#include "user_unti.h"
#include "user_lcd.h"

void wifi_contorl_mode();

void wifi_open();

void user_wifi_ipset(uint8_t ip[]);

#endif

