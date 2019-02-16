#ifndef __ACCOUNT_H_
#define __ACCOUNT_H_

#include <stdint.h>

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

void filename_decoder(uint8_t *buff,uint8_t num);

//-----------------------------------------
// 主机搜索
void user_host_search_recive();
//-----------------------------------------
// 账户登录处理
void account_login_recive();
//-----------------------------------------
// 账户列表查询
void account_userlist_recive();

void ac_list_sending_decode();
//-----------------------------------------
// 权限列表处理
void account_div_list_recive();
//-----------------------------------------
// 账户配置
void account_config_recive();
//-----------------------------------------
// 系统注册
void account_sys_register_recive();
//-----------------------------------------
// 系统在线监测
void account_sysonline_recive();
//-----------------------------------------
// 时间同步
void user_timer_sync_recive();

void cld_timer_sync_recive();
//-----------------------------------------
// 连接超时
void account_login_overtime();

// 话筒登录
void mic_userlist_chk_recive();

// 话筒寻呼请求
void mic_aux_request_recive();

// 话筒通道存活包
void mic_aux_heart_recive();

// 超时关闭话筒通道
void mic_time1hz_close();

//恢复操作繁忙查询
void backup_busy_chk();

// 恢复备份开始控制
void backup_contorl_chk();

// 恢复操作信息推送
void backup_mes_send_recive();

// 临时IP配置   BF09
void tmp_ipset_recive();

// 配置主机IP
void sysset_ipset_recive();

// 机器码生成
void maschine_code_init();

// 系统注册查询
void register_could_chk();

// 注册申请   BE08
void cld_register_request();

// 云注册申请回复
void cld_register_recive();

// 手机注册申请 B90D
void app_register_request();


// 时间同步申请 BE03
void cld_timesysnc_request();

// 账号云登录
void cld_account_login_recive();

// 账号列表推送
void account_list_updat();

void backup_sendmes_10hz();

#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__ACCOUNT_H_

