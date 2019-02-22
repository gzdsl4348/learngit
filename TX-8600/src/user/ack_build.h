#ifndef __ACK_BUILD_H_
#define __ACK_BUILD_H_

#include <stdint.h>
#include "list_instance.h"

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

void could_list_init();

uint16_t build_endpage_decode(uint16_t len,uint16_t cmd,uint8_t id[]);

// 上线应答发送
uint16_t online_request_ack_build(uint8_t online_state,uint8_t mode);

// 应答发送
uint16_t onebyte_ack_build(uint8_t mode,uint16_t cmd);

uint16_t twobyte_ack_build(uint8_t state1,uint8_t state2,uint16_t cmd);

uint16_t threebyte_ack_build(uint8_t state1,uint8_t state2,uint8_t state3,uint16_t cmd);

uint16_t id_ack_build(uint16_t id,uint8_t state,uint16_t cmd);

// 心跳包应答
uint16_t heart_ack_build(uint8_t state);

// 设备详细信息发送
uint16_t extra_info_build_ack();

// 设备列表发送
uint16_t div_list_resend_build(uint16_t cmd,div_node_t **div_list_p,uint8_t div_send_num);
// ip mac 列表发送
uint16_t div_ipmac_list_send();

// 分区列表组包
uint16_t area_list_send_build(uint16_t cmd);
// 分区配置回复
uint16_t area_config_ack_build(uint16_t area_sn,uint8_t state,uint8_t contorl);
// 用户登录信息回复
uint16_t account_login_ack_build(uint8_t log_state,uint8_t user_id,uint8_t *mac_buf,uint16_t cmd);

// 账户列表查看
uint16_t account_list_ack_build();

// 账户权限列表查看
uint16_t account_maclist_ack_build(account_all_info_t *account_all_info);

// 账户配置回复
uint16_t account_control_ack_build(uint8_t contorl,uint8_t id,uint8_t state);

// 方案列表查看
uint16_t solution_list_ack_build(uint16_t cmd);
// 方案列表配置回复
uint16_t solution_config_build(uint16_t id,uint8_t state,uint8_t config);

// 任务配置回复
uint16_t task_config_ack_build(uint16_t id,uint8_t state);

// 任务列表发送
uint16_t task_list_ack_build();

// 任务详细信息发送
uint16_t task_dtinfo_chk_build();

// 今日任务查询
uint16_t todaytask_ack_build();

// 即时任务配置回复
uint16_t rttask_config_ack_build(uint16_t id,uint8_t state);

// 即时任务列表获取
uint16_t rttask_list_chk_build();

// 即时任务详细信息查询
uint16_t rttask_dtinfo_chk_build(uint16_t id);

// 即时任务信息建立组包
uint16_t rttask_connect_build(uint8_t contorl,uint16_t ran_id,uint16_t id,uint16_t cmd,uint8_t no_task);

// 即时任务信息创建包
uint16_t rttask_creat_build(uint16_t ran_id,uint8_t state,uint16_t task_id);

// 话筒 用户 查询回复
uint16_t mic_userlist_ack_build(uint8_t state,account_all_info_t *account_all_info);

// 音乐文件列表查询
uint16_t music_patchlist_chk_build();

// 音乐列表名查询
uint16_t music_namelist_chk_build(uint8_t state);
// 文件操作进度查询
uint16_t file_progress_build(uint8_t state,uint8_t progress,uint8_t id[],uint8_t *name,uint8_t *patch);

// 批量处理回复
uint16_t file_batinfo_build(uint8_t *patch,uint8_t *file,uint8_t contorl,uint8_t bat_state,uint8_t state);

//注册回复
uint16_t host_resiged_ack_build(uint8_t state);

uint16_t sysonline_chk_build(uint8_t state);
// 消息通知
uint16_t listinfo_upgrade_build(uint8_t type);
// 任务通知
uint16_t taskinfo_upgrade_build(task_allinfo_tmp_t *task_allinfo_tmp,uint8_t contorl,uint16_t task_id);
// 即时任务通知
uint16_t rttaskinfo_upgrade_build(uint16_t id);
// 账户通知
uint16_t acinfo_upgrade_build(uint16_t id);
// 方案通知
uint16_t sulo_upgrade_build(uint8_t id);

// 备份控制 协议  B90A
uint16_t backup_contorl_build(uint8_t state,uint8_t *data);

// 备份消息推送 协议  B90B
uint16_t backup_updata_build(uint8_t state,uint8_t bar);
// 同步主机IP 协议  BF07
uint16_t sync_hostip_build(uint8_t mac[],uint8_t *ipaddr);

// 搜索设备列表 协议  BF09
uint16_t divsrc_list_build();

// 云服务器心跳 协议  BE00
uint16_t cld_heart_build();

// 云服务器注册查询 协议  BE01
uint16_t cld_resigerchk_build();

// 云注册申请 协议  BE08
uint16_t cld_resiger_request_build();

// 时间同步申请 BE03
uint16_t cld_timesysnc_request_build();


#if defined(__cplusplus) || defined(__XC__)
}
#endif


#endif //__ACK_BUILD_H_

