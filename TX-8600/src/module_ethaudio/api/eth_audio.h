#ifndef  __ETH_AUDIO_H
#define  __ETH_AUDIO_H

#include <xs1.h>
#include <platform.h>
#include <stdint.h>

#include "eth_audio_config.h"

#include "file.h"

#include "ethernet.h"

//---------------------------------------------------------------------------
//set ip moudle ip config
//---------------------------------------------------------------------------
typedef struct audio_ethinfo_t{
	uint8_t macaddress[6];
	uint8_t ipgate_macaddr[6];
	uint8_t ipaddr[4];
	uint8_t ipmask[4];
	uint8_t ipgate[4];
}audio_ethinfo_t;

//====================================================================================================================
// AUDIO ETH SEND NEED 
//====================================================================================================================
#ifndef __AUDIO_TYPE_DEF_
#define __AUDIO_TYPE_DEF_
//---------------------------------------------------------------------------
//set audio send desip info 
//---------------------------------------------------------------------------
typedef struct des_info_t{
	uint8_t	ip[4];		//destination ip address
	uint8_t mac[6]; 	//destination macaddress
	uint8_t area_contorl;
}des_info_t;
//---------------------------------------------------------------------------
//set audio send desip info list
//---------------------------------------------------------------------------
typedef struct audio_txlist_t{
	des_info_t t_des_info[MAX_SENDCHAN_NUM]; // tx desip info
	uint8_t    num_info;					 //the num of tx info	
}audio_txlist_t;
#endif
//====================================================================================================================

#ifdef __XC__

#include "music_decoder_server.h"

//---------------------------------------------------------------------------
// audio config interface 
//---------------------------------------------------------------------------
typedef interface ethaud_cfg_if{
	//---------------------------------------------------------------------------
	// set audio moudle ip and mac config fun
	//---------------------------------------------------------------------------
	void set_audio_ethinfo(audio_ethinfo_t *t_audio_ethinfo);
	//
	//---------------------------------------------------------------------------
	// set audio destination ip and mac to send fun
	//---------------------------------------------------------------------------
	void set_audio_desip_infolist(audio_txlist_t *t_audio_txlist,uint8_t ch,uint8_t priority);
	//
	//---------------------------------------------------------------------------
	// set audio ch0-chx type
	//---------------------------------------------------------------------------
	void set_audio_type(enum AUDIO_TYPE_E     audio_type[NUM_MEDIA_INPUTS]);
	//
	//---------------------------------------------------------------------------
	// set audio tx ch_num enable or disable
	//---------------------------------------------------------------------------
	void set_audio_txen(uint8_t audio_txen[NUM_MEDIA_INPUTS],unsigned timestamp[NUM_MEDIA_INPUTS]);
	//	
	//---------------------------------------------------------------------------
	// set audio tx volume 	(config volume is 0-50)
	//---------------------------------------------------------------------------
	void set_audio_txvol(uint8_t audio_val[NUM_MEDIA_INPUTS]);	//0-100
	//
	//---------------------------------------------------------------------------
	// set audio tx prio level   
	//---------------------------------------------------------------------------
	void set_audio_priolv(enum AUDIO_PRIOLV_E audio_priolv[NUM_MEDIA_INPUTS]);	
	//
	//---------------------------------------------------------------------------
	// set audio tx silent level (silent level is 0-50)
	//---------------------------------------------------------------------------
	void set_audio_silentlv(uint8_t audio_silentlv[NUM_MEDIA_INPUTS]);		//0-50

    void update_audio_desip_info(uint8_t dev_mac[6], uint8_t ip[4]);

	void chk_txpage_cnt(unsigned &txch_cnt);
	//
#if 0
	void send_text_en(uint8_t audio_txen[NUM_MEDIA_INPUTS],unsigned timestamp[NUM_MEDIA_INPUTS],
	                                        unsigned max_send_page[NUM_MEDIA_INPUTS],
	                                        unsigned have_send_num[NUM_MEDIA_INPUTS]);
#endif
}ethaud_cfg_if;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void eth_audio(client ethernet_cfg_if i_eth_cfg,
                 streaming chanend c_tx_hp,
			     server ethaud_cfg_if i_ethaud_cfg[n_ethaud_cfg],
				 static const unsigned n_ethaud_cfg,
                 client music_decoder_output_if if_mdo);
#endif //__XC__

#endif // __ETH_AUDIO_H

