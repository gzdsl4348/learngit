/*
*  File Name:remotel_business_base.h
*  Created on: 2019年5月26日
*  Author: caiws
*  description :系统变量及基本操作函数定义
*  Modify date: 
* 	Modifier Author:
*  description :
*/


#ifndef __USER_UNTI_H_
#define __USER_UNTI_H_

#include <stdint.h>
#include <string.h>
#include "eth_audio.h"
#include "eth_audio_config.h"
#include "protocol_adrbase.h"
#include "sys_config_dat.h"
#include "debug_print.h"
#include "sys_log.h"


#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

uint8_t mac_cmp(uint8_t *a, uint8_t *b);

uint8_t ip_cmp(uint8_t *a, uint8_t *b);

uint8_t charncmp(uint8_t *c1,uint8_t *c2,unsigned len);

void watchdog_process();

void watchdog_clear();

#define sn_cmp(a,b) (charncmp(a,b,SYS_PASSWORD_NUM))  

#define NEED_FL_HOSTINFO    0x01
#define NEED_FL_AREALIST    0x02
#define NEED_FL_DIVLIST     0x04
#define NEED_FL_SOLUTION    0x08
#define NEED_FL_RTTASK_LIST 0x10

typedef struct xtcp_fifo_t
{
    unsigned int in_index;
    unsigned int out_index;
    unsigned int size;
} xtcp_fifo_t;

typedef struct bat_contorlobj_t
{
    uint8_t bat_state;
    uint8_t succeed_num;
    uint8_t fail_num;
    uint8_t remain_num;
    uint8_t bat_id[6];
}bat_contorlobj_t;

