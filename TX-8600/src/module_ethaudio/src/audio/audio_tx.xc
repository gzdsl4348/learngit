#include <string.h>
#include "xassert.h"

#include "audio_tx.h"
#include "eth_audio_config.h"
#include "gobal_val.h"
#include "eth_page_build.h"

#include "debug_print.h"


uint8_t txbuff[1552];

on tile[1]: out port p_mclk = XS1_PORT_1A;

on tile[1]: out buffered port:8  p_48kclk = XS1_PORT_1G;

on tile[1]: clock mclk = XS1_CLKBLK_3;


typedef struct
{
    uint32_t sample_rate;
    uint32_t per_frame_tick;// 100MHZ 1152采样点
    uint32_t timer_tick;
}mp3_frame_send_info_t;

static mp3_frame_send_info_t g_mp3_frame_send_info[] = {{48000, 2400000+5, 0}, {44100, 2612245+5, 0} };

void audio_tx(  client music_decoder_output_if if_mdo,
                client ethernet_rx_if ? i_eth_rx_lp,
			    client ethernet_tx_if ? i_eth_tx_lp,
		        streaming chanend ? c_rx_hp,
                streaming chanend ? c_tx_hp)
{
    unsafe{
	// Eth Recive Packet Info
	ethernet_packet_info_t packet_info;
	//-----------------------------------------------------------------------------
	// gobal val init
	volatile g_val_t *unsafe g_t_val;
	unsafe{
	g_t_val = (g_val_t *)(get_gval_point());
	//tx_en = &g_t_val->audio_txen;
	while(g_t_val->standby!=1);		//wait for reset
	aud_udpdata_init(txbuff,g_t_val->macaddress);
    //*tx_en = 0;
    }
    if((isnull(i_eth_rx_lp)||isnull(i_eth_tx_lp))&&(isnull(c_rx_hp)||isnull(c_tx_hp))) {
        fail("Using high priority channels or low priority channels");
    }    
	//-----------------------------------------------------------------------------
	// time init
    uint16_t iptmp,udptmp;
	timer sys_timer; 
    unsigned audio_timer_tick;
	sys_timer :> audio_timer_tick;
    sys_timer :> g_mp3_frame_send_info[0].timer_tick;
    sys_timer :> g_mp3_frame_send_info[1].timer_tick;
	//-----------------------------------------------------------------------------
	//main loop
	while(1){
		select{
            [[independent_guard]]
			case (!isnull(i_eth_rx_lp))=> i_eth_rx_lp.packet_ready():
                i_eth_rx_lp.get_packet(packet_info,txbuff,1500);
                aud_udpdata_init(txbuff,g_t_val->macaddress);
				break;
            case (!isnull(c_rx_hp))=> ethernet_receive_hp_packet(c_rx_hp, txbuff, packet_info):
                aud_udpdata_init(txbuff,g_t_val->macaddress);
                break;            
	        default:
	            sys_timer :> audio_timer_tick;
                
                for(uint8_t i=0; i<sizeof(g_mp3_frame_send_info)/sizeof(mp3_frame_send_info_t); i++)
                {
                    if(timeafter(g_mp3_frame_send_info[i].timer_tick+g_mp3_frame_send_info[i].per_frame_tick, audio_timer_tick)) continue;
                    g_mp3_frame_send_info[i].timer_tick += g_mp3_frame_send_info[i].per_frame_tick;
                    
                    for(uint8_t ch=0; ch<NUM_MEDIA_INPUTS; ch++)
                    {
                        static uint32_t cnt = 0;
                        uint32_t t1,t2,t3,t4;
                        
                        // 通道是否使能
                        if(g_t_val->audio_txen[ch]==0) continue;
                        // 通道采样率是否符合
                        if(g_t_val->sample_rate[ch] != g_mp3_frame_send_info[i].sample_rate) continue;
                        sys_timer :> t1;
                        uint32_t num, sr, len;
                        if_mdo.get_mp3_frame(ch,txbuff+AUDIO_CHDATA_BASE_ADR+AUDIO_DATABASE_ADR,len,num,sr);
                        sys_timer :> t2;
                        // 无数据
                        if(len==0) continue;
                        
                        // 切换采样率
                        if(sr != g_t_val->sample_rate[ch]) g_t_val->sample_rate[ch] = sr;

                        // 清buff标志位
                        if(num == 0) ;
                        
                        len = audio_page_build(txbuff,g_t_val->ipaddr,
                                                         g_t_val->audio_format,
                                                         g_t_val->audio_type[ch],
                                                         g_t_val->audio_txvol[ch],
                                                         g_t_val->aux_timestamp[ch],
                                                         iptmp,udptmp,
                                                         len,ch);
                        sys_timer :> t3;
#if NEW_SEND_LIST_MODE_ENABLE
                        for(uint8_t i=0; i<MAX_SENDCHAN_NUM; i++){
                            if(g_t_val->audio_devlist[i].channel_num==0 || 
                               g_t_val->audio_devlist[i].media_list[0].channel!=(ch+1)) continue;
#if 0
                            static int send_cnt = 0;
                            if(send_cnt++ > 500) debug_printf("mp3_frame_send[%d]\n", send_cnt);
#endif
                            audpage_sum_build(txbuff,
                                              g_t_val->audio_devlist[i].ip,
                                              g_t_val->audio_devlist[i].mac,
                                              g_t_val->audio_devlist[i].media_list[0].area_contorl);
                            // Send Packet
                            g_t_val->audio_tx_cnt++;
                            if(!isnull(c_tx_hp))
                                ethernet_send_hp_packet(c_tx_hp,txbuff,len,ETHERNET_ALL_INTERFACES);
                            else if(!isnull(i_eth_tx_lp))
        					    i_eth_tx_lp.send_packet(txbuff,len,ETHERNET_ALL_INTERFACES);                         
                        }// for end ip info build          
#else
                        for(uint8_t i=0; i<g_t_val->t_audio_txlist[ch].num_info; i++){
                            audpage_sum_build(txbuff,
                                              g_t_val->t_audio_txlist[ch].t_des_info[i].ip,
                                              g_t_val->t_audio_txlist[ch].t_des_info[i].mac,
                                              g_t_val->t_audio_txlist[ch].t_des_info[i].area_contorl);
                            // Send Packet
                            g_t_val->audio_tx_cnt++;
                            if(!isnull(c_tx_hp))
                                ethernet_send_hp_packet(c_tx_hp,txbuff,len,ETHERNET_ALL_INTERFACES);
                            else if(!isnull(i_eth_tx_lp))
        					    i_eth_tx_lp.send_packet(txbuff,len,ETHERNET_ALL_INTERFACES);                         
                        }// for end ip info build
#endif
                        sys_timer :> t4;
                        if(cnt++ == 20000 || cnt == 30000)
                        //if(cnt++ > 500)
                        {
                            debug_printf("mp3_frame_send[%d]:%d %d %d\n", cnt, t2-t1, t3-t2, t4-t3);
                        }
                    }
                }
				break;
		}//select
	}//while
	}//unsafe
}

