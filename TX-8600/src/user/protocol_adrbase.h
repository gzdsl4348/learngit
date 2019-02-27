#ifndef __PROTOCOL_ADRBASE_H
#define __PROTOCOL_ADRBASE_H

#define DIV_NAME_NUM    32  //22BYTE FOR DIVICE NAME
#define MUSIC_NAME_NUM  64  //
#define PATCH_NAME_NUM  32  //
#define SYS_PASSWORD_NUM 14  // SYSTEM PASSWORD NUMBER
#define DIV_TYPE_NUM  32

//=======================================================================
//command define 
#define COLUD_HEADER_TAG      0x69746379 

#define HEADER_TAG      0x55AA 
#define END_TAG         0xAA55 
//#define HEADER_TAG      0x5A5A 
//#define END_TAG         0xA5A5 

//
#define DIV_HEART_CMD           0xD000  
// 设备列表
#define ONLINE_REQUEST_CMD      0xB000
// 
#define DIVLIST_REQUEST_CMD     0xB100
#define DIVLIST_SEND_CMD        0xB101
#define DIV_EXTRA_INFO_CMD      0xB102
#define DIV_INFO_SET_CMD        0xB104
// 分区管理
#define AREA_GETREQUEST_CMD     0xB200
#define AREA_REFRESH_CMD        0xB201
#define AREA_CONFIG_CMD         0xB202
// 账户管理
#define ACCOUNT_LOGIN_CMD       0xB900
#define ACCOUNT_USER_LIST_CMD   0xB901
#define ACCOUNT_DIV_LIST_CMD    0xB902
#define ACCOUNT_CONFIG_CMD      0xB903
#define ACCOUNT_REGISTER_CMD    0xB904
#define ACCOUNT_SYSONLINE_CMD   0xB905
// 搜索主机
#define HOST_SEARCH_CMD         0xB906

// 时间同步
#define USER_TIMER_SYNC_CMD     0xBB00
#define CLD_TIMER_SYNC_CMD      0xBB01

// 定时任务
#define SOLUTION_CHECK_CMD      0xB300
#define TASK_CHECK_CMD          0xB301
#define TASK_UPDATE_CMD         0xB302
#define TASK_DTINFO_CK_CMD      0xB304
#define SOLUTION_CONFIG_CMD     0xB305
#define TASK_CONFIG_CMD         0xB306
#define TASK_DIINFO_CONFIG_CMD  0xB307
#define TASK_PLAYTEXT_CMD       0xB308
#define TASK_TODAYWEEK_CK_CMD   0xB309
#define TASK_CONFIG_WEEK_CMD    0xB30B
#define TASK_BAT_CONFIG_CMD     0xB30D
#define TASK_EN_CONFIG_CMD      0xB30E
#define TASK_BAT_DIVSET_CMD     0xB30F

// 即时任务
#define RTTASK_CHECK_CMD        0xB400
#define RTTASK_DTINFO_CHECK_CMD 0xB402 
#define RTTASK_CONFIG_CMD       0xB403
#define RTTASK_CONTORL_CMD      0xB404
#define RTTASK_BUILD_CMD        0xB407
#define RTTASK_REBUILD_CMD      0xB40A


// 报警采集
#define DIV_IPMAC_CHL_CMD       0xB709

// 寻呼话筒
#define MIC_USERLIST_CHK_CMD    0xB502
#define MIC_AUX_REQUEST_CMD     0xB503
#define MIC_AUX_CLOSE_CMD       0xB504

// 音乐库管理
#define MUSIC_PATCH_CHK_CMD     0xB800
#define MUSIC_LIB_CHK_CMD       0xB801

#define MUSIC_PATCHNAME_CON_CMD 0xB802
#define MUSIC_BUSY_CHK_CMD      0xB803

#define MUSIC_FILE_CONTORL_CMD  0xB804
#define MUSIC_PROCESS_BAR_CMD   0xB805

#define MUSIC_BAT_CONTORL_CMD   0xB806
#define MUSIC_BAT_STATE_CMD     0xB807
//----------------------------------------------------
// 列表更新消息
#define LISTINFO_UPDATA_CMD     0xBF02
#define DIVLIS_INFO_REFRESH        00
#define AREALIS_INFO_REFRESH       01
#define MUSICLIS_INFO_REFRESH      02
#define ACCOUNT_INFO_REFRESH       03
#define TODAYTASK_INFO_REFRESH     04
#define RTTASKERROR_INFO_REFRESH   05

// 定时任务更新
#define TIMETASK_UPDATA_CMD     0xB303
// 即时任务更新
#define RTTASK_UPDATA_CMD       0xB401
// 账户信息更新
#define ACCOUNT_UPDATA_CMD      0xB907
// 方案信息更新
#define SULO_UPDATA_CMD         0xB302

//-----------------------------------------------------
// 同步IP
#define SYNC_HOSTIP_CMD         0xBF07
//系统部署 
#define LAN_DIVRESEARCH_CMD      0xBF08
//获取搜索列表
#define SYSSET_DIVFOUNT_CMD      0xBF0C
// 配置设备IP
#define SYSSET_IPSET_CMD         0xBF0B
// 配置目标设备的主机IP
#define SYSSET_DIV_HOSTSET_CMD   0xBF0A
// 配置设备临时IP 
#define TMP_IPSET_CMD            0xBF09
//------------------------------------------------------
// 云协议
// 云心跳
#define CLD_HEART_CMD            0xBE00
// 注册查询
#define CLD_REGISTER_CHK_CMD     0xBE01
// 注册状态更新
#define CLD_REGISTER_RECIVE_CMD  0xBE02
// 申请时间同步
#define CLD_TIMESYNC_CMD         0xBE03
// 重置管理员密码
#define CLD_ADMINRESET_CMD       0xBE04
// 云登录申请
#define CLD_CLOULDLOGIN_CMD      0xBE06
// 云登录信息回复
#define CLD_REGISTER_INFO_CMD    0xBE07
// 申请注册
#define CLD_REGISTER_REQUEST_CMD 0xBE08

//备份操作
//备份操作繁忙查询
#define BACKUP_BUSY_CHK_CMD     0xB909
//备份控制
#define BACKUP_CONTORL_CMD      0xB90A
//备份状态推送
#define BACKUP_UPDATA_CMD       0xB90B

#define APP_CONNECTHAND_CMD     0xB90C

#define APP_REGISTER_CONTORL    0xB90D


//---------------------------------------------------------------------
// 云包头
#define CLH_TYPE_BASE           0                                   //4 云包头类型
#define CLH_LEN_BASE            (CLH_TYPE_BASE+4)                   //2 云包头长度
#define CLH_DESIP_BASE          (CLH_LEN_BASE+2)                    //4 目标IP
#define CLH_HOSTMAC_BASE        (CLH_DESIP_BASE+4)                  //6 主机MAC
#define CLH_CONTORL_ID_BASE     (CLH_HOSTMAC_BASE+6)                //6 控制ID
#define CLH_TRANTYPE_BASE       (CLH_CONTORL_ID_BASE+6)             //1 透传类型
#define CLH_DIVTYPE_BASE        (CLH_TRANTYPE_BASE+1)               //1 设备类型
#define CLH_NULLBTYE_BASE       (CLH_DIVTYPE_BASE+1)                //8 空字节

#define CLH_HEADEND_BASE        (CLH_NULLBTYE_BASE+8)               //云包头结束

