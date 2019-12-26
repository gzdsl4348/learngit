#ifndef __SYS_CONFIG_DAT_H_
#define __SYS_CONFIG_DAT_H_

#include <stdint.h>
#include "xtcp.h"
#include "protocol_adrbase.h"
#include "eth_audio_config.h"
#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

#define VERSION_H    (0x01)
#define VERSION_L    (0x30)

#define VERSION_TEN_H    (01)
#define VERSION_TEN_L    (30)

#define COULD_TCP_EN     1
#define DNS_IP     ("yunbo.itc-pa.cn")

#define LIST_TEXT_DEBUG 0 // 开启列表打印

#define NO_NEED_REGISTER 0 // 无需注册

#define ALL_ACCOUNT_ENTER   0 // 允许无限制登录

#define PRINT_CONNET_ACCOUNT 0 // 打印链接账号数

#define REGITSER_NEWVESION_DISP 1 // 云服务器显示 试用 过期 激活 未激活模式

#define DISABLE_COULDOFFLINE_MODE    0 // 关闭云离线停止工作模式

#define ENABLE_AUD_TRAINSMIT        0

#define DIV_PROBATION_DAY       90 //出厂试用期

#define ENGLISH_VERSION         0 //英文版

//-----------------------------------------------------
#define INIT_VAL -1	// None ID
//---------------------------------

//------------------------------------------------------------
// 即时任务 不能跨网络启动，因为原机制是使用ip查找对应的conn
//------------------------------------------------------------

// XTCP buffer size define 
#define TX_BUFFER_SIZE 1472
#define RX_BUFFER_SIZE 1472

// This PORT For Eth Data Communication Port
#define ETH_COMMUN_PORT	        8805    //协议交换端口
#define TCP_COULD_PROT          7003    //云服务器端口
#define ETH_DEBUG_PROT			8806    //DEBUG打印端口
#define ETH_DNS_PROT			53      //DNS端口
#define ETN_AUD_TRAINPORT       7002    //远程寻呼端口

#define  PC_CONFIG_TOOL_PORT  5121
#define LISTEN_BROADCAST_LPPORT 4094

// 最大UDP连接数
#define MAX_UDP_CONNET	160

#define MAX_ACCOUNT_CONNET (20) //同时支持30个控制机

#define MAX_ENTER_ACCOUNT   11  //最大同时登录账号

#define MAX_MESSAGE_SEND   (MAX_ENTER_ACCOUNT) //同时支持30个控制机

//最大设备
#define MAX_DIV_LIST    150

#define DIV_SEND_NUM    10  //每包发送设备列表数
//最大分区
#define MAX_AREA_NUM    100
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
#define MAX_RTTASK_CONTORL_NUM     5

// 最大方案数
#define MAX_TASK_SOULTION   10 //10 //4打铃 
// 每个方案最大任务数
#define MAX_SOUL_HAVETASK   100

// 任务最大歌曲数
#define MAX_MUSIC_NUM   20   //20首歌曲 
// 定时任务最大指定日期
#define MAX_TASK_DATE_NUM   10  //最大指定10天   
// 即时任务最大歌曲数
#define MAX_RTMUSIC_NUM     50   //50首歌曲 

#define SOLU_MAX_PLAYCH     8

// 最大MP3解码数
#define MAX_MUSIC_CH        NUM_MEDIA_INPUTS    //48路音频

// 最大显示任务数
#define MAX_DISP_TASK       4

// 发送数据堆栈
#define MAX_TXBUFF_FIFOSIZE   64
// 接收数据堆栈
#define MAX_RXBUFF_FIFOSIZE   16

// 最大记录日数
#define MAX_LOGDATE_NUM 7

// 注册信息
#define HOST_UNREGISTER             0 //未激活
#define HOST_REGISTER_DAYS          1 //有限期激活
#define HOST_REGISTER_FOREVER       2 //无限期激活
// 
#define MAX_SDCARD_MUSIC_NUM        100 //sd卡每个文件最大100个音乐
//
#define MES_STACK_NUM       8   //消息更新 堆栈数

#define MAX_SEND_ACCOUNT_NUM_FORPACK    10  //每数据包发送多少个账号

#define MAX_TASK_ONCESEND   10  //每数据包发送多少个任务
//
#define MAX_APP_AUD_TRAINSMIT_NUM   4 //最大手机寻呼路数

extern char *xtcp_tx_buf;
extern char *xtcp_rx_buf;

extern char all_rx_buf[RX_BUFFER_SIZE];
extern char all_tx_buf[TX_BUFFER_SIZE];

#define MAX_BATCONTORL_OBJ_NUM 5    //最大保存批量文件操作 恢复对象数

