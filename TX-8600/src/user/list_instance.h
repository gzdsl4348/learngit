#ifndef __LIST_INSTANCE_H_
#define __LIST_INSTANCE_H_

#include <stdint.h>
#include "xtcp.h"
#include "protocol_adrbase.h"
#include "eth_audio_config.h"
#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

#define VERSION_H    (0x01)
#define VERSION_L    (0x12)

#define COULD_TCP_EN 0

//-----------------------------------------------------
#define INIT_VAL -1	// None ID
//---------------------------------

// XTCP buffer size define 
#define TX_BUFFER_SIZE 1472
#define RX_BUFFER_SIZE 1472

// This PORT For Eth Data Communication Port
#define ETH_COMMUN_PORT	8805
#define TCP_COULD_PROT  7003
// 最大UDP连接数
#define MAX_UDP_CONNET	80

#define MAX_CONNET_LIST	(MAX_UDP_CONNET/4) //支持同时20部机建立任务

#define MAX_LONG_CONNET (MAX_UDP_CONNET/4) //同时支持20个控制机

//最大设备
#define MAX_DIV_LIST    80

#define DIV_SEND_NUM    10  //每包发送设备列表数
//最大分区
#define MAX_AREA_NUM    80 
// 设备所属最大分区
#define MAX_DIV_AREA    10
//
#define AREA_SEND_NUM   35  //每包发送区信息列表数
//最大账户列表
#define MAX_ACCOUNT_NUM 30
// 最大打铃任务数
#define MAX_RING_TASK_NUM   400
// 最大定时任务数
#define MAX_TIMED_TASK_NUM  50
// 
#define MAX_HOST_TASK   (MAX_RING_TASK_NUM+MAX_TIMED_TASK_NUM)
// 最大即时任务数
#define MAX_RT_TASK_NUM     50
// 同时开关即时任务用户数
#define MAX_RTTASK_CONTORL_NUM     10

// 最大方案数
#define MAX_TASK_SOULTION   10 //4打铃  
// 任务最大歌曲数
#define MAX_MUSIC_NUM   20   //20首歌曲 
// 定时任务最大指定日期
#define MAX_TASK_DATE_NUM   10  //最大指定10天   

// 最大MP3解码数
#define MAX_MUSIC_CH        NUM_MEDIA_INPUTS

#define HOST_UNREGISTER     0
#define HOST_REGISTER_DAYS          1
#define HOST_REGISTER_FOREVER       2

#define MAX_SDCARD_MUSIC_NUM        100

extern char *xtcp_tx_buf;
extern char *xtcp_rx_buf;

extern char all_rx_buf[RX_BUFFER_SIZE];
extern char all_tx_buf[TX_BUFFER_SIZE];

extern uint16_t user_sending_len;

extern xtcp_connection_t conn;

//=================================================================
// 日期信息
typedef struct task_dateinfo_t{
    uint8_t year;
    uint8_t month;
    uint8_t date;
}task_dateinfo_t;

typedef struct time_info_t{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}time_info_t;

typedef struct date_info_t{
    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t week;
}date_info_t;

//===================================================
//  host info  主机信息
//===================================================
typedef struct host_info_t{
    uint8_t mac[6];                     //MAC
    uint8_t name[DIV_NAME_NUM];         //名称
    uint8_t sn[SYS_PASSWORD_NUM];       //系统密码    
    uint8_t version[2];                 //版本
    uint8_t div_type[DIV_TYPE_NUM];     //设备类型
    uint8_t aux_type;                   //音频类型
    uint8_t hostmode;                   //主从模式
    uint8_t slient_en;                  //默音使能
    uint8_t slient_lv;                  //默音等级
    uint8_t dhcp_en;                    //dhcp 状态
    uint8_t regiser_state;              //注册状态
    uint16_t regiser_days;              //注册天数
    xtcp_ipconfig_t ipconfig;           //ip 信息
    uint8_t div_brand[DIV_NAME_NUM];              //设备品牌
    uint8_t regiser_code[3][20];        //保存注册码
    uint8_t regiser_inc;
}host_info_t;

extern host_info_t host_info;
extern host_info_t host_info_tmp;

//===================================================
//divice list def  设备列表定义
//===================================================
enum DIV_STATE_E{
    OFFLINE=0,
    ONLINE=1,
    BY_USED=2,
    CALLING=3,
    SN_ER=0xFF,
};