//---------------------------------------------------------------------
// 基本协议头地址
#define POL_HEAD_BASE   0                   //2
#define POL_LEN_BASE    (POL_HEAD_BASE+2)   //2
#define POL_COM_BASE    (POL_LEN_BASE+2)    //2
#define POL_MAC_BASE    (POL_COM_BASE+2)    //6
#define POL_ID_BASE     (POL_MAC_BASE+6)    //6
#define POL_COULD_S_BASE (POL_ID_BASE+6)    //1            //云转发标识
#define POL_NULL_BASE     (POL_COULD_S_BASE+1)    //9

#define POL_DAT_BASE    (POL_NULL_BASE+9)

//====================================================================================================
// 云服务心跳包
//====================================================================================================
#define CLD_HEART_IP        (POL_DAT_BASE)      //4主机本地IP
#define CLD_HEART_MAC       (CLD_HEART_IP+4)    //6主机MAC
#define CLD_HEART_MASCHCODE (CLD_HEART_MAC+6)    //6主机MAC
#define CLD_HEART_VER       (CLD_HEART_MASCHCODE+10)   //2主机硬件版本
#define CLD_HEART_DIVTYPE   (CLD_HEART_VER+2)   //32主机型号
#define CLD_SYSNAME_BASE    (CLD_HEART_DIVTYPE+DIV_TYPE_NUM)   //32主机型号

#define CLD_HEART_DATEND    (CLD_SYSNAME_BASE+DIV_TYPE_NUM)     //

//====================================================================================================
// 方案查询包
//====================================================================================================
//ACK
#define SOLU_CK_TOLNUM      (POL_DAT_BASE)      //1 方案总数

#define SOLU_CK_DAT_BASE    (SOLU_CK_TOLNUM+1)   
#define SOLU_CK_ID          (0)                 //1 方案ID
#define SOLU_CK_STATE       (SOLU_CK_ID+1)      //1 方案状态
#define SOLU_CK_NAME        (SOLU_CK_STATE+1)   //32 方案名称
#define SOLU_CK_BEGDATE     (SOLU_CK_NAME+DIV_NAME_NUM)  //3 方案开始日期
#define SOLU_CK_ENDDATE     (SOLU_CK_BEGDATE+3)  //3 方案结束日期

#define SOLU_CK_LEN_END     (SOLU_CK_ENDDATE+3)

//====================================================================================================
// 方案更新包
//====================================================================================================
// 同上
//====================================================================================================
// 方案更新包
//====================================================================================================

//====================================================================================================
// 配置定时方案信息协议 B305
//====================================================================================================
// SEND
#define SOLU_CFG_SOLU_CONTORL   (POL_DAT_BASE)      //1 操作符
#define SOLU_CFG_SOLU_ID        (SOLU_CFG_SOLU_CONTORL+1)      //1 配置方案ID
#define SOLU_CFG_SOLU_CONFIGBIT (SOLU_CFG_SOLU_ID+1)      //1 配置位
#define SOLU_CFG_SOLU_STATE     (SOLU_CFG_SOLU_CONFIGBIT+1)   //1 配置方案状态 
#define SOLU_CFG_SOLU_NAME      (SOLU_CFG_SOLU_STATE+1) //22 配置方案名称
#define SOLU_CFG_SOLU_BEGDATE   (SOLU_CFG_SOLU_NAME+DIV_NAME_NUM) //3 配置方案开始日期   
#define SOLU_CFG_SOLU_ENDDATE   (SOLU_CFG_SOLU_BEGDATE+3) //3 配置方案开始日期   

#define SOLU_CFG_SOLU_LEN_END   (SOLU_CFG_SOLU_ENDDATE+3)
// ACK  
#define SOLU_CFGACK_ID          (POL_DAT_BASE)  //1
#define SOLU_CFGACK_CONFIG      (SOLU_CFGACK_ID+1) //1
#define SOLU_CFGACK_STATE       (SOLU_CFGACK_CONFIG+1) //1

#define SOLU_CFGACK_LENEND      (SOLU_CFGACK_STATE+1)

//====================================================================================================
// 任务列表查询包
//====================================================================================================
//ACK
#define TASK_CK_TOLPACK     (POL_DAT_BASE)          //1 总包数
#define TASK_CK_PACK_NUM    (TASK_CK_TOLPACK+1)     //1 本包序号
#define TASK_CK_TASK_TOL    (TASK_CK_PACK_NUM+1)    //1 本包任务总量

#define TASK_CK_DAT_BASE    (TASK_CK_TASK_TOL+1)
#define TASK_CK_SOLU_ID     (0)                         //1 方案编号
#define TASK_CK_TASK_ID     (TASK_CK_SOLU_ID+1)         //2 任务编号
#define TASK_CK_TASK_NAME   (TASK_CK_TASK_ID+2)         //22 任务名称
#define TASK_CK_TASK_STATE  (TASK_CK_TASK_NAME+DIV_NAME_NUM)    //1 任务状态
#define TASK_CK_TASK_PRIO   (TASK_CK_TASK_STATE+1)      //1 任务优先级
#define TASK_CK_TASK_VOL    (TASK_CK_TASK_PRIO+1)       //1 任务音量
#define TASK_CK_REPE_MODE   (TASK_CK_TASK_VOL+1)        //1 重复模式
#define TASK_CK_REPE_WEEK   (TASK_CK_REPE_MODE+1)       //1 按周重复
#define TASK_CK_REPE_DATE   (TASK_CK_REPE_WEEK+1)      //30 按日期重复
#define TASK_CK_BEG_TIME    (TASK_CK_REPE_DATE+3*MAX_TASK_DATE_NUM) //3 任务开始时间
#define TASK_CK_DURA_TIME   (TASK_CK_BEG_TIME+3)        //3 任务持续时间
#define TASK_CK_PLAY_MODE   (TASK_CK_DURA_TIME+3)       //1 任务播放模式
#define TASK_CK_PLAY_TOL    (TASK_CK_PLAY_MODE+1)       //1 任务播放的总曲目
#define TASK_CK_TEXTPLAY_S  (TASK_CK_PLAY_TOL+1)        //1 任务测试播放状态

#define TASK_CK_LEN_END     (TASK_CK_TEXTPLAY_S+1)      //1

//====================================================================================================
// 定时器更新定时任务协议
//====================================================================================================
// SEND 
#define TASK_SE_CONFIG      (POL_DAT_BASE)              //1 任务操作符
#define TASK_SE_SOLU_ID     (TASK_SE_CONFIG+1)          //1 方案编号
#define TASK_SE_TASK_ID     (TASK_SE_SOLU_ID+1)         //2 任务编号
#define TASK_SE_TASK_NAME   (TASK_SE_TASK_ID+2)         //22 任务名称
#define TASK_SE_TASK_STATE  (TASK_SE_TASK_NAME+DIV_NAME_NUM)    //1 任务状态
#define TASK_SE_TASK_PRIO   (TASK_SE_TASK_STATE+1)      //1 任务优先级
#define TASK_SE_TASK_VOL    (TASK_SE_TASK_PRIO+1)       //1 任务音量
#define TASK_SE_REPE_MODE   (TASK_SE_TASK_VOL+1)        //1 重复模式
#define TASK_SE_REPE_WEEK   (TASK_SE_REPE_MODE+1)       //1 按周重复
#define TASK_SE_REPE_DATE   (TASK_SE_REPE_WEEK+1)      //30 按日期重复
#define TASK_SE_BEG_TIME    (TASK_SE_REPE_DATE+3*MAX_TASK_DATE_NUM) //3 任务开始时间
#define TASK_SE_DURA_TIME   (TASK_SE_BEG_TIME+3)        //3 任务持续时间
#define TASK_SE_PLAY_MODE   (TASK_SE_DURA_TIME+3)       //1 任务播放模式
#define TASK_SE_PLAY_TOL    (TASK_SE_PLAY_MODE+1)       //1 任务播放的总曲目
#define TASK_SE_PLAY_STATE  (TASK_SE_PLAY_TOL+1)       //1 任务播放的状态

