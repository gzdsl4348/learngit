#ifndef  __GOBAL_VAL_H
#define  __GOBAL_VAL_H

#include "eth_audio_config.h"
#include "eth_audio.h"

#include <stdint.h>
/*
* 1.配置发送列表
* 2.使能通道发送
* 3.失能通道发送
*/
#define NEW_SEND_LIST_MODE_ENABLE 1
#define CHANNEL_FREE_FLAG 0

typedef struct
{
    uint8_t channel;//为原来通道值+1, 0为空值
    uint8_t area_contorl;
    uint8_t priority;
}media_info_t;

typedef struct
{
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t channel_num;// 如果为0, 可以清空mac、ip数据
    media_info_t media_list[NUM_MEDIA_INPUTS];
}eth_audio_dev_t;

typedef struct g_val_t{
	uint8_t macaddress[6];
	uint8_t ipgate_macaddr[6];
	uint8_t ipaddr[4];
	uint8_t ipmask[4];
	uint8_t ipgate[4];
#if NEW_SEND_LIST_MODE_ENABLE 
    eth_audio_dev_t audio_devlist[MAX_SENDCHAN_NUM];
    uint32_t align_reserved;
#else
	audio_txlist_t t_audio_txlist[NUM_MEDIA_INPUTS];	//2byte align
#endif
	uint8_t audio_format;
	uint8_t audio_type[NUM_MEDIA_INPUTS];
	uint8_t audio_txen[NUM_MEDIA_INPUTS];
	uint8_t audio_txvol[NUM_MEDIA_INPUTS];
    uint32_t aux_timestamp[NUM_MEDIA_INPUTS];
	uint32_t silent_count;
	uint8_t	 silent_lv;
#if 0
	uint8_t tx_page_size;
	unsigned tx_sendtime;
	uint32_t p_frame;
	unsigned tx_timer;
	uint8_t tx_flag;
#endif    
    uint32_t sample_rate[NUM_MEDIA_INPUTS];

#if 0
    unsigned send_text_en[NUM_MEDIA_INPUTS];
    unsigned max_send_page[NUM_MEDIA_INPUTS];
    unsigned have_send_num[NUM_MEDIA_INPUTS];
#endif
    
	uint8_t standby;
}g_val_t;

#define ipaddr_maskcmp(addr1,addr2,mask) \
					  (((((uint8_t *)addr1)[0] & ((uint8_t *)mask)[0])==(((uint8_t *)addr2)[0] & ((uint8_t *)mask)[0])) && \
					   ((((uint8_t *)addr1)[1] & ((uint8_t *)mask)[1])==(((uint8_t *)addr2)[1] & ((uint8_t *)mask)[1])) && \
					   ((((uint8_t *)addr1)[2] & ((uint8_t *)mask)[2])==(((uint8_t *)addr2)[2] & ((uint8_t *)mask)[2])) && \
					   ((((uint8_t *)addr1)[3] & ((uint8_t *)mask)[3])==(((uint8_t *)addr2)[3] & ((uint8_t *)mask)[3])) \
					  )

uint32_t get_gval_point();


#endif

