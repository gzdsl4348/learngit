#ifndef __USER_LCD_H_
#define __USER_LCD_H_

#include "stdint.h"
#include "xtcp.h"

void user_disp_time();

void user_disp_data();

void user_disp_ip(xtcp_ipconfig_t ipconfig);

// 显示任务
void user_disp_task(uint8_t state);
// 刷新任务
void user_disptask_refresh();
// 定时刷任务
void timer_task_disp();
// 显示版本
void user_disp_version();

void user_dispunti_init();

// 延时显示
void disp_task_delay();

#endif

