#ifndef  __FLASH_ADRBASE_H_
#define  __FLASH_ADRBASE_H_

#include "list_instance.h"
#include "protocol_adrbase.h"

//-------------------------------------------------------------------------
// USER FLASH  DATA BASE ADDRESS
#define FLASH_INIT_F        0x5AA57349

#define FLASH_ADR_BASE		0
#define FLASH_ADR_INIT      (FLASH_ADR_BASE)    //4
//---------------------------------------------------------------------------
// host info  主机信息位置
#define FLASH_HOST_INFO     (FLASH_ADR_INIT+4)
// 
#define FLASH_ADR_MAC		(FLASH_HOST_INFO)		//6
#define FLASH_ADR_NAME      (FLASH_ADR_MAC+6)       //22
#define FLASH_ADR_SN        (FLASH_ADR_NAME+DIV_NAME_NUM)     //6
#define FLASH_ADR_VERSION   (FLASH_ADR_SN+SYS_PASSWORD_NUM)        //2
#define FLASH_ADR_DIVTYPE   (FLASH_ADR_VERSION+2)   //15
#define FLASH_ADR_AUXTYPE   (FLASH_ADR_DIVTYPE+DIV_TYPE_NUM)   //1
#define FLASH_ADR_HOSTMODE  (FLASH_ADR_AUXTYPE+1)    //1
#define FLASH_ADR_SLIENT_EN (FLASH_ADR_HOSTMODE+1)   //1
#define FLASH_ADR_SLIENT_LV (FLASH_ADR_SLIENT_EN+1)  //1
#define FLASH_ADR_DHCPEN	(FLASH_ADR_SLIENT_LV+1)	 //1

#define FLASH_ADR_REGISER_S (FLASH_ADR_DHCPEN+1)  //1
#define FLASH_ADR_REGISER_D (FLASH_ADR_REGISER_S+1)  //2
//
#define FLASH_ADR_IPADR     (FLASH_ADR_REGISER_D+2)	//4
#define FLASH_ADR_IPMASK 	(FLASH_ADR_IPADR+4)	//4
#define FLASH_ADR_IPGATE 	(FLASH_ADR_IPMASK+4)	//4
// 
#define FLASH_HOSTINFO_END  (FLASH_ADR_IPGATE+4)
//------------------------------------------------------------------------
//
#define FLASH_SOLUSION_LIST (0)      //30*MAX_TASK_SOULTION
//
#define FLASH_ADR_TIMER_RESET  (FLASH_SOLUSION_LIST+60*MAX_TASK_SOULTION) //1
//
//===================================================================================
// 分区信息地址量
#define FLB_AREA_DAT_BASE   (0) //
#define FLB_AREA_SN         (FLB_AREA_DAT_BASE+1) //2 编号
#define FLB_AREA_NAME       (FLB_AREA_SN+2)     //22

//===================================================================================
// 账户信息地址量
#define FLB_ACCOUNT_DAT_BASE   (0) //

//==================================================================================
// 任务信息地址量
// 任务基本信息
#define FLB_TASK_DAT_BASE   (0)
//
#define FLB_RTTASK_DAT_BASE   (0)
#define FLB_RTTASK_LIST_BASE   (0)

//====================================================================================
// flash sector define
#define FLASH_DAT_SECTOR_BASE    200   //800K开始

#define USER_DAT_SECTOR         (FLASH_DAT_SECTOR_BASE)     //1 用户数据块
#define SOLUSION_DAT_SECTOR     (USER_DAT_SECTOR+1)
#define DIV_INFOLIST_SECTOR     (SOLUSION_DAT_SECTOR+1)           //5 设备信息块
#define AREA_INFOLIST_SECTOR    (DIV_INFOLIST_SECTOR+5)     //1 分区信息块
#define TIMED_TASK_SECTOR       (AREA_INFOLIST_SECTOR+1)     //MAX_TIMED_TASK_NUM+MAX_RING_TASK_NUM  定时任务块 
#define RT_TASK_SECTOR          (TIMED_TASK_SECTOR+MAX_TIMED_TASK_NUM+MAX_RING_TASK_NUM) //MAX_RT_TASK_NUM 即时任务块
#define RT_TASK_LIST_SECTOR     (RT_TASK_SECTOR+MAX_RT_TASK_NUM)     //1 即时任务块

#define ACCOUT_SECTOR           (RT_TASK_LIST_SECTOR+10)    // MAX_ACCOUNT_NUM 账户列表块

#define END_FL_SECTOR           (ACCOUT_SECTOR+MAX_ACCOUNT_NUM)        

#endif  //__FLASH_ADRBASE_H_