typedef struct g_sys_val_t{

    // TCP 包分割处理
    uint8_t tcp_buff_tmp[RX_BUFFER_SIZE];
    uint16_t tcp_tmp_len;
    uint16_t tcp_timout;
    uint8_t tcp_recing_f;
    uint8_t tcp_decode_f;

    //-------------------------------------------
    // flash 烧写标志位
    //unsigned need_flash;
    // flash 设备列表烧写计数位
    unsigned fl_divlist_inc;
    //
    tmp_union_l_t tmp_union;
    // 协议任务处理部分
    uint8_t task_busy;
    uint8_t task_pol_cmd;
    // 音乐路径
    uint16_t fsrc[(PATCH_NAME_NUM+MUSIC_NAME_NUM)/2];
    uint16_t fdes[(PATCH_NAME_NUM+MUSIC_NAME_NUM)/2];
    //-------------------------
    //任务列表 详细信息配置状态
    uint16_t task_recid;
    uint16_t task_con_id;
    uint16_t task_music_inc;
    uint16_t task_packrec_inc;
    uint16_t task_rec_count;
    uint8_t  task_con_state;
    uint8_t  task_creat_s;
    uint8_t  task_config_s;
    uint8_t  task_delete_s;
    uint8_t  task_dtinfo_setmusic_f;
    uint8_t  task_dtinfo_setdiv_f;
    //------------------------
    //批量音乐文件处理 状态
    xtcp_connection_t file_bat_conn;
    uint8_t file_bat_contorl_s;
    unsigned file_bat_tim;
    uint8_t file_batpack_inc;
    uint8_t file_bat_tolnum;
    uint8_t file_bat_musicinc;
    uint8_t file_bat_contorl;
    uint8_t file_bat_nametmp[MUSIC_NAME_NUM];
    uint8_t file_bat_srcpatch[PATCH_NAME_NUM];
    uint8_t file_bat_despatch[MUSIC_NAME_NUM];
    uint8_t file_bat_id[6];
    uint8_t file_bat_could_f;
    uint8_t file_bat_resendtim;
    uint8_t file_bat_resendf;
    uint8_t file_bat_resend_inc;
    uint8_t file_bat_overtime;

    uint8_t file_bat_resend_tmp[3];
    //---------------------------------------------------
    //优先级列表
    enum AUDIO_TYPE_E audio_type[NUM_MEDIA_INPUTS];
    //-------------------------------------------
    // 用户 flash
     uint8_t fl_account_id;
    //-------------------------------------------
    // 以太网连接状态
    uint8_t eth_link_state;
    //------------------------------------------
    // 时间信息
    time_info_t time_info;
    date_info_t date_info;
    //
    //-------------------------------------------
    // 任务连续播放 处理标志
    // 
    uint8_t task_wait_state[MAX_MUSIC_CH];
    uint8_t play_ok;
    uint16_t music_task_id[MAX_MUSIC_CH];
    uint8_t play_error_inc[MAX_MUSIC_CH];
    uint8_t play_rttask_f[MAX_MUSIC_CH];
    uint8_t rttask_musicset_f[MAX_MUSIC_CH];
    //-------------------------------------
    date_info_t today_date;     //今日日期
    //----------------------------------------
    // 任务显示
    uint8_t disp_furef;     //即将进行 正在进行 状态
    uint8_t disp_ch[MAX_DISP_TASK];
    uint8_t dispname_buff[MAX_DISP_TASK][DIV_NAME_NUM*2];
    uint8_t disinfo2buf[MAX_DISP_TASK][MUSIC_NAME_NUM];
    uint16_t disp_task_id[MAX_DISP_TASK];
    uint8_t disp_num;
    uint8_t disp_delay_inc;
    uint8_t disp_delay_f;
    // 文件操作
    uint16_t file_ack_cmd;
    xtcp_connection_t file_conn_tmp;
    uint8_t file_contorl_couldf;
    uint8_t file_contorl_id[6];

    uint8_t bat_contorling;
    bat_contorlobj_t bat_contorlobj[MAX_BATCONTORL_OBJ_NUM];
    //-----------------------------------------------
    // 信息更新部分
    //uint8_t connect_ip[4];
    //uint8_t connect_build_f;
    //uint8_t connect_send_f;
    // 
    //uint8_t messend_state;
    //uint8_t messend_inc;
    //uint16_t messend_len;
    //uint8_t messend_over_time;
    //uint8_t tx_buff[1472];
    //-------------------------------------------------
    // 重发标志
    uint8_t resend_inc;
     //---------------------------------
    // 按键标志
    uint8_t key_state;
    uint8_t key_delay;
    uint8_t wifi_io;
    //
    uint8_t key_wait_release;
    #define KEY_RESET_RELEASE 01
    #define KEY_WIFI_RELEASE  02
    //
    uint8_t key_wait_inc;
    //---------------------------------
    // wifi 模式
    uint8_t wifi_mode;
    #define WIFI_DHCPDIS_MODE 01
    #define WIFI_DHCPEN_MODE  02
    //
    uint8_t wifi_contorl_state;
    #define WIFI_WAIT_POWERON   01
    #define WIFI_AT_ENTER       02
    #define WIFI_AT_COM_DHCP    03
    #define WIFI_LANIP_SET      04
    #define WIFI_AT_SAVE        05
    #define WIFI_AT_SETNAME     06
    #define WIFI_AT_APPLY       07
    //
    uint8_t wifi_io_tmp;
    #define D_IO_WIFI_POWER     01
    #define D_IO_WIFI_CONTORL   02
    uint8_t wifi_timer;
    //---------------------------------
    // 重启标志
    uint8_t reboot_f;
    uint8_t reboot_inc;
    // 网关预备模式
    uint8_t gateway_standy;
    uint8_t gateway_time;
    uint8_t gateresend_inc;
    // SD卡状态
    uint8_t sd_state; //0 正常  1拔出
    // sys timer
    unsigned sys_timinc;
    // ----------------------------------------------
    // 发送通道时间戳
    unsigned tx_timestamp[MAX_MUSIC_CH];
    //-----------------------------------------------
    // 话筒占用状态
    #if 0
    uint8_t aux_ch_state[AUX_TYPE_NUM*AUX_RXCH_NUM];
    uint8_t aux_ch_tim[AUX_TYPE_NUM*AUX_RXCH_NUM];
    uint8_t aux_ch_ip[AUX_TYPE_NUM*AUX_RXCH_NUM][4];
    #endif
    //-----------------------------------------------------
    // 广播连接
    xtcp_connection_t broadcast_conn;
    // 云连接
    uint8_t could_send_cnt;
    xtcp_connection_t could_conn; //云链接状态
    xtcp_ipaddr_t could_ip; //云服务器ip
    uint8_t colud_connect_f;
    int colud_port;
    // DNS连接
    xtcp_ipaddr_t dns_ip;   //dns服务器ip
    xtcp_connection_t dns_conn;
    uint8_t dns_timecnt;
    uint8_t dns_resend_cnt;
    // ip 冲突连接
    xtcp_connection_t ipchk_conn;
    uint8_t ipchk_timecnt;
    uint8_t ipchk_ipconflict_f;
    // tftp 标志位
    uint8_t tftp_busy_f;
    uint8_t tftp_dat_ack;
    // 
    uint8_t tftp_dat_needack;
    //备份数据恢复状态
    uint8_t backup_busy_f;
    uint8_t backup_bar;
    uint8_t backup_timechk;
    uint8_t backup_resend;
    uint8_t backup_resend_inc;
    xtcp_connection_t backup_conn;
    //
    //机器码
    uint8_t maschine_code[10];
    //
    uint8_t register_code[10];
    xtcp_connection_t regsiter_conn;
    uint8_t register_rec_s_tmp;
    uint8_t register_need_send;
    uint8_t register_could_f;
    uint8_t register_could_id[6];
    // 系统部署
    // 搜索设备数
    unsigned search_div_tol;
    xtcp_connection_t divsearch_conn;
    uint8_t divsreach_f;
    uint8_t contorl_id[6];
    uint8_t divsreach_tim_inc;
    uint8_t divsreach_could_f;
    //
    uint8_t sn_key[DIV_NAME_NUM];
    //
    uint8_t con_id_tmp[6];
    // 即时任务列表更新
    rttask_info_t *rttask_updat_p;
    uint8_t rttask_updat_f;
    div_node_t *rttask_div_p;
    uint8_t rttask_up_ip[4];
    // 广播端口接收处理
    xtcp_connection_t brocast_rec_conn;
    uint8_t brocast_rec_timinc;

    // 云心跳计时
    uint8_t could_heart_timcnt;
    //
    uint8_t sys_dhcp_state_tmp;

    // 收发堆栈
    xtcp_fifo_t tx_buff_fifo;
    xtcp_fifo_t rx_buff_fifo;
    uint8_t tx_fifo_timout;
    uint8_t tcp_sending;
    uint8_t tcp_resend_cnt;
    uint16_t tx_fifo_len[MAX_TXBUFF_FIFOSIZE];
    uint16_t rx_fifo_len[MAX_RXBUFF_FIFOSIZE];

	//列表发送变量
	uint8_t list_sending_cnt;
	uint8_t list_sending_f;

	//内部测试变量
	uint8_t eth_debug_f;
	uint8_t eth_debug_des_ip[4];
	xtcp_connection_t debug_conn;

    // 日志处理
    log_info_t *log_info_p;
    uint8_t log_waitmk_f;
    // 主机IP获取管理
    uint8_t host_ipget_mode;
    uint8_t host_disp_tim;
    uint8_t host_clear_f;

    uint8_t reset_ethtim;
    
    uint8_t resetio_ethtmp;

    // 即时任务-主机音源
    //uint16_t rttask_music_recid;
    task_music_info_t  rttask_musinfo;

}g_sys_val_t;

extern g_sys_val_t g_sys_val;

extern tmp_union_t g_tmp_union;

#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__USER_UNTI_H_

