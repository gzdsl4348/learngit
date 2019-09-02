#ifndef  __FL_BUFF_DECODE_H_
#define  __FL_BUFF_DECODE_H_

#include <stdint.h>
#include "flash_adrbase.h"
#include "file_list.h"

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

//--------------------------------------------------
//系统信息 读写
void sys_dat_read(uint8_t buff[],uint16_t num,uint16_t base_adr);

void sys_dat_write(uint8_t buff[],uint16_t num,uint16_t base_adr);

//------------------------------------------------------------------------
// 初始化用户数据
void fl_hostinfo_init();

// 读取flash初始化状态，判断FLASH是否需要初始化
uint8_t read_hostinfo_reset_state();
//----------------------------------------------------------
// 读写主机数据
void fl_hostinfo_write();

void fl_hostinfo_read();
//------------------------------------------------------
// 账户列表信息读写
void fl_account_read(account_all_info_t *account_all_info,uint8_t id);

void fl_account_write(account_all_info_t *account_all_info,uint8_t id);
//------------------------------------------------------
// 分区信息读写
void fl_area_read();
//
void fl_area_write();
//------------------------------------------------------
// 设备列表读写
void fl_divlist_read();
//
void fl_divlist_write();
//-------------------------------------------------------
// 方案列表烧写
void fl_solution_write();

void fl_solution_read();
//-------------------------------------------------------
// 定时任务读写
void fl_timertask_read(task_allinfo_tmp_t     *task_allinfo_tmp,uint16_t id);
//
void fl_timertask_write(task_allinfo_tmp_t *task_allinfo_tmp, uint16_t id);
//-------------------------------------------------------
// 即时任务读写
void fl_rttask_read(rttask_dtinfo_t     *rttask_dtinfo,uint16_t id);
//
void fl_rttask_write(rttask_dtinfo_t *rttask_dtinfo,uint16_t id);

#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__FL_BUFF_DECODE_H_

