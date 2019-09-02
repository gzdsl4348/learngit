#ifndef __USER_MESSEND_H
#define __USER_MESSEND_H

#include <stdint.h>
#include "sys_config_dat.h"
#include "user_unti.h"
#include "ack_build.h"
#include "user_xccode.h"
#include "xtcp.h"
#include "debug_print.h"

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

// 连接建立
uint8_t mes_list_add(xtcp_connection_t conn,uint8_t could_f,uint8_t could_id[],uint8_t account_f);

// 连接关闭
void mes_list_close(unsigned id);

// 通知连发
void mes_send_decode();

// 各列表信息更新通知
void mes_send_listinfo(uint8_t type,uint8_t need_send);

// 账号更新通知
void mes_send_acinfo(uint16_t id);

// 任务更新通知 附带页面更新
void mes_send_taskinfo(task_allinfo_tmp_t* task_all_info);

// 任务更新通知 不带页面更新
void mes_send_taskinfo_nopage(task_allinfo_tmp_t* task_all_info);

// 即时任务更新通知
void mes_send_rttaskinfo(uint16_t id,uint8_t contorl,uint8_t page_state);

// 方案更新通知
void mes_send_suloinfo(uint16_t id);

// 发送超时
void mes_send_overtime();

// 任务页面更新通知
void taskview_page_messend();

#if defined(__cplusplus) || defined(__XC__)
}
#endif


#endif