#define TASK_SE_LEN_END     (TASK_SE_PLAY_STATE+1)
// ACK

//====================================================================================================
// 获取定时任务详细信息协议 B304
//====================================================================================================
// SEND
#define TASK_DTG_TASK_ID    (POL_DAT_BASE)      //需获取信息的编号
// ACK
#define TASK_DTG_TOLPACK    (POL_DAT_BASE)          //1 总包数
#define TASK_DTG_PACK_NUM   (TASK_DTG_TOLPACK+1)    //1 包序号
#define TASK_DTG_PACK_TYPE  (TASK_DTG_PACK_NUM+1)   //1 包类型
#define TASK_DTG_ACK_ID     (TASK_DTG_PACK_TYPE+1)      //2 需获取信息的编号

// MAC LIST
#define TASK_DTG_DIV_TOL    (TASK_DTG_ACK_ID+2)  //1 设备总数   

#define TASK_DTG_DIV_BASE   (TASK_DTG_DIV_TOL+1)    

#define TASK_DTG_DIV_AREACFG    (0) //2 分区控制位
#define TASK_DTG_DIV_MAC        (TASK_DTG_DIV_AREACFG+2)  //6 MAC位

#define TASK_DTG_DIV_LEN        (TASK_DTG_DIV_MAC+6)

// MUSIC LIST
#define TASK_DTG_MUSIC_TOL    (TASK_DTG_ACK_ID+2)  //1 音乐总数 

#define TASK_DTG_MUSIC_BASE   (TASK_DTG_MUSIC_TOL+1)    

#define TASK_DTG_MUSIC_PATH   (0)                       //32 曲目路径
#define TASK_DTG_MUSIC_NAME   (TASK_DTG_MUSIC_PATH+PATCH_NAME_NUM)  //32 曲目名称

#define TASK_DTG_MUSIC_LEN    (TASK_DTG_MUSIC_NAME+MUSIC_NAME_NUM)

//====================================================================================================
// 配置定时任务信息协议 B306
//====================================================================================================
#define TASK_DTCFG_ID       (POL_DAT_BASE)          //2 需配置的定时任务ID
#define TASK_DTCFG_TOLPACK  (TASK_DTCFG_ID+2)       //1 总包数 
#define TASK_DTCFG_PACKNUM  (TASK_DTCFG_TOLPACK+1)       //1 当前包序号 
#define TASK_DTCFG_PACKTYPE (TASK_DTCFG_PACKNUM+1)       //1 包类型

// SEND 
#define TASK_CFG_CONTORL        (TASK_DTCFG_PACKTYPE+1)  // 1 定时任务操作符
#define TASK_CFG_CFGBIT         (TASK_CFG_CONTORL+1)    //1 定时任务配置位
//
#define TASK_CFG_SOLU_ID     (TASK_CFG_CFGBIT+1)          //1 方案编号
#define TASK_CFG_TASK_ID     (TASK_CFG_SOLU_ID+1)         //2 任务编号
#define TASK_CFG_TASK_NAME   (TASK_CFG_TASK_ID+2)         //22 任务名称
#define TASK_CFG_TASK_STATE  (TASK_CFG_TASK_NAME+DIV_NAME_NUM)    //1 任务状态
#define TASK_CFG_TASK_PRIO   (TASK_CFG_TASK_STATE+1)      //1 任务优先级
#define TASK_CFG_TASK_VOL    (TASK_CFG_TASK_PRIO+1)       //1 任务音量
#define TASK_CFG_REPE_MODE   (TASK_CFG_TASK_VOL+1)        //1 重复模式
#define TASK_CFG_REPE_WEEK   (TASK_CFG_REPE_MODE+1)       //1 按周重复
#define TASK_CFG_REPE_DATE   (TASK_CFG_REPE_WEEK+1)      //30 按日期重复
#define TASK_CFG_BEG_TIME    (TASK_CFG_REPE_DATE+3*MAX_TASK_DATE_NUM) //3 任务开始时间
#define TASK_CFG_DURA_TIME   (TASK_CFG_BEG_TIME+3)        //3 任务持续时间
#define TASK_CFG_PLAY_MODE   (TASK_CFG_DURA_TIME+3)       //1 任务播放模式
#define TASK_CFG_PLAY_TOL    (TASK_CFG_PLAY_MODE+1)       //1 任务播放的总曲目

#define TASK_CFG_LEN_END     (TASK_CFG_PLAY_TOL+1)
// ACK
#define TASKC_CFG_TASK_ID   (POL_DAT_BASE)              //2 应答方案编号
#define TASKC_CFG_STATE     (TASKC_CFG_TASK_ID+2)       //1 应答状态

#define TASKC_CFG_ACK_LEN   (TASKC_CFG_STATE+1)

//====================================================================================================
// 配置定时任务详细信息协议                     B307
//====================================================================================================
// SEND 
//
// MAC LIST
#define TASK_DTCFG_MACTOL   (TASK_DTCFG_PACKTYPE+1)     //1 MAC总数

#define TASK_DTCFG_MAC_BASE (TASK_DTCFG_MACTOL+1)       //

#define TASK_DTCFG_AREACFG  (0)                         //2 物理分区控制位
#define TASK_DTCFG_MAC      (TASK_DTCFG_AREACFG+2)      //6 MAC地址位    

#define TASK_DTCFG_MAC_LEN  (TASK_DTCFG_MAC+6)

// MUSIC LIST
#define TASK_DTCFG_MUSICTOL    (TASK_DTCFG_PACKTYPE+1)     //1 音乐总数

#define TASK_DTCFG_MUSIC_BASE  (TASK_DTCFG_MUSICTOL+1)

#define TASK_DTCFG_MUSICPATCH  (0)                         //32 音乐路径
#define TASK_DTCFG_MUSICNAME   (TASK_DTCFG_AREACFG+PATCH_NAME_NUM)     //32 音乐名称    

#define TASK_DTCFG_MUSICLEN    (TASK_DTCFG_MUSICNAME+MUSIC_NAME_NUM)   

//====================================================================================================
// 定时任务播放测试协议                  B308
//====================================================================================================
// SEND
#define TASK_PLAY_ID         (POL_DAT_BASE)     //2 播放任务ID
#define TASK_PLAY_CONTORL    (TASK_PLAY_ID+2)   //1 播放任务测试控制位

#define TASK_PLAY_LEN        (TASK_PLAY_CONTORL+1)

//====================================================================================================
// 定时任务批量编辑协议                  B30D
//====================================================================================================
// SEND
#define TASK_BAT_CONFIG_S         (POL_DAT_BASE)            //1 删除.开始时间变更操作符
#define TASK_BAT_DURATIME_S       (TASK_BAT_CONFIG_S+1)     //1 持续时间操作符
#define TASK_BAT_DURATIME         (TASK_BAT_DURATIME_S+1)   //3 持续时间
#define TASK_BAT_BEGTIME          (TASK_BAT_DURATIME+3)     //3 提前延后时间
#define TASK_BAT_TASKTOL          (TASK_BAT_BEGTIME+3)      //1 任务总数
#define TASK_BAT_TASKID           (TASK_BAT_TASKTOL+1)      //


