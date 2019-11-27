#include "string.h"

#include "audio_buffmanage.h"
#include "eth_audio_config.h"
#include "gobal_val.h"
#include "eth_audio.h"
#include "adpcm.h"
#include "debug_print.h"
#include "sys_log.h" 

#include "src_fifo.h"

#include "audio_tx.h"

extern unsigned reboot_inc;
/*
static void debug_audio_devlist(eth_audio_dev_t audio_devlist[MAX_SENDCHAN_NUM], int num)
{
    //debug_printf("\ndebug_audio_devlist\n");
    for(int i=0; i<num; i++) {
        if(audio_devlist[i].channel_num == 0) continue;
        debug_printf("[%d] IP:%d.%d.%d.%d  num:%d  media_list:", 
        i, audio_devlist[i].ip[0], audio_devlist[i].ip[1], audio_devlist[i].ip[2], audio_devlist[i].ip[3],audio_devlist[i].channel_num);
        for(int j=0; j<audio_devlist[i].channel_num; j++)
            debug_printf("[%d|%d]", audio_devlist[i].media_list[j].channel, audio_devlist[i].media_list[j].priority);
        debug_printf("\n\n");
    }
}*/
static void inset_media_list(media_info_t media_list[NUM_MEDIA_INPUTS], uint8_t &channel_num, media_info_t info)
{
    uint8_t i, j;

    // 首个通道与插入值相同, 更新并退出
    if(media_list[0].channel && media_list[0].channel == info.channel) {
        media_list[0] = info;
        return;
    }
    // 清空相同通道的标志位 
    for(i=0; i<channel_num; i++) {
        if(media_list[i].channel == info.channel) {
            memmove(&media_list[i], &media_list[i+1], (channel_num-(i+1))*sizeof(media_info_t));
            channel_num--;
            break;// 确保后面没有相同通道
        }
    }
    // 插值复制
    for(i=0; i<channel_num; i++) {
        if(media_list[i].priority <= info.priority) {
            for((NUM_MEDIA_INPUTS==channel_num)?(j=channel_num-1):(j=channel_num); j>i; j--) 
                media_list[j] = media_list[j-1];
            media_list[i] = info;
            channel_num++;
            break;
        }
    }
    if(i==channel_num){
        media_list[i] = info;
        channel_num++;
    }
}

static void remove_media_list(media_info_t media_list[NUM_MEDIA_INPUTS], uint8_t &channel_num, uint8_t ch)
{
    // 清空相同通道的标志位 
    for(uint8_t i=0; i<channel_num; i++) {
        if(media_list[i].channel == ch) {
            memmove(&media_list[i], &media_list[i+1], (channel_num-(i+1))*sizeof(media_info_t));
            channel_num--;
            break;
        }
    }
}
//--------------------------------------------------------------------------------

