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

static mp3_frame_send_info_t g_mp3_frame_send_info[] = {{48000,2400000+5, 0}, //48K
                                                        {44100, 2612245+5, 0},//44K
                                                        {16000,3600000+5, 0}, //16K
                                                        //{8000,1800000+5, 0}, //8K
                                                        {88200,1160997+5,0},   //WAV 44.1K 传512字节 特殊处理
                                                        {96000,1066666+5,0},   //WAV 48K 传512字节 特殊处理
                                                        {31000,3200000+5,0}   //WAV 16K 传512字节 特殊处理
                                                       };

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
	//
	uint8_t sample_error_f[NUM_MEDIA_INPUTS];			
	memset(sample_error_f,NUM_MEDIA_INPUTS,0);
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
            case (!isnull(c_rx_hp))=> ethernet_receive_hp_packet(c_rx_hp, txbuff, packet_info):
                aud_udpdata_init(txbuff,g_t_val->macaddress);
                break;            
	        default:
	            sys_timer :> audio_timer_tick;
                for(uint8_t i=0; i<sizeof(g_mp3_frame_send_info)/sizeof(mp3_frame_send_info_t); i++)
                {
                    if(timeafter(g_mp3_frame_send_info[i].timer_tick+g_mp3_frame_send_info[i].per_frame_tick, audio_timer_tick)) continue;
                    g_mp3_frame_send_info[i].timer_tick += g_mp3_frame_send_info[i].per_frame_tick;
                    
					memset(sample_error_f,NUM_MEDIA_INPUTS,0);
                    for(uint8_t ch=0; ch<NUM_MEDIA_INPUTS; ch++)
                    {
                        static uint32_t cnt = 0;
                        uint32_t t1,t2,t3,t4,t5,t6;
                        
                        // 通道是否使能
                        if(g_t_val->audio_txen[ch]==0) continue;
                        // 通道采样率是否符合
                        if(g_t_val->sample_rate[ch] != g_mp3_frame_send_info[i].sample_rate){
							if((g_t_val->sample_rate[ch]==48000 || g_t_val->sample_rate[ch]==44100 || g_t_val->sample_rate[ch]==16000 
                                ||g_t_val->sample_rate[ch]==88200 ||g_t_val->sample_rate[ch]==96000 || g_t_val->sample_rate[ch]==31000)==0){
								//if(ch==0)
									debug_printf("sample error ch%d\n",ch);
								sample_error_f[ch]++;
								continue;
							}
							continue;
						}
						
                        //sys_timer :> t1;
						uint32_t num, sr, lent,file_typem,format,send_num;
                        //-------------------------------------------------------------------------------------------------------
                        // 多于4个发送下，强制发送ADPCM
                        format = 0;
                        send_num = 0;
                        /*
                        for(uint8_t i=0; i<MAX_SENDCHAN_NUM; i++){
                            if(g_t_val->audio_devlist[i].channel_num==0 || 
                               g_t_val->audio_devlist[i].media_list[0].channel!=(ch+1)) continue;
                            send_num++;
                            if(send_num>1)
                                format = 1;
                        }*/
                        //---------------------------------------------------------------------------------------------------------
                        uint32_t len;
                        uint8_t file_type;
                        if_mdo.get_mp3_frame(ch,txbuff+AUDIO_CHDATA_BASE_ADR+AUDIO_DATABASE_ADR,len,num,sr,file_type,format);
                        //sys_timer :> t2;
                        // 无数据
                        if(len==0) continue;
                        
                        // 切换采样率
                        if(sr != g_t_val->sample_rate[ch]) g_t_val->sample_rate[ch] = sr;

                        // mp3 或 wav格式
                        if(file_type)
                            g_t_val->audio_format = (SAMPLE_RATE_48K<<4)|AUDIOWIDTH_WAV;
                        else
                            g_t_val->audio_format = (SAMPLE_RATE_48K<<4)|AUDIOWIDTH_MP3;
                        
                        // 清buff标志位
                        if(num == 0) ;
                        
                        //sys_timer :> t2;
                        len = audio_page_build(txbuff,g_t_val->ipaddr,
                                                         g_t_val->audio_format,
                                                         g_t_val->audio_type[ch],
                                                         g_t_val->audio_txvol[ch],
                                                         g_t_val->aux_timestamp[ch],
                                                         iptmp,udptmp,
                                                         len,ch);
                        //sys_timer :> t3;
#if NEW_SEND_LIST_MODE_ENABLE
                        for(uint8_t i=0; i<MAX_SENDCHAN_NUM; i++){ //MAX_SENDCHAN_NUM
                            if(g_t_val->audio_devlist[i].channel_num==0 || 
                               g_t_val->audio_devlist[i].media_list[0].channel!=(ch+1)) continue;
                            //sys_timer :> t5;
                            audpage_sum_build(txbuff,
                                              g_t_val->audio_devlist[i].ip,
                                              g_t_val->audio_devlist[i].mac,
                                              g_t_val->audio_devlist[i].media_list[0].area_contorl);                          
                            //sys_timer :> t6;
                            //debug_printf("page build %d\n",t6-t5);
                            // Send Packet
                            g_t_val->audio_tx_cnt++;
                            ethernet_send_hp_packet(c_tx_hp,txbuff,len,ETHERNET_ALL_INTERFACES);
                        }// for end ip info build 
#else
                        for(uint8_t i=0; i<g_t_val->t_audio_txlist[ch].num_info; i++){
                            audpage_sum_build(txbuff,
                                              g_t_val->t_audio_txlist[ch].t_des_info[i].ip,
                                              g_t_val->t_audio_txlist[ch].t_des_info[i].mac,
                                              g_t_val->t_audio_txlist[ch].t_des_info[i].area_contorl);
                            // Send Packet
                            g_t_val->audio_tx_cnt++;
                            ethernet_send_hp_packet(c_tx_hp,txbuff,len,ETHERNET_ALL_INTERFACES);                   
                        }// for end ip info build
#endif
                        //sys_timer :> t4;
                        //if(cnt++ == 20000 || cnt == 30000)
                        //if(cnt++ > 500)
                        //{
                        //    debug_printf("mp3_frame_send[%d]:tol %d pagebuild %d get time %d\n", cnt, t4-t1,t3-t2,t2-t1);
                        //}
                    }
			        for(uint8_t ch=0; ch<NUM_MEDIA_INPUTS; ch++){
						if(sample_error_f[ch]>=2){
							uint32_t num, sr, len,format;
                            uint8_t file_type;
	                        if_mdo.get_mp3_frame(ch,txbuff+AUDIO_CHDATA_BASE_ADR+AUDIO_DATABASE_ADR,len,num,sr,file_type,format);
	                        //sys_timer :> t2;
	                        // 无数据
							sample_error_f[ch]=0;;
	                        if(len==0) continue;
                        
	                        // 切换采样率
	                        if(sr != g_t_val->sample_rate[ch]) g_t_val->sample_rate[ch] = sr;
							
						}
					}
                }
				break;
		}//select
	}//while
	}//unsafe
}