extern uint16_t user_sending_len;

extern xtcp_connection_t conn;

#define D_RTTASK_SOULID   0xFE

#define MAX_SEND_RTTASKINFO_NUM     10  // 即时任务信息推送用户，最大10个

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
    uint8_t mac_write_f;         // 是否已烧录MAC
    uint8_t div_have_register;  //是否已被注册
    uint8_t div_brand_f;
    uint8_t sys_dhcp_state; //WIFI的DHCP打开状态
    uint8_t log_daycnt;     //日志计数
    unsigned offline_day;
    uint8_t log_filename[MAX_LOGDATE_NUM][64];//日志名称
    uint8_t wav_mode;  // 0 关闭WAV   1 允许播放WAV
    uint8_t reset_data_f;
    //    
    date_info_t online_date_info;          // 记录日期
    
    uint8_t offline_f;           // 离线模式
    // wifi名称标志
    uint8_t wifi_have_setname;
    //uint8_t noneed_register;  // 0 无需注册   1 需要注册
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
    uint8_t div_onlineok;
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
    uint8_t state;  //空方案状态为 0xFF
    uint8_t name[DIV_NAME_NUM];
    date_info_t begin_date;
    date_info_t end_date;
    uint8_t prio;
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
    ONCE_PLAY_M,
    ONCE_LOOPPLAY_M,
};

typedef struct task_musicplay_t{
    uint16_t task_id;
    //uint8_t name[DIV_NAME_NUM]; //任务名称
    uint8_t music_inc;  //当前播放序号
    uint8_t music_tol;  //音乐总数
    uint8_t play_mode;  //播放模式
    uint8_t sulo_id;    //方案id
    uint32_t time_inc;  //播放时间
    uint32_t dura_time; //持续时间
    uint8_t rttask_f;   // 0 打铃任务  1 即时任务
    uint8_t task_vol;   //任务音量
    uint16_t music_tolsec;  //曲目总时间
    uint16_t music_sec;     //曲目当前时间
    uint8_t  play_state;    //播放状态    0：播放  1：暂停
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

// 主机音源信息
typedef struct rttask_host_t{
    uint8_t music_tol;      // 音乐总数
    uint8_t music_inc;      // 音乐计数
    uint16_t music_time;    // 音乐运行时间
    uint8_t play_modd;      // 播放模式
    uint8_t play_state;     // 播放状态
}rttask_host_t;


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
    //
    uint8_t    play_mode;
    uint8_t    music_tol;
    task_music_info_t music_info[MAX_RTMUSIC_NUM];
}rttask_dtinfo_t;

