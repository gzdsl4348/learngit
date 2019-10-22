#ifndef  __DECODE_FUNLIST_H
#define  __DECODE_FUNLIST_H

#include <stdint.h>

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

// 接收心跳处理
void div_heart_recive(); //0xD000

// 接收上线包处理
void div_online_recive();   //0xB000

//-------------------------------------------------
// 设备列表请求管理
void divlist_request_recive(); //0xB100

void div_sending_decode(uint8_t list_num); //列表分包传送处理
//--------------------------------------------------

// 详细信息获取管理
void div_extra_info_recive(); //0xB102

// 设备信息配置管理
void div_info_set_recive(); //B104
//----------------------------------------------------
//分区信息请求
void arealist_request_rec();    // B200

void arealist_sending_decode(uint8_t list_num);
//
//分区配置
void area_config_recive();  //B202

//-----------------------------------------------------
// 设备连接超时处理 1HZ process
void div_heart_overtime_close();
//-----------------------------------------------------
// 设备IP MAC获取
void div_ip_mac_check_recive();

//-----------------------------------------------------
// 搜索设备信息获取 BF08
void research_lan_revice();
//-----------------------------------------------------
// 获取搜索设备列表
void sysset_divfound_recive();
//-----------------------------------------------------
// 配置所选搜索设备 目标主机IP
void divresearch_hostset_recive();
//-----------------------------------------------------
// 查找指定mac的IP
void divlist_ipchk_recive();

void divfound_over_timeinc();
//
void divsrc_sending_decode(uint8_t list_num);

// 云登录离线模式 C004
void offlinediv_mode_recive();

// 终端测试指令 BE0F
void div_textsend_recive();


#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif  //__DECODE_FUNLIST_H