//====================================================================================================
// 即时任务列表查询 B400
//====================================================================================================
// ACK
#define RTTASK_CK_TOLPACK   (POL_DAT_BASE)          //1 总包数
#define RTTASK_CK_PACKNUM   (RTTASK_CK_TOLPACK+1)   //1 当前包序号
#define RTTASK_CK_TASKTOL   (RTTASK_CK_PACKNUM+1)   //1 当前包任务总数 
//
#define RTTASK_CK_BASE      (RTTASK_CK_TASKTOL+1)
//
#define RTTASK_CK_ACID      (0)                     //1 账户ID
#define RTTASK_CK_TASKID    (RTTASK_CK_ACID+1)      //2 任务ID
#define RTTASK_CK_TASKNAME  (RTTASK_CK_TASKID+2)    //22 任务名称
#define RTTASK_CK_SRCMAC    (RTTASK_CK_TASKNAME+DIV_NAME_NUM)   //6 播放设备MAC
#define RTTASK_CK_TASKPRIO  (RTTASK_CK_SRCMAC+6)    //1 播放优先级 保留
#define RTTASK_CK_TASKVOL   (RTTASK_CK_TASKPRIO+1)  //1 任务音量
#define RTTASK_CK_DURATIME  (RTTASK_CK_TASKVOL+1)   //3 播放持续时间
#define RTTASK_CK_KEYINFO   (RTTASK_CK_DURATIME+3)  //1 遥控按键信息     
#define RTTASK_CK_STATE     (RTTASK_CK_KEYINFO+1)   //1

#define RTTASK_CK_LEN       (RTTASK_CK_STATE+1)

//====================================================================================================
// 即时任务详细信息查询 B402
//====================================================================================================
// SEND
#define RTTASK_DTCK_ID      (POL_DAT_BASE)  //2 查询详细信息的任务ID
// ACK
#define RTTASK_DTCK_ACKID   (POL_DAT_BASE)  //2
#define RTTASK_DTCK_DIVTOL  (RTTASK_DTCK_ACKID+2)  //1 设备总数

#define RTTASK_DTCK_DATBASE (RTTASK_DTCK_DIVTOL+1)

#define RTTASK_DTCK_DIVMAC  (0)             //6 设备MAC地址
#define RTTASK_DTCK_AREACONTORL (RTTASK_DTCK_DIVMAC+6)  //2 物理分区控制地址

#define RTTASK_DTCK_LEN     (RTTASK_DTCK_AREACONTORL+2)

//====================================================================================================
// 即时任务配置协议 B403
//====================================================================================================
// SEND 
#define RTTASK_CFG_CONTORL  (POL_DAT_BASE)  //1 任务操作符
#define RTTASK_CFG_ACID     (RTTASK_CFG_CONTORL+1)       //1 创建任务的账户编号
#define RTTASK_CFG_TASKID   (RTTASK_CFG_ACID+1)          //2 任务ID 
#define RTTASK_CFG_TASKNAME (RTTASK_CFG_TASKID+2)        //22 任务名称
#define RTTASK_CFG_SRCMAC   (RTTASK_CFG_TASKNAME+DIV_NAME_NUM)  //6 播放源MAC
#define RTTASK_CFG_TASKPRIO (RTTASK_CFG_SRCMAC+6)       //1 优先级 不使用 保留
#define RTTASK_CFG_TASKVOL  (RTTASK_CFG_TASKPRIO+1)     //1 任务音量
#define RTTASK_CFG_DURATIME (RTTASK_CFG_TASKVOL+1)      //3 持续时间
#define RTTASK_CFG_KETINFO  (RTTASK_CFG_DURATIME+3)     //1 遥控按键信息
#define RTTASK_CFG_DIVTOL   (RTTASK_CFG_KETINFO+1)      //1 播放目标总数

#define RTTASK_CFG_DIV_BASE (RTTASK_CFG_DIVTOL+1)

#define RTTASK_CFG_MAC          (0)                     //6 目标MAC             
#define RTTASK_CFG_AREACONTORL  (RTTASK_CFG_MAC+6)      //2 物理分区控制位

#define RTTASK_CFG_LEN          (RTTASK_CFG_AREACONTORL+2)

// ACK
#define RTTASK_CFGC_ID          (POL_DAT_BASE)              //2 即时任务编号
#define RTTASK_CFGC_STATE       (RTTASK_CFGC_ID+2)          //1 即时任务状态

#define RTTASK_CFGC_LEN         (RTTASK_CFGC_STATE+1)       // 

//====================================================================================================
// 即时任务列表更新 B401
//====================================================================================================
// 
#define RTTASK_REFRESH_STATE      (RTTASK_CFG_DIVTOL)             //
#define RTTASK_REFRESH_DATEND     (RTTASK_REFRESH_STATE+1)             //

//====================================================================================================
// 即时任务执行协议 B404
//====================================================================================================
// SEND
#define RTTASK_PLAY_TASKID   (POL_DAT_BASE)              //2 任务ID
#define RTTASK_PLAY_USERID   (RTTASK_PLAY_TASKID+2)              //2 任务ID
#define RTTASK_PLAY_CONTORL  (RTTASK_PLAY_USERID+2)      //1 任务控制位
// ACK
#define RTTASKC_PLAY_TASKID      (POL_DAT_BASE)  
#define RTTASKC_PLAY_USERID      (RTTASKC_PLAY_TASKID+2)    //2 控制ID
#define RTTASKC_PLAY_STATE       (RTTASKC_PLAY_USERID+2)           //1 控制状态

#define RTTASKC_PLAY_END         (RTTASKC_PLAY_STATE+1)

//====================================================================================================
// 即时任务分配协议 B407
//====================================================================================================
// SEND 
#define RTTASK_BUILD_CONTORL    (POL_DAT_BASE)                //1控制字符
#define RTTASK_BUILD_ID         (RTTASK_BUILD_CONTORL+1)      //2 任务id
#define RTTASK_BUILD_CONID      (RTTASK_BUILD_ID+2)           //2 控制ID
#define RTTASK_BUILD_PRIO       (RTTASK_BUILD_CONID+2)        //1 保留不使用
#define RTTASK_BUILD_DURATIME   (RTTASK_BUILD_PRIO+1)          //3 任务持续时间
#define RTTASK_BUILD_VOL        (RTTASK_BUILD_DURATIME+3)     //1 任务音量
#define RTTASK_BUILD_DIVTOL     (RTTASK_BUILD_VOL+1)          //1 设备总量

#define RTTASK_BUILD_BASE       (RTTASK_BUILD_DIVTOL+1)         //

#define RTTASK_BUILD_IP             (0)                             //4 播放设备IP
#define RTTASK_BUILD_MAC            (RTTASK_BUILD_IP+4)             //6 播放设备MAC
#define RTTASK_BUILD_AREACONTORL    (RTTASK_BUILD_MAC+6)            //2 播放设备分区控制位

#define RTTASK_BUILD_LEN            (RTTASK_BUILD_AREACONTORL+2)    //
//  ACK
#define RTTASK_BUILDC_STATE     (POL_DAT_BASE) //申请成功失败标志

//====================================================================================================
// 即时任务列表更新协议 BF0C
//====================================================================================================
#define RTTASK_LISTUP_DIVTOL        (POL_DAT_BASE)
#define RTTASK_LISTUP_LIST_BASE     (RTTASK_LISTUP_DIVTOL+1)