typedef struct div_info_t{  
    uint8_t name[DIV_NAME_NUM];
    xtcp_ipaddr_t ip;
    uint8_t mac[6];
    uint8_t vol;
    uint8_t div_state;      //0离线   1在线
    uint8_t div_type[DIV_NAME_NUM];
    uint8_t version[2];
    uint16_t area[MAX_DIV_AREA];
    uint16_t area_contorl[MAX_DIV_AREA];
    uint8_t id;
}div_info_t;

typedef struct div_node_t{
    div_info_t div_info;
    uint8_t over_time;
    struct div_node_t *next_p;
}div_node_t;

typedef struct div_list_t{
    div_node_t div_node[MAX_DIV_LIST];
    div_node_t *div_head_p;
    div_node_t *div_end_p;
    uint8_t     div_tol;
}div_list_t;

extern div_list_t div_list;

//=================================================================
//分区信息列表
//=================================================================
typedef struct area_info_t{
    uint16_t area_sn;
    uint8_t account_id;
    uint8_t area_name[DIV_NAME_NUM];
}area_info_t;

extern area_info_t area_info[];
extern uint16_t area_total_num;

//===================================================================
// 账户列表
//===================================================================
#define ADMIN_USER_TYPE  0
#define NOR_USER_TYPE    1

typedef struct account_info_t{
    uint8_t type;                   //账户类型
    uint8_t id;                     //账户编码
    uint8_t login_state;            //账户状态
    uint8_t name[DIV_NAME_NUM];     //账户名称
    uint8_t phone_num[DIV_NAME_NUM]; //手机号
    uint8_t sn[SYS_PASSWORD_NUM];   //登录密码
    date_info_t date_info;          //登录日期
    time_info_t time_info;          //登录时间
    date_info_t build_date_info;    //创建日期
    time_info_t build_time_info;    //创建时间
    uint8_t div_tol;                //账户可操作设备总数
    uint16_t over_time;             //连接时间
}account_info_t;

typedef struct account_all_info_t{
    account_info_t account_info;
    uint8_t mac_list[MAX_DIV_LIST*6];
}account_all_info_t;

extern account_info_t account_info[];

//===================================================================
// 任务列表
//===================================================================
//---------------------------------------------------
// 方案信息
typedef struct solu_info_t{
    uint8_t id;
    uint8_t data_en;     //方案是否在有效期内
    uint8_t en;     //方案生效失效
    uint8_t state;  //方案是否为空
    uint8_t name[DIV_NAME_NUM];
    date_info_t begin_date;
    date_info_t end_date;
}solu_info_t;
// 方案列表
typedef struct  solution_list_t{
    solu_info_t solu_info[MAX_TASK_SOULTION];
}solution_list_t;


extern solution_list_t solution_list;
//============================================================
//任务基础信息 
typedef struct timetask_t{
    uint16_t id;
    uint8_t solu_id;
    uint8_t task_en;
    uint8_t repe_mode; //重复模式
    uint8_t week_repebit;   //星期重复模式
    time_info_t time_info;  //开始时间
    task_dateinfo_t date_info[MAX_TASK_DATE_NUM];   //日期重复模式
    struct timetask_t *today_next_p;
    struct timetask_t *all_next_p;
}timetask_t;
//---------------------------------------------------
//任务列表基础信息
typedef struct timetask_list_t{
    timetask_t timetask[MAX_RING_TASK_NUM+MAX_TIMED_TASK_NUM];
    timetask_t *today_timetask_head;
    timetask_t *today_timetask_end;
    timetask_t *all_timetask_head;
    timetask_t *all_timetask_end;
    unsigned task_total;
}timetask_list_t;
//-------------------------------
//定时任务音乐播放信息
enum PLAY_MODE_E{
    ORDER_PLAY_M = 0,
    LOOP_PLAY_M,
    RANDOM_PLAY_M,
};

typedef struct task_musicplay_t{
    uint16_t task_id;
    uint8_t name[DIV_NAME_NUM];
    uint8_t music_inc;
    uint8_t music_tol;
    uint8_t play_mode;
    uint8_t sulo_id;
    uint32_t time_inc;
    uint32_t dura_time;
    time_info_t time_info;  //开始时间
}task_musicplay_t;

// 现在任务信息
typedef struct timetask_now_t{
    task_musicplay_t task_musicplay[MAX_MUSIC_CH];
    uint8_t ch_state[MAX_MUSIC_CH];
}timetask_now_t;

