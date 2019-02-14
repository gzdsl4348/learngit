#ifndef  __GOBAL_VAL_H
#define  __GOBAL_VAL_H

#include "eth_audio_config.h"
#include "eth_audio.h"

#include <stdint.h>

typedef struct g_val_t{
	uint8_t macaddress[6];
	uint8_t ipgate_macaddr[6];
	uint8_t ipaddr[4];
	uint8_t ipmask[4];
	uint8_t ipgate[4];
	audio_txlist_t t_audio_txlist[NUM_MEDIA_INPUTS];	//2byte align
	uint8_t audio_format;
	uint8_t audio_type[NUM_MEDIA_INPUTS];
	uint8_t audio_txen;
	uint8_t audio_txvol[NUM_MEDIA_INPUTS];
    unsigned aux_timestamp[NUM_MEDIA_INPUTS];
	uint8_t tx_page_size;
	uint32_t silent_count;
	uint8_t	 silent_lv;
	unsigned tx_sendtime;
	uint32_t p_frame;
	unsigned tx_timer;
	uint8_t tx_flag;

    unsigned send_text_en[NUM_MEDIA_INPUTS];
    unsigned max_send_page[NUM_MEDIA_INPUTS];
    unsigned have_send_num[NUM_MEDIA_INPUTS];
    
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