#define RTTASK_LISTUP_IP            (0)                             //4 播放设备IP
#define RTTASK_LISTUP_MAC           (RTTASK_LISTUP_IP+4)             //6 播放设备MAC
#define RTTASK_LISTUP_AREACONTORL   (RTTASK_LISTUP_MAC+6)            //2 播放设备分区控制位
#define RTTASK_LISTUP_LEN            (RTTASK_LISTUP_AREACONTORL+2)    //

//====================================================================================================
// ONLINE PACKPAGE 
//====================================================================================================
#define ONLINE_MAC_B         (POL_DAT_BASE)                       // 设备MAC 6byte
#define ONLINE_NAME_B        (POL_DAT_BASE+6)                     // 设备名称 22byte
#define ONLINE_STATE_B       (ONLINE_NAME_B+DIV_NAME_NUM)         // 设备状态 1byte
#define ONLINE_VOL_B         (ONLINE_STATE_B+1)                   // 设备默认音量 1byte
#define ONLINE_PASSWORD_B    (ONLINE_VOL_B+1)                     // 设备密码 6byte
#define ONLINE_DIV_TYPE_B    (ONLINE_PASSWORD_B+SYS_PASSWORD_NUM) // 设备类型 15byte
#define ONLINE_VERSION_B     (ONLINE_DIV_TYPE_B+DIV_TYPE_NUM)  // 版本字符 2byte
//
//ACK 
#define ONLINE_ACK_STATE_B   (POL_DAT_BASE)                       //上线状态 1byte
#define ONLINE_MASTERMODE_B  (ONLINE_ACK_STATE_B+1)               //主从模式状态 1byte
//---------------------------------------------------------------------

//====================================================================================================

//====================================================================================================
// DIVICE HEART PACKPAGE 
//====================================================================================================
#define HEART_MAC_B         (POL_DAT_BASE)                       // 设备MAC 6byte
#define HEART_NAME_B        (POL_DAT_BASE+6)                     // 设备名称 22byte
#define HEART_STATE_B       (HEART_NAME_B+DIV_NAME_NUM)          // 设备状态 1byte
#define HEART_VOL_B         (HEART_STATE_B+1)                    // 设备默认音量 1byte
#define HEART_NEEDACK_B     (HEART_VOL_B+1)                      // 是否需回复心跳包 1byte
//
//ACK
#define HEART_MASTERMODE_B  (POL_DAT_BASE)               //主从模式状态 1byte
//====================================================================================================

//====================================================================================================
// 设备配置 地址
//====================================================================================================
#define DIVSET_SETBITMASK_B         (POL_DAT_BASE)      //6
#define DIVSET_MAC_B                (POL_DAT_BASE+1)    //6
#define DIVSET_NAME_B               (DIVSET_MAC_B+6)    //22
#define DIVSET_IPINFO_B             (DIVSET_NAME_B+DIV_NAME_NUM)  //13
#define DIVSET_DHCPEN_B             (DIVSET_IPINFO_B)   //1
#define DIVSET_IP_B                 (DIVSET_DHCPEN_B+1)     //4
#define DIVSET_NETMASK_B            (DIVSET_IP_B+4)         //4
#define DIVSET_GATEWAY_B            (DIVSET_NETMASK_B+4)    //4
#define DIVSET_AUXTYPE_B            (DIVSET_IPINFO_B+13)//1 
#define DIVSET_SLIENT_B             (DIVSET_AUXTYPE_B+1)//2
#define DIVSET_SLIENT_EN_B          (DIVSET_SLIENT_B)
#define DIVSET_SLIENT_LV_B          (DIVSET_SLIENT_B+1)
#define DIVSET_VOL_B                (DIVSET_SLIENT_B+2) //1
#define DIVSET_OLDSN_B              (DIVSET_VOL_B+1)    //12 旧密码
#define DIVSET_NEWSN_B              (DIVSET_OLDSN_B+SYS_PASSWORD_NUM)    //12 旧密码
//
//ACK
#define DIVSET_SETSTATE             (POL_DAT_BASE)

//====================================================================================================
// 详细信息查询 地址
//====================================================================================================
//ACK
#define EXTRAINFO_MAC_B               (POL_DAT_BASE)  //6
#define EXTRAINFO_DHCP_EN_B           (EXTRAINFO_MAC_B+6)
#define EXTRAINFO_NETMAK_B            (EXTRAINFO_DHCP_EN_B+1) //4
#define EXTRAINFO_GATEWAY_B           (EXTRAINFO_NETMAK_B+4)  //4
#define EXTRAINFO_AUXTYPE_B           (EXTRAINFO_GATEWAY_B+4) //4
#define EXTRAINFO_SLIENT_B            (EXTRAINFO_AUXTYPE_B+1) //2
#define EXTRAINFO_LEN_END             (EXTRAINFO_SLIENT_B+2) 

//====================================================================================================
// 设备列表请求 地址
//====================================================================================================
//ACK
#define DIVLISTRE_TOTALPACK_B               (POL_DAT_BASE)   //1
#define DIVLISTRE_CURRENTPACK_B             (DIVLISTRE_TOTALPACK_B+1) //1
#define DIVLISTRE_TOTALDIV_B                (DIVLISTRE_CURRENTPACK_B+1) //1
#define DIVLISTRE_INFO_B                    (DIVLISTRE_TOTALDIV_B+1)

#define DIVLISTRE_NAME_B                    (0) //22
#define DIVLISTRE_IP_B                      (DIVLISTRE_NAME_B+DIV_NAME_NUM)   //4
#define DIVLISTRE_MAC_B                     (DIVLISTRE_IP_B+4)  //6
#define DIVLISTRE_VOL_B                     (DIVLISTRE_MAC_B+6) //1                  
#define DIVLISTRE_STATE_B                   (DIVLISTRE_VOL_B+1) //1
#define DIVLISTRE_TYPE_B                    (DIVLISTRE_STATE_B+1) //15
#define DIVLISTRE_VERSION_B                 (DIVLISTRE_TYPE_B+DIV_TYPE_NUM)  //2
#define DIVLISTRE_AREA_B                    (DIVLISTRE_VERSION_B+2) //10*2
#define DIVLISTRE_AREA_CONTORL_B            (DIVLISTRE_AREA_B+10*2) //10*2
#define DIVLISTRE_DIV_INFO_LEN              (DIVLISTRE_AREA_CONTORL_B+10*2)

//====================================================================================================       

//====================================================================================================
// 设备ip mac 列表请求协议 B709
//====================================================================================================
//ACK
#define DIVIPMAC_TOTALPACK_B               (POL_DAT_BASE)   //1
#define DIVIPMAC_CURRENTPACK_B             (POL_DAT_BASE+1) //1
#define DIVIPMAC_TOTALDIV_B                (DIVIPMAC_CURRENTPACK_B+1) //1

#define DIVIPMAC_DAT_BASE                  (DIVIPMAC_TOTALDIV_B+1)
#define DIVIPMAC_DAT_MAC                   (0)  //6
#define DIVIPMAC_DAT_IP                    (DIVIPMAC_DAT_MAC+6)  //4

#define DIVIPMAC_DTA_LEN                   (DIVIPMAC_DAT_IP+4)