extern timetask_now_t timetask_now;
//---------------------------------------------------
//任务详细信息缓冲
typedef struct task_coninfo_t{
    uint8_t solution_sn;    //方案编号
    uint16_t task_id;       //任务编号
    uint8_t task_name[DIV_NAME_NUM];    //任务名称
    uint8_t task_state;
    uint8_t task_prio;
    uint8_t task_vol;
    uint8_t task_repe_mode;
    uint8_t week_repebit;
    task_dateinfo_t dateinfo[MAX_TASK_DATE_NUM];  //开始日期
    time_info_t time_info;  //开始时间
    uint32_t dura_time; //持续时间
    uint8_t play_mode;  //播放模式
    uint8_t div_tolnum; //设备总数
    uint8_t music_tolnum; //音乐总数
}task_coninfo_t;
//-------------------------------------------------
//任务播放目标MAC信息
typedef struct taskmac_info_t{
    uint16_t zone_control;
    uint8_t mac[6];
}taskmac_info_t;
//
//任务播放目标MAC列表
typedef struct task_maclist_t{
    taskmac_info_t taskmac_info[MAX_DIV_LIST];
}task_maclist_t;
//--------------------------------------------------
//音乐信息
typedef struct task_music_info_t{
    uint8_t music_path[PATCH_NAME_NUM];
    uint8_t music_name[MUSIC_NAME_NUM];
}task_music_info_t;
//
//播放音乐列表
typedef struct task_musiclist_t{
    task_music_info_t music_info[MAX_MUSIC_NUM];
}task_musiclist_t;
//
//任务缓冲信息
typedef struct task_allinfo_tmp_t{
    task_coninfo_t task_coninfo;
    task_musiclist_t task_musiclist;
    task_maclist_t task_maclist;
}task_allinfo_tmp_t;

//-----------------------------------------------------------------
extern timetask_list_t timetask_list;

//====================================================================
//即时任务
// 即时任务详细信息
typedef struct rttask_dtinfo_t{
    uint8_t     account_id; //所属账户编号
    uint16_t    rttask_id;  //即时任务编号
    uint8_t     name[DIV_NAME_NUM];   //即时任务名称
    uint8_t     src_mas[6]; //音源MAC
    uint8_t     task_vol;   //任务音量
    uint32_t    dura_time;  //持续时间
    uint8_t     task_key;   //遥控键值
    uint8_t     div_tol;    //设备总数
    uint8_t     prio;
    taskmac_info_t des_info[MAX_DIV_LIST];     
}rttask_dtinfo_t;

typedef struct rttask_info_t{
    uint16_t      rttask_id;
    uint32_t      dura_time;
    uint32_t      over_time;
    uint16_t      user_id;
    struct rttask_info_t *run_next_p; //运行中的任务
    struct rttask_info_t *all_next_p;  //所有任务
}rttask_info_t;

typedef struct rttask_lsit_t{
    rttask_info_t rttask_info[MAX_RT_TASK_NUM];
    uint8_t rttask_tol;
    struct rttask_info_t *run_head_p;
    struct rttask_info_t *run_end_p;
    struct rttask_info_t *all_head_p;
    struct rttask_info_t *all_end_p;
}rttask_lsit_t;
//-----------------------------------------------------------------
extern rttask_lsit_t rttask_lsit;

//=================================================================
// 临时变量
typedef union{
    task_allinfo_tmp_t task_allinfo_tmp;
    account_all_info_t account_all_info;
    rttask_dtinfo_t rttask_dtinfo;
    uint8_t buff[8*1024];
    xtcp_ipconfig_t ipconfig;
}tmp_union_t;
//
//================================================================
//fun list 
//================================================================
#ifndef __XC__
typedef struct rec_fun_lis_t{
    uint16_t cmd;
    void (*cmd_fun)(void); 
}rec_fun_lis_t; 

typedef struct con_fun_lis_t{
    uint16_t cmd;
    void (*cmd_fun)(uint8_t con_num); 
}con_fun_lis_t; 

// extern gobal val
extern uint16_t fun_list_len,connect_fun_list_len;
extern rec_fun_lis_t rec_fun_lis[]; //接收函数列表
//---------------------------------------------

typedef struct sending_fun_lis_t{
    void (*sending_fun)(void); 
}sending_fun_lis_t; 

extern sending_fun_lis_t sending_fun_lis[]; //列表发送函数表 xtcp sending 事件
extern con_fun_lis_t connect_fun_lis[];

#endif
//================================================================
// 显示属性
typedef struct disp_info_t{
    uint8_t disp_name[DIV_NAME_NUM];
}disp_info_t;

//===================================================
//即时任务建立状态
//===================================================
typedef struct rttask_build_state_t{
    int src_conn_id;
    int des_conn_id;
    uint8_t over_time;
    uint16_t rttask_id;
    uint16_t user_id;
    uint32_t dura_time;
    uint8_t contorl;
    uint8_t state;
}rttask_build_state_t;