typedef struct rttask_info_t{
    uint16_t      rttask_id;
    uint32_t      dura_time;
    uint32_t      over_time;
    uint16_t      user_id;
    uint8_t       run_state;
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

typedef struct taskconflict_info_s{
    uint32_t bt[MAX_SOUL_HAVETASK];
    uint32_t et[MAX_SOUL_HAVETASK];
    uint8_t  state[MAX_SOUL_HAVETASK];
    uint8_t  next_t[MAX_SOUL_HAVETASK];
    uint8_t  state_bg[SOLU_MAX_PLAYCH];
}taskconflict_info_s;

typedef struct audts_divinfo_s{
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t area_info; 
}audts_divinfo_s;

typedef struct audts_divlist_s{
    uint8_t num;
    uint8_t id; //0,1，2，3
    uint8_t prio;
    unsigned timestamp;
    audts_divinfo_s divinfo[MAX_SENDCHAN_NUM];
}audts_divlist_s;

//=================================================================
// 临时变量
typedef union{
    task_allinfo_tmp_t task_allinfo_tmp;
    account_all_info_t account_all_info;
    rttask_dtinfo_t rttask_dtinfo;
    uint8_t buff[8*1024];
    xtcp_ipconfig_t ipconfig;
    xtcp_connection_t conn_tmp;
    audts_divlist_s audts_divlist;
    uint8_t buff2[4*1024];
}tmp_union_t;

typedef union{
    task_allinfo_tmp_t task_allinfo_tmp;
    account_all_info_t account_all_info;
    uint8_t buff[4*1024];
    xtcp_ipconfig_t ipconfig;
}tmp_union_l_t;

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
    void (*sending_fun)(uint8_t list_num); 
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
#define MAX_SEND_LIST_NUM   5

enum CONN_STATE_E{
    DIV_LIST_SENDING=0x00,
    AREA_LIST_SENDING=0x01,
    TASK_LIST_SENDING=0x02,
    TASK_DTINFO_SENDING=0X03,
    RTTASK_LIST_SENDING=0x04,
    PATCH_LIST_SENDING=0x05,
    MUSICNAME_LIST_SENDING=0x06,
    AC_LIST_SENDING=0x07,
    DIVSRC_LIST_SENDING=0x08,
    RTTASKMUSIC_LIST_SENDING=0x09,

	LIST_SEND_END=0xFE,
    LIST_SEND_INIT=0xFF,
};
//-------------------------------------------
// 设备列表发送状态
// 设备列表
typedef struct divlist_sending_t{
    uint16_t cmd;
    div_node_t *div_list_p;
}divlist_sending_t;
// 分区列表
typedef struct arealist_sending_t{
    uint16_t cmd;
    uint16_t area_inc;
}arealist_sending_t;
// 任务列表
typedef struct tasklist_sending_t{
    timetask_t *task_p;
    uint8_t  solu_id;
    uint8_t  solu_en;
    uint16_t cmd;
    uint8_t  task_tol;
}tasklist_sending_t;
// 任务详细信息
typedef struct task_dtinfo_sending_t{
    uint8_t  music_inc;
    uint16_t task_id;
}task_dtinfo_sending_t;
// 即时任务信息
typedef struct rttasklist_sending_t{
    rttask_info_t *rttask_p;
}rttasklist_sending_t;
// 文件夹名称信息
typedef struct patchlist_sending_t{
    uint8_t  patch_inc;
}patchlist_sending_t;

// 音乐名称信息
typedef struct musiclist_sending_t{
    uint8_t  music_inc;
    uint8_t  sector_index;
	uint8_t  music_state;
    uint8_t  music_patch_name[PATCH_NAME_NUM];
}musiclist_sending_t;

typedef struct account_sending_t{
    uint8_t account_inc;
    uint8_t could_send_en;
}account_sending_t;

typedef struct divsrc_sending_t{
    uint8_t div_inc;
}divsrc_sending_t;

typedef struct rttaskmusic_sending_t{
    uint8_t music_inc;
    uint16_t task_id;
}rttaskmusic_sending_t;


//---------------------------------------------------------------------
// senging 事件状态位 列表发送标记 同一时间只能往一个设备列表

typedef union{
    divlist_sending_t divlist;
    arealist_sending_t arealist;
    tasklist_sending_t tasklist;
    task_dtinfo_sending_t task_dtinfo;
    rttasklist_sending_t rttasklist;
    patchlist_sending_t patchlist;
    musiclist_sending_t musiclist;
    account_sending_t ac_list;
    divsrc_sending_t divsrc_list;
    rttaskmusic_sending_t rttaskmusic_ilst;
}conn_list_s;


typedef struct list_connsend_t{    
    uint16_t conn_state;  // 列表状态
    uint8_t pack_inc;   // 包序号
    uint8_t pack_tol;   // 包总数
    uint8_t tim_cnt;    // 超时计时器
    uint8_t could_s;    // 云标志
    uint8_t could_id[6];    // 云id
    xtcp_connection_t conn; // 连接信息
    conn_list_s list_info;  // 列表信息
}list_connsend_t;

extern list_connsend_t t_list_connsend[MAX_SEND_LIST_NUM];
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

#if 0
typedef struct lconn_unit_t{
    xtcp_connection_t conn;
    uint8_t id;
    unsigned tim_inc;
}lconn_unit_t;

typedef struct conn_long_list_t{
    lconn_unit_t lconn[MAX_LONG_CONNET];
}conn_long_list_t;

extern conn_long_list_t conn_long_list;
#endif

typedef struct messend_conn_t{
    xtcp_connection_t conn;
    uint8_t state;
    uint8_t could_id[6];
    uint8_t could_f;
    uint8_t over_timeinc;    
    uint8_t account_f;
}messend_conn_t;

typedef struct mes_send_list_t{
    messend_conn_t messend_conn[MAX_ACCOUNT_CONNET];
    //uint8_t tx_buff[MES_STACK_NUM][1472];
    uint8_t send_inc;
    uint16_t len[MES_STACK_NUM];
    uint8_t tim_inc;
    uint8_t wrptr;
    uint8_t rpttr;
}mes_send_list_t;

extern mes_send_list_t mes_send_list;

typedef struct rttask_info_list_t{
    xtcp_connection_t conn;
    uint16_t task_id;
    uint16_t user_id;
    uint8_t need_send;
    uint8_t could_f;
    uint8_t could_id[6];
}rttask_info_list_t;

extern rttask_info_list_t rttask_info_list[MAX_SEND_RTTASKINFO_NUM];

//======================================================================
void init_funlist_len();

#if defined(__cplusplus) || defined(__XC__)
}
#endif


#endif//    __SYS_CONFIG_DAT_H_