//====================================================================================================
// 分区列表获取 地址
//====================================================================================================
//ACK
#define AREAGET_PACKTOTAL_B                 (POL_DAT_BASE)      //1
#define AREAGET_CURRENTPACK_B               (POL_DAT_BASE+1)    //1
#define AREAGET_TOTALAREA_B                 (AREAGET_CURRENTPACK_B+1)   //1
//
#define AREAGET_DAT_BASE_B                  (AREAGET_TOTALAREA_B+1)       
#define AREAGET_AREA_SN                     (0) //2
#define AREAGET_ACCOUNT_ID                  (AREAGET_AREA_SN+2) //1
#define AREAGET_AREA_NAME                   (AREAGET_ACCOUNT_ID+1) //32
#define AREAGET_DAT_END                     (AREAGET_AREA_NAME+DIV_NAME_NUM)

//====================================================================================================
// 分区列表更新 地址
//====================================================================================================
//ACK
#define AREAFRESH_PACKTOTAL_B               (POL_DAT_BASE)  //1
#define AREAFRESH_CURRENTPACK_B             (POL_DAT_BASE+1)    //1
#define AREAFRESH_TOTALAREA_B               (AREAFRESH_CURRENTPACK_B+1)   //1

//====================================================================================================
// 分区列表设置 地址
//====================================================================================================
#define AREASET_CONFIG_BYE_B                (POL_DAT_BASE)          //1
#define AREASET_AREA_SN_B                   (POL_DAT_BASE+1)        //2
#define AREASET_ACCONUT_ID                  (AREASET_AREA_SN_B+2)
#define AREASET_AREA_NAME_B                 (AREASET_ACCONUT_ID+1)   //32
#define AREASET_DIV_TOL_B                   (AREASET_AREA_NAME_B+DIV_NAME_NUM)   //1

#define AREASET_DIV_BASE_B                  (AREASET_DIV_TOL_B+1)
#define AREASET_DIV_MAC_B                   (0)   //6*N
#define AREASET_DIV_AREA_CONTORL_B          (AREASET_DIV_MAC_B+6)   //6*N


//====================================================================================================
// 音乐库 文件夹名称列表获取                MUSIC_PATCH_CHK_CMD     0xB800
//====================================================================================================
// ACK 
#define MUS_PTHCHK_PACKTOL                  (POL_DAT_BASE)          //1 总包数
#define MUS_PTHCHK_CURRENTPACK              (MUS_PTHCHK_PACKTOL+1)  //1 当前包序号  
#define MUS_PTHCHK_PATCHTOL                 (MUS_PTHCHK_CURRENTPACK+1)  //1

#define MUS_PTHCHK_DATBASE                  (MUS_PTHCHK_PATCHTOL+1) 
#define MUS_PTHCHK_PATCHNAME                (0)                     // 32  
#define MUS_PTHCHK_PATCHMUSICTOL            (MUS_PTHCHK_PATCHNAME+PATCH_NAME_NUM)  // 1  
#define MUS_PTHCHK_MUSICOVER_F              (MUS_PTHCHK_PATCHMUSICTOL+1)  // 1  

#define MUS_PTHCHK_DAT_LEN                  (MUS_PTHCHK_MUSICOVER_F+1)

//====================================================================================================
// 音乐库 详细音乐名称列表获取   MUSIC_LIB_CHK_CMD                 0xB801
//====================================================================================================
#define MUS_LIBHCK_CHKPATCH_NAME            (POL_DAT_BASE)          //32
//ACK
#define MUS_LIBCHK_PACKTOL                  (POL_DAT_BASE)          //1
#define MUS_LIBCHK_CURRENTPACK              (MUS_LIBCHK_PACKTOL+1)  //1
#define MUS_LIBCHK_MUSICTOL                 (MUS_LIBCHK_CURRENTPACK+1)  //1

#define MUS_LIBCHK_PATCHNAME                (MUS_LIBCHK_MUSICTOL+1) //32

#define MUS_LIBCHK_DATBASE                  (MUS_LIBCHK_PATCHNAME+PATCH_NAME_NUM)

#define MUS_LIBCHK_MUSICNAME                (0) //  32
#define MUS_LIBCHK_DURATIME                 (MUS_LIBCHK_MUSICNAME+MUSIC_NAME_NUM)   //2

#define MUS_LIBCHK_DAT_LEN                  (MUS_LIBCHK_DURATIME+2)

//====================================================================================================
// 音乐库 音乐库名称修改                   MUSIC_PATCHNAME_CON_CMD   0xB802
//====================================================================================================
//SEND
#define MUS_PTHCON_CONFIG              (POL_DAT_BASE)
#define MUS_PTHCON_SRCNAME             (MUS_PTHCON_CONFIG+1)      //32 
#define MUS_PTHCON_DESNAME             (MUS_PTHCON_SRCNAME+PATCH_NAME_NUM)    //32

#define MUS_PTHCON_LEN                 (MUS_PTHCON_DESNAME+PATCH_NAME_NUM)

//====================================================================================================
// 音乐库 上传状态查询                   MUSIC_BUSY_CHK_CMD   0xB803
//====================================================================================================
// one byte

//====================================================================================================
// 音乐库 文件操作                    MUSIC_FILE_CONTORL_CMD   0xB804
//====================================================================================================
#define FILECON_CONTORL_B                   (POL_DAT_BASE)      //1
#define FILECON_SRC_PATCHNAME               (FILECON_CONTORL_B+1)   //32
#define FILECON_SRC_MUSICNAME               (FILECON_SRC_PATCHNAME+PATCH_NAME_NUM) //32
#define FILECON_DES_PATCHNAME               (FILECON_SRC_MUSICNAME+MUSIC_NAME_NUM)

#define FILECON_DAT_LEN                     (FILECON_DES_PATCHNAME+PATCH_NAME_NUM)

//====================================================================================================
// 音乐库 进度查询                    MUSIC_PROCESS_BAR_CMD    0xB805
//====================================================================================================
#define FILEPRO_STATE_B                     (POL_DAT_BASE)      //1 操作状态
#define FILEPRO_BAR_B                       (FILEPRO_STATE_B+1) //1 进度提示

#define FILEPRO_DAT_LEN                     (FILEPRO_BAR_B+1)

//====================================================================================================
// 音乐 批量操作                    MUSIC_PROCESS_BAR_CMD    0xB806
//====================================================================================================
#define MUSIC_BAT_PACKTOL                 (POL_DAT_BASE)                //1 包总数  
#define MUSIC_BAT_PACKINC                 (MUSIC_BAT_PACKTOL+1)         //1 包序号  
#define MUSIC_BAT_CONTORL                 (MUSIC_BAT_PACKINC+1)         //1 操作符  
#define MUSIC_BAT_SRC_PATCH               (MUSIC_BAT_CONTORL+1)         //32 源文件夹名称  
#define MUSIC_BAT_DES_PATCH               (MUSIC_BAT_SRC_PATCH+PATCH_NAME_NUM) //32 目标文件夹名称  
#define MUSIC_BAT_TOL_NUM                 (MUSIC_BAT_DES_PATCH+PATCH_NAME_NUM) //1 音乐文件总数
#define MUSIC_BAT_NAME_BASE               (MUSIC_BAT_TOL_NUM+1)                //

//====================================================================================================
// 音乐 批量操作应答                  MUSIC_BAT_STATE_CMD    0xB807
//====================================================================================================
#define MUSIC_BATINFO_CONTORL            (POL_DAT_BASE)                 //1 操作符
#define MUSIC_BATINFO_STATE              (MUSIC_BATINFO_CONTORL+1)      //1 批量状态符
#define MUSIC_BATINFO_FILESTATE          (MUSIC_BATINFO_STATE+1)        //1 文件状态符
#define MUSIC_BATINFO_PATCH              (MUSIC_BATINFO_FILESTATE+1)    //32 文件夹名
#define MUSIC_BATINFO_FILE               (MUSIC_BATINFO_PATCH+PATCH_NAME_NUM)   //64 文件名
#define MUSIC_BATINFO_LEN_END            (MUSIC_BATINFO_FILE+MUSIC_NAME_NUM)