extern rttask_build_state_t rttask_build_state[MAX_RTTASK_CONTORL_NUM];

//===================================================
//xtcp conn list def 链接列表定义
//===================================================
//
enum CONN_STATE_E{
    CONN_INIT=0,
    DIV_LIST_SENDING=0x01,
    AREA_LIST_SENDING=0x02,
    TASK_LIST_SENDING=0x04,
    TASK_DTINFO_SENDING=0X08,
    RTTASK_LIST_SENDING=0x10,
    PATCH_LIST_SENDING=0x20,
    MUSICNAME_LIST_SENDING=0x40,
    AC_LIST_SENDING=0x80,
    DIVSRC_LIST_SENDING=0x100,
};
//-------------------------------------------
// 设备列表发送状态
// 设备列表
typedef struct divlist_sending_t{
    uint8_t pack_total;
    uint8_t pack_inc;
    uint16_t cmd;
    uint8_t  id[6];
    div_node_t *div_list_p;
}divlist_sending_t;
// 分区列表
typedef struct arealist_sending_t{
    uint8_t  pack_total;
    uint8_t  pack_inc;
    uint16_t cmd;
    uint16_t area_inc;
    uint8_t  id[6];
}arealist_sending_t;
// 任务列表
typedef struct tasklist_sending_t{
    timetask_t *task_p;
    uint8_t  pack_inc;
    uint8_t  id[6];
}tasklist_sending_t;
// 任务详细信息
typedef struct task_dtinfo_sending_t{
    uint8_t  music_inc;
    uint8_t  pack_inc;
    uint8_t  id[6];
}task_dtinfo_sending_t;
// 即时任务信息
typedef struct rttasklist_sending_t{
    rttask_info_t *rttask_p;
    uint8_t  pack_inc;
    uint8_t  id[6];
}rttasklist_sending_t;
// 文件夹名称信息
typedef struct patchlist_sending_t{
    uint8_t  patch_inc;
    uint8_t  pack_inc;
    uint8_t  pack_tol;
    uint8_t  id[6];
}patchlist_sending_t;

// 音乐名称信息
typedef struct musiclist_sending_t{
    uint8_t  music_inc;
    uint8_t  sector_index;
    uint8_t  pack_inc;
    uint8_t  pack_tol;
    uint8_t  id[6];
}musiclist_sending_t;

typedef struct account_sending_t{
    uint8_t account_inc;
    uint8_t  pack_inc;
    uint8_t  pack_tol;
    uint8_t  id[6];
}account_sending_t;

typedef struct divsrc_sending_t{
    uint8_t div_inc;
    uint8_t  pack_inc;
    uint8_t  pack_tol;
    uint8_t  id[6];
}divsrc_sending_t;

//---------------------------------------------------------------------
// senging 事件状态位 列表发送标记 同一时间只能往一个设备列表
typedef struct conn_sending_s_t{    
    int id;
    uint16_t conn_state; 
    uint8_t conn_sending_tim;
    uint8_t could_s;
    xtcp_connection_t conn;
    divlist_sending_t divlist;
    arealist_sending_t arealist;
    tasklist_sending_t tasklist;
    task_dtinfo_sending_t task_dtinfo;
    rttasklist_sending_t rttasklist;
    patchlist_sending_t patchlist;
    musiclist_sending_t musiclist;
    account_sending_t ac_list;
    divsrc_sending_t divsrc_list;
}conn_sending_s_t;

extern conn_sending_s_t conn_sending_s;
//---------------------------------------------------------------------
typedef struct conn_list_t{
	xtcp_connection_t conn;
	struct conn_list_t *next_p;
    uint8_t over_time;
}conn_list_t;

//--------------------------------------------------------------------
// CONN主动连接链表
enum CONN_CONNECT_STATE{
    CONN_CONNECTING=0,
    CONN_ENDING,
};

//--------------------------------------------------------------------

extern conn_list_t conn_list[MAX_UDP_CONNET];

//======================================================================
// udp 长连接列表
//======================================================================

typedef struct lconn_unit_t{
    xtcp_connection_t conn;
    uint8_t id;
    unsigned tim_inc;
}lconn_unit_t;

typedef struct conn_long_list_t{
    lconn_unit_t lconn[MAX_LONG_CONNET];
}conn_long_list_t;

extern conn_long_list_t conn_long_list;

//======================================================================
void init_funlist_len();

#if defined(__cplusplus) || defined(__XC__)
}
#endif


#endif//    __LIST_INSTANCE_H_

