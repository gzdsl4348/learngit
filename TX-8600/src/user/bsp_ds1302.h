#ifndef  __BSP_DS1302_H
#define  __BSP_DS1302_H

#include "stdint.h"

#define ds1302_second_write  0x80
#define ds1302_second_read   0x81

#define ds1302_minute_write  0x82
#define ds1302_minute_read   0x83

#define ds1302_hour_write    0x84
#define ds1302_hour_read     0x85

#define ds1302_date_write    0x86
#define ds1302_date_read     0x87

#define ds1302_month_write   0x88
#define ds1302_month_read    0x89

#define ds1302_week_write    0x8A
#define ds1302_week_read     0x8B

#define ds1302_year_write    0x8C
#define ds1302_year_read     0x8D



void ds1302_init();

uint8_t ds1302_contorl(uint8_t cmd,uint8_t data);

void ds1302_get_date();

void ds1302_time_set();

void ds1302_date_set();

#endif  //__BSP_DS1302_H