//====================================================================================================
// 账户登录设置 地址
//====================================================================================================
#define A_LOGIN_NAME_B                      (POL_DAT_BASE)  //22
#define A_LOGIN_SN_B                        (A_LOGIN_NAME_B+DIV_NAME_NUM) //14
#define A_LOGIN_DIV_STATE_B                 (A_LOGIN_SN_B+14)

//ACK
#define AC_LOGIN_NAME_B                     (POL_DAT_BASE)  //22
#define AC_LOGIN_PHONENUM_B                 (AC_LOGIN_NAME_B+DIV_NAME_NUM)     
#define AC_LOGIN_STATE_B                    (AC_LOGIN_PHONENUM_B+DIV_NAME_NUM) //1
#define AC_LOGIN_SYSSN_B                    (AC_LOGIN_STATE_B+1)    //6
#define AC_LOGIN_ACCOUNT_TYPE_B             (AC_LOGIN_SYSSN_B+SYS_PASSWORD_NUM)    //1
#define AC_LOGIN_ACCOUNT_ID_B               (AC_LOGIN_ACCOUNT_TYPE_B+1) //1
#define AC_LOGIN_SYS_MAC_B                  (AC_LOGIN_ACCOUNT_ID_B+1)  //6
#define AC_LOGIN_SYS_MACHCODE_B             (AC_LOGIN_SYS_MAC_B+6)  //10
#define AC_LOGIN_SYS_TYPE_B                 (AC_LOGIN_SYS_MACHCODE_B+10)     //32         设备类型
#define AC_LOGIN_SYS_BRAND_B                (AC_LOGIN_SYS_TYPE_B+DIV_TYPE_NUM)   //32 设备品牌
#define AC_LOGIN_SYS_NAME_B                 (AC_LOGIN_SYS_BRAND_B+DIV_TYPE_NUM) //设备名称
#define AC_LOGIN_SYS_VERSION_B              (AC_LOGIN_SYS_NAME_B+DIV_TYPE_NUM)  //设备版本
#define AC_LOGIN_DHCP_EN_B                  (AC_LOGIN_SYS_VERSION_B+2)  //1
#define AC_LOGIN_IPMASK_B                   (AC_LOGIN_DHCP_EN_B+1)     //4
#define AC_LOGIN_IPGATE_B                   (AC_LOGIN_IPMASK_B+4)      //4
#define AC_LOGIN_RES_STATE_B                (AC_LOGIN_IPGATE_B+4)      //1
#define AC_LOGIN_RES_DAY_B                  (AC_LOGIN_RES_STATE_B+1)   //2
#define AC_LOGIN_DIV_TOL_B                  (AC_LOGIN_RES_DAY_B+2)     //1
#define AC_LOGIN_DIV_MAC_B                  (AC_LOGIN_DIV_TOL_B+1)     //6*N

//====================================================================================================
// 账户列表查看       B901
//====================================================================================================
//ACK
#define AC_LISTCK_TOLPAGE_B                 (POL_DAT_BASE)   
#define AC_LISTCK_PAGENUM_B                 (AC_LISTCK_TOLPAGE_B+1)
#define AC_LISTCK_TOLNUM_B                  (AC_LISTCK_PAGENUM_B+1)  //1
//
#define AC_LISTCK_DAT_BASE                  (AC_LISTCK_TOLNUM_B+1) //BASE
#define AC_LISTCK_TYPE_B                    (0) //1
#define AC_LISTCK_NUM_B                     (AC_LISTCK_TYPE_B+1)  //1
#define AC_LISTCK_NAME_B                    (AC_LISTCK_NUM_B+1)     //32
#define AC_LISTCK_PHONE_NUM_B               (AC_LISTCK_NAME_B+DIV_NAME_NUM)
#define AC_LISTCK_SN_B                      (AC_LISTCK_PHONE_NUM_B+DIV_NAME_NUM)   //6
#define AC_LISTCK_DATE_B                    (AC_LISTCK_SN_B+SYS_PASSWORD_NUM)    // 3
#define AC_LISTCK_TIME_B                    (AC_LISTCK_DATE_B+3)      //3
#define AC_LISTCK_BUILD_DATE_B              (AC_LISTCK_TIME_B+3)    // 3
#define AC_LISTCK_BUILD_TIME_B              (AC_LISTCK_BUILD_DATE_B+3)      //3
#define AC_LISTCK_DIV_TOL_B                 (AC_LISTCK_BUILD_TIME_B+3) //1
//
#define AC_LISTCK_DATLEN_B                  (AC_LISTCK_DIV_TOL_B+1)                     

//====================================================================================================
// 账户可操作的设备列表查看
//====================================================================================================
#define A_MACLIST_ID_B                     (POL_DAT_BASE) //1
//
//ACK
#define AC_MACLIST_ID_B                    (POL_DAT_BASE)  //1
#define AC_MACLIST_TOL_B                   (AC_MACLIST_ID_B+1)    //1
#define AC_MACLIST_MAC_B                   (AC_MACLIST_TOL_B+1)    //6*N

//====================================================================================================
// 账户管理配置协议
//====================================================================================================
#define A_CONFIG_CONTORL_B                 (POL_DAT_BASE)  //1
#define A_CONFIG_ACNUM_B                   (A_CONFIG_CONTORL_B+1) //1
#define A_CONFIG_NAME_B                    (A_CONFIG_ACNUM_B+1) //22
#define A_CONFIG_ACTYPE_B                  (A_CONFIG_NAME_B+DIV_NAME_NUM)   //1
#define A_CONFIG_AC_SN_B                   (A_CONFIG_ACTYPE_B+1)  //6
#define A_CONFIG_PHONE_NUM_B               (A_CONFIG_AC_SN_B+SYS_PASSWORD_NUM)
#define A_CONFIG_AC_DIVTOL_B               (A_CONFIG_PHONE_NUM_B+DIV_NAME_NUM)  //1
#define A_CONFIG_AC_DIV_MAC_B              (A_CONFIG_AC_DIVTOL_B+1)  //1
//
//ACK
#define AC_CONFIG_CONTORL_B                (POL_DAT_BASE)   //1
#define AC_CONFIG_ACNUM_B                  (AC_CONFIG_CONTORL_B+1)  //1
#define AC_CONFIG_ACKS_B                   (AC_CONFIG_ACNUM_B+1)    //1

#define AC_CONFIG_ENDLEN_B                 (AC_CONFIG_ACKS_B+1)

//====================================================================================================
// 系统回复协议        B905
//====================================================================================================
#define SYS_ONLINE_CHK_DATA_B              (POL_DAT_BASE) 
#define SYS_ONLINE_CHK_TIME_B              (SYS_ONLINE_CHK_DATA_B+3) 
#define SYS_ONLINE_CHK_SD_B                (SYS_ONLINE_CHK_TIME_B+3) 
#define SYS_ONLINE_CHK_DIVSTATE_B          (SYS_ONLINE_CHK_SD_B+1) 
#define SYS_ONLINE_CHK_TASKTOL_B           (SYS_ONLINE_CHK_DIVSTATE_B+1) 
#define SYS_ONLINE_CHK_TASKID_B            (SYS_ONLINE_CHK_TASKTOL_B+1)