void audio_buffmanage_process(client ethernet_cfg_if i_eth_cfg,
						 			 server ethaud_cfg_if i_ethaud_cfg[n_ethaud_cfg],
						  			 static const unsigned n_ethaud_cfg){
	//-----------------------------------------------------------------------
	// global val point get 
	//-----------------------------------------------------------------------
	volatile g_val_t *unsafe g_t_val;
	uint8_t i, j;
	
	// Reset global val point
	unsafe{
	uint32_t tmp;
	tmp = get_gval_point();
	g_t_val = (g_val_t *)tmp;
	//
	//g_t_val->audio_format= (SAMPLE_RATE_48K<<4)|AUDIOWIDTH_ADPCM;
	//g_t_val->audio_format= (SAMPLE_RATE_48K<<4)|AUDIOWIDTH_16BIT;
	g_t_val->audio_format= (SAMPLE_RATE_48K<<4)|AUDIOWIDTH_MP3;
	
	for(i=0; i<NUM_MEDIA_INPUTS; i++)
		g_t_val->audio_txvol[i] = MIX_VOL;

	//g_t_val->tx_flag = 0;
	//tx_flag = &g_t_val->tx_flag;
	}//unsafe
    timer systime;
    unsigned time_tmp;
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// main loop
	//-------------------------------------------------------------------------
	unsafe{
	while(1){
		select{
			//======================================================================================================
			case i_ethaud_cfg[uint8_t a].set_audio_ethinfo(audio_ethinfo_t *t_audio_ethinfo):
				memcpy(g_t_val->macaddress,t_audio_ethinfo->macaddress,6);	
				memcpy(g_t_val->ipgate_macaddr,t_audio_ethinfo->ipgate_macaddr,6);
				memcpy(g_t_val->ipaddr,t_audio_ethinfo->ipaddr,4);
				memcpy(g_t_val->ipmask,t_audio_ethinfo->ipmask,4);
				memcpy(g_t_val->ipgate,t_audio_ethinfo->ipgate,4);
				//---------------------------------------------------------------------------------
				// Set Local Mac Address
				// i_eth_cfg.set_macaddr(0,g_t_val->macaddress);
				//---------------------------------------------------------------------------------
				// Add Eth Eth MAC Filter
				#if ENABLE_AUD_TRAINSMIT
				ethernet_macaddr_filter_t macaddr_filter;
                memcpy(macaddr_filter.addr,g_t_val->macaddress,6);
                i_eth_cfg.add_macaddr_filter(0, 1, macaddr_filter);
                #endif
#if 0
				{
				ethernet_macaddr_filter_t macaddr_filter;
				// Add Local MAC Address
				for(i=0;i<6;i++)
					macaddr_filter.addr[i]=g_t_val->macaddress[i];
				 i_eth_cfg.add_macaddr_filter(0, 0, macaddr_filter);
				}
#endif
				//---------------------------------------------------------------------------------		
				// Reset gobal val
				g_t_val->standby=1;
				break;
			case i_ethaud_cfg[uint8_t a].set_audio_desip_infolist(audio_txlist_t *t_audio_txlist,uint8_t ch,uint8_t priority):
                timer tmr;
                uint32_t t1, t2;
                //uint8_t priority = 0;
                media_info_t info;
                int audio_devlist_free_index = -1;
                
                if(ch >= NUM_MEDIA_INPUTS) break;
                if(t_audio_txlist->num_info>=MAX_SENDCHAN_NUM) break;
                
                //tmr :> t1;
                //eth_audio_dev_t dev;
                for(i=0; i<t_audio_txlist->num_info; i++) { // 轮训设备
                    audio_devlist_free_index = -1;
                    for(j=0; j<MAX_SENDCHAN_NUM; j++) {     // 轮训音频设备列表
                        // 记录第一个空闲设备
                        if(g_t_val->audio_devlist[j].channel_num==0 && audio_devlist_free_index==-1) {
                            audio_devlist_free_index = j;
                        }
                        // 查找相同设备MAC地址的设备
                        if(memcmp(g_t_val->audio_devlist[j].dev_mac, t_audio_txlist->t_des_info[i].mac, 6)==0) {
                            // 复制IP地址
                            memcpy(g_t_val->audio_devlist[j].ip, t_audio_txlist->t_des_info[i].ip, 4);
                            // 复制发送MAC地址
                            if(!(ipaddr_maskcmp(g_t_val->ipaddr,g_t_val->audio_devlist[j].ip,g_t_val->ipmask)))
                                memcpy(g_t_val->audio_devlist[j].mac, g_t_val->ipgate_macaddr, 6);
                            else
                                memcpy(g_t_val->audio_devlist[j].mac, t_audio_txlist->t_des_info[i].mac, 6);

                            // 复制解码通道信息
                            info.channel = ch+1;
                            info.area_contorl = t_audio_txlist->t_des_info[i].area_contorl;
                            info.priority = priority;
                            
                            // 更新通道信息: 清空相同通道的标志位 优先级判断 插值后移 
                            inset_media_list(g_t_val->audio_devlist[j].media_list, g_t_val->audio_devlist[j].channel_num, info);
                            break;
                        }
                    }
                    // 没有找到相同设备MAC地址的设备, 并且有空闲设备, 进行复制
                    if(j==MAX_SENDCHAN_NUM && audio_devlist_free_index!=-1) {
                        j = audio_devlist_free_index;
                        // 复制设备MAC地址
                        memcpy(g_t_val->audio_devlist[j].dev_mac, t_audio_txlist->t_des_info[i].mac, 6);
                        
                        // 复制IP地址
                        memcpy(g_t_val->audio_devlist[j].ip, t_audio_txlist->t_des_info[i].ip, 4);
                        
                        // 复制发送MAC地址
                        if(!(ipaddr_maskcmp(g_t_val->ipaddr,g_t_val->audio_devlist[j].ip,g_t_val->ipmask)))
                            memcpy(g_t_val->audio_devlist[j].mac, g_t_val->ipgate_macaddr, 6);
                        else
                            memcpy(g_t_val->audio_devlist[j].mac, t_audio_txlist->t_des_info[i].mac, 6);
                        
                        // 复制解码通道信息
                        g_t_val->audio_devlist[j].media_list[0].channel = ch+1;
                        g_t_val->audio_devlist[j].media_list[0].area_contorl = t_audio_txlist->t_des_info[i].area_contorl;
                        g_t_val->audio_devlist[j].media_list[0].priority = priority;
                        g_t_val->audio_devlist[j].channel_num++;
                    }
                }
                //tmr :> t2;
                //debug_audio_devlist(g_t_val->audio_devlist, MAX_SENDCHAN_NUM);
                //debug_printf("audio_devlist tick %d\n", t2-t1);
                g_t_val->sample_rate[ch] = 44100;
				break;

            case i_ethaud_cfg[uint8_t a].update_audio_desip_info(uint8_t dev_mac[6], uint8_t ip[4]):
            {
#if NEW_SEND_LIST_MODE_ENABLE
                uint8_t tmp_mac[6];
                memcpy(tmp_mac, dev_mac, 6);
                for(j=0; j<MAX_SENDCHAN_NUM; j++) {
                    if(g_t_val->audio_devlist[j].channel_num && 
                       memcmp(tmp_mac, g_t_val->audio_devlist[j].dev_mac, 6)==0) {
                        // 更新设备IP
                        memcpy(g_t_val->audio_devlist[j].ip, ip, 4);
                        // 复制发送MAC地址
                        if(!(ipaddr_maskcmp(g_t_val->ipaddr,g_t_val->audio_devlist[j].ip,g_t_val->ipmask)))
                            memcpy(g_t_val->audio_devlist[j].mac, g_t_val->ipgate_macaddr, 6);
                        else
                            memcpy(g_t_val->audio_devlist[j].mac, tmp_mac, 6);                        
                        break;
                    }
                }
#endif                   
                break;
            }
			case i_ethaud_cfg[uint8_t a].set_audio_type(enum AUDIO_TYPE_E audio_type[NUM_MEDIA_INPUTS]):
				for(i=0; i<NUM_MEDIA_INPUTS; i++)
					g_t_val->audio_type[i] = audio_type[i];
				break;
			case i_ethaud_cfg[uint8_t a].set_audio_txen(uint8_t audio_txen[NUM_MEDIA_INPUTS],unsigned timestamp[NUM_MEDIA_INPUTS]):
                for(i=0;i<NUM_MEDIA_INPUTS;i++){
#if NEW_SEND_LIST_MODE_ENABLE
                    if(g_t_val->audio_txen[i] && !audio_txen[i]) {
                        for(j=0; j<MAX_SENDCHAN_NUM; j++) {
                            if(g_t_val->audio_devlist[j].channel_num) {
                                // 更新通道信息:清空相同通道的标志位 前移 
                                remove_media_list(g_t_val->audio_devlist[j].media_list, g_t_val->audio_devlist[j].channel_num, i+1);
                                // 清空数据
                                if(g_t_val->audio_devlist[j].channel_num == 0) {
                                    memset(&g_t_val->audio_devlist[j], 0, sizeof(eth_audio_dev_t));
                                }
                            }
                        }
                    }
#endif                    
                    g_t_val->aux_timestamp[i] = timestamp[i];
                    if(g_t_val->audio_txen[i] != audio_txen[i]){
                        g_t_val->audio_txen[i] = audio_txen[i];
                        if(g_t_val->audio_txen[i])
                            g_t_val->play_state[i] = 1;
                    }
                }
				break;
			case i_ethaud_cfg[uint8_t a].set_audio_txvol(uint8_t audio_val[NUM_MEDIA_INPUTS]):
				//memcpy(g_t_val->audio_txvol,audio_val,NUM_MEDIA_INPUTS);
				for(i=0; i<NUM_MEDIA_INPUTS; i++){
					g_t_val->audio_txvol[i] = audio_val[i];
                }
				break;
			case i_ethaud_cfg[uint8_t a].set_audio_priolv(enum AUDIO_PRIOLV_E audio_priolv[NUM_MEDIA_INPUTS]):
				break;
			case i_ethaud_cfg[uint8_t a].set_audio_silentlv(uint8_t audio_silentlv[NUM_MEDIA_INPUTS]):
				break;
            /*
			case i_ethaud_cfg[uint8_t a].chk_txpage_cnt(unsigned &txch_cnt):
				txch_cnt = g_t_val->audio_tx_cnt;
				g_t_val->audio_tx_cnt=0;
				break;
		    */
            case i_ethaud_cfg[uint8_t a].audio_play_stateset(uint8_t state,uint8_t ch):
                g_t_val->play_state[ch] = state;
                break;

            case systime when timerafter(time_tmp+100000000):> time_tmp:	//1hz process
                break;
#if 0
            case i_ethaud_cfg[unsigned a].send_text_en(uint8_t audio_txen[NUM_MEDIA_INPUTS],unsigned timestamp[NUM_MEDIA_INPUTS],
                                                                          unsigned max_send_page[NUM_MEDIA_INPUTS],
                                                                          unsigned have_send_num[NUM_MEDIA_INPUTS]):
                
                for(uint8_t i=0;i<NUM_MEDIA_INPUTS;i++){
                    g_t_val->audio_txen[i] = audio_txen[i];
                    g_t_val->send_text_en[i] = audio_txen[i];
                    g_t_val->aux_timestamp[i] = timestamp[i];
                    g_t_val->max_send_page[i] = max_send_page[i];
                    have_send_num[i] = g_t_val->have_send_num[i];
                    g_t_val->have_send_num[i] = 0; 
                }
                break;
#endif
        }//select
	} //while
	}//unsafe
}//main


