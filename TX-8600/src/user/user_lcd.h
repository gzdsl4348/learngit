/*
*  File Name:remotel_business_base.h
*  Created on: 2019年5月26日
*  Author: caiws
*  description :lcd显示函数
*   Modify date: 
* 	Modifier Author:
*  description :
*/

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
//显示初始化
void user_dispunti_init();
// 延时显示
void disp_task_delay();
// 显示云链接状态
void disp_couldstate(uint8_t state);
// DHCP图标开启显示
void dhcp_disp_en();
// DHCP图标关闭显示
void dhcp_disp_dis();
void dhcp_disp_none();

void ip_disp_decode(uint8_t data,uint8_t *base_adr);

void ip_conflict_disp(uint8_t state);

void dhcp_getin_disp();

void dhcp_getin_over_disp(uint8_t state);

void dhcp_getin_clear();

void wifi_open_disp();

void reset_data_disp(uint8_t second);

#endif