//#define SYS_ONLINE_CHK_LEN                 (SYS_ONLINE_CHK_SD_B+1)

//====================================================================================================
// 注册协议
//====================================================================================================
#define A_RES_CODE_XOR                     (POL_DAT_BASE) //5

#define A_RES_CODE_STATE                   (A_RES_CODE_XOR+5)   //1
#define A_RES_CODE_DATA                    (A_RES_CODE_STATE+1) //6
#define A_RES_CODE_MAC                     (A_RES_CODE_DATA+6) //6
#define A_RES_CODE_RESDAY                  (A_RES_CODE_MAC+6)

// Ack
#define AC_RES_STATE                       (POL_DAT_BASE) //1
#define AC_RES_CODE                        (AC_RES_STATE+1) //20

#define AC_RES_LENEND                      (AC_RES_CODE+20) 

//====================================================================================================
// 定时器在线状态查询 
//====================================================================================================
#define ACCOUNT_ONLINECEK_ID_B              (POL_DAT_BASE)  //1
//ACK
#define ACCOUNT_SYSONLINE_CEK_B             (POL_DAT_BASE)

//====================================================================================================
// 话筒查询用户权限列表及登录 查询 B502 
//====================================================================================================
//SEND
#define MIC_USERCHK_NAME_B                  (POL_DAT_BASE)  //22
#define MIC_USERCHK_SN_B                    (MIC_USERCHK_NAME_B+DIV_NAME_NUM)
//ACK
#define MIC_USERCHK_STATE_B                 (POL_DAT_BASE)  //1
#define MIC_USERCHK_USERID_B                (MIC_USERCHK_STATE_B+1)  //1
#define MIC_USERCHK_TOL_B                   (MIC_USERCHK_USERID_B+1) //1

#define MIC_USERCHK_DATBASE                 (MIC_USERCHK_TOL_B+1)
#define MIC_USERCHK_MAC                     (0)

#define MIC_USERCHK_END_LEN                 (MIC_USERCHK_MAC+6)

//====================================================================================================
// 临时IP配置 BF09
//====================================================================================================
#define TMP_IPSET_STATE                     (POL_DAT_BASE)          //1 应答状态
#define TMP_IPSET_MAC                       (TMP_IPSET_STATE+1)     //6 MAC地址
#define TMP_IPSET_IP                        (TMP_IPSET_MAC+6)
#define TMP_IPSET_MASK                      (TMP_IPSET_IP+4)
#define TMP_IPSET_GATE                      (TMP_IPSET_MASK+4)

#define TMP_IPSET_DATEND                    (TMP_IPSET_GATE+4)

//====================================================================================================
// 时间同步
//====================================================================================================
//
#define USER_TIMSYNC_STATE_B                (POL_DAT_BASE)  //1
#define USER_TIMSYNC_YEAR_B                 (USER_TIMSYNC_STATE_B+1)    //1
#define USER_TIMSYNC_MONTH_B                (USER_TIMSYNC_YEAR_B+1)    //1
#define USER_TIMSYNC_DAY_B                  (USER_TIMSYNC_MONTH_B+1)    //1
#define USER_TIMSYNC_HOUR_B                 (USER_TIMSYNC_DAY_B+1)    //1
#define USER_TIMSYNC_MINUTE_B               (USER_TIMSYNC_HOUR_B+1)    //1
#define USER_TIMSYNC_SECOND_B               (USER_TIMSYNC_MINUTE_B+1)    //1
#define USER_TIMSYNC_WEEK_B                 (USER_TIMSYNC_SECOND_B+1)    //1

#define USER_TIMSYNC_END_B                  (USER_TIMSYNC_WEEK_B+1)    //1

//====================================================================================================
// 系统部署 BF08
//====================================================================================================
#define DIVSRC_RECIVE_STATE                 (POL_DAT_BASE)
#define DIVSRC_DAT_BASE                     (DIVSRC_RECIVE_STATE+1)
//
#define DIVSRC_MAC_B                        (0)  //6
#define DIVSRC_NAME_B                       (DIVSRC_MAC_B+6)//32
#define DIVSRC_STATE_B                      (DIVSRC_NAME_B+DIV_NAME_NUM) //1
#define DIVSRC_VOL_B                        (DIVSRC_STATE_B+1) //1
#define DIVSRC_PASSWORD_B                   (DIVSRC_VOL_B+1) //系统密码
#define DIVSRC_TYPE_B                       (DIVSRC_PASSWORD_B+SYS_PASSWORD_NUM) //32
#define DIVSRC_VERSION_B                    (DIVSRC_TYPE_B+DIV_NAME_NUM) //2
#define DIVSRC_HOSTIP_B                     (DIVSRC_VERSION_B+2) //4 
#define DIVSRC_DHCPEN_B                     (DIVSRC_HOSTIP_B+4) //4 
#define DIVSRC_DIVIP_B                      (DIVSRC_DHCPEN_B+1) //4 
#define DIVSRC_DIVMASK_B                    (DIVSRC_DIVIP_B+4) //4 
#define DIVSRC_DIVGATE_B                    (DIVSRC_DIVMASK_B+4) //4 

#define DIVSRC_DATEND_B                     (DIVSRC_HOSTIP_B+4)

//====================================================================================================
// 系统部署配置目标主机 BF0A
//====================================================================================================
#define SYSSET_HOSTIP_PTOL_B                (POL_DAT_BASE)  //6
#define SYSSET_HOSTIP_PNUM_B                (SYSSET_HOSTIP_PTOL_B+1)//1
#define SYSSET_HOSTIP_HOSTIP_B              (SYSSET_HOSTIP_PNUM_B+1)//4
#define SYSSET_HOSTIP_DIVTOL_B              (SYSSET_HOSTIP_HOSTIP_B+4)//1
#define SYSSET_HOSTIP_DIVMAC_B              (SYSSET_HOSTIP_DIVTOL_B+1)//1


//====================================================================================================
// 搜索列表上传     BF09
//====================================================================================================
#define DIVSRC_LIST_TOLPACK                 (POL_DAT_BASE)
#define DIVSRC_LIST_PACKNUM                 (DIVSRC_LIST_TOLPACK+1)
#define DIVSRC_LIST_DIVTOL                  (DIVSRC_LIST_PACKNUM+1)

#define DIVSRC_LIST_DAT_BASE                (DIVSRC_LIST_DIVTOL+1)

//====================================================================================================
// 搜索列表上传     BF0B
//====================================================================================================
#define SYSSET_IPSET_SENDSTATE              (POL_DAT_BASE)  //1
#define SYSSET_IPSET_DESMAC                 (SYSSET_IPSET_SENDSTATE+1) //6
#define SYSSET_IPSET_CONFIG_F               (SYSSET_IPSET_DESMAC+6)    //1
#define SYSSET_IPSET_HOSTIP                 (SYSSET_IPSET_CONFIG_F+1)  //4
#define SYSSET_IPSET_IP_MODE                (SYSSET_IPSET_HOSTIP+4)    //1
#define SYSSET_IPSET_IP                     (SYSSET_IPSET_IP_MODE+1)   //4
#define SYSSET_IPSET_MASK                   (SYSSET_IPSET_IP+4)   //4
#define SYSSET_IPSET_GATE                   (SYSSET_IPSET_MASK+4)   //4


#endif //__PROTOCOL_ADRBASE_H

