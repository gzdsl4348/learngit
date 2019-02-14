#include "string.h"

#include "audio_buffmanage.h"
#include "eth_audio_config.h"
#include "gobal_val.h"
#include "eth_audio.h"
#include "adpcm.h"
#include "debug_print.h"
#include "src.h"

#include "src_fifo.h"

#include "audio_tx.h"

//---------------------------------------------------------------------------------
//adpcm def
struct adpcm_state t_coder_adpcm[NUM_MEDIA_INPUTS]={0};
//--------------------------------------------------------------------------------
//SRC def
fs_code_t sample_rate_in = FS_CODE_48;
fs_code_t sample_rate_out = FS_CODE_48;

ssrc_state_t  t_ssrc_state[SRC_CH_TOLNUM];  
int			  ssrc_stack[SRC_CH_TOLNUM][SRC_CH_TOLNUM*SRC_SAMPLEIN_NUM*4];
ssrc_ctrl_t   t_ssrc_ctrl[SRC_CH_TOLNUM];

int in_srcbuff[SRC_SAMPLEIN_NUM*4];
int out_srcbuff[SRC_SAMPLEIN_NUM*4*2];

char fifo_buff[NUM_MEDIA_INPUTS][512];

static struct
{
    uint8_t flag;
    uint16_t dst_ip[4];
    uint16_t dst_mask[4];
    uint8_t dst_mac[6];
}g_static_route;

//--------------------------------------------------------------------------------

void audio_buffmanage_process(       client music_decoder_output_if if_mdo,
                         			 client ethernet_cfg_if i_eth_cfg, int is_hp,
						 			 server ethaud_cfg_if i_ethaud_cfg[n_ethaud_cfg],
						  			 static const unsigned n_ethaud_cfg){
	//-----------------------------------------------------------------------
	// global val point get 
	//-----------------------------------------------------------------------
	volatile g_val_t *unsafe g_t_val;
	//uint8_t volatile *unsafe tx_flag;
	
	// Reset global val point
	unsafe{
	uint32_t tmp;
	tmp = get_gval_point();
	g_t_val = (g_val_t *)tmp;
	//
	//g_t_val->audio_format= (SAMPLE_RATE_48K<<4)|AUDIOWIDTH_ADPCM;
	g_t_val->audio_format= (SAMPLE_RATE_48K<<4)|AUDIOWIDTH_16BIT;
	g_t_val->tx_page_size = ETH_AUDIO_PAGE_4MS;
	g_t_val->tx_sendtime = 100000*ETH_AUDIO_PAGE/ETH_AUDIO_PAGE_4MS;
	
	for(uint8_t i=0; i<NUM_MEDIA_INPUTS; i++)
		g_t_val->audio_txvol[i] = MIX_VOL;

	g_t_val->tx_flag = 0;
	//tx_flag = &g_t_val->tx_flag;
	}//unsafe
	//------------------------------------------------------------------------
	// BUFF init
	mp3_buf_t t_mp3_buf[NUM_MEDIA_INPUTS]={0};
	int16_t *unsafe p_mp3buf;
	//------------------------------------
	audio_tx_frame_t *unsafe t_tx_frame;
	//-------------------------------------------------------------------------
	// SRC Init
	uint32_t ssrc_len=SRC_SAMPLEIN_NUM;
	unsigned srcbuf_inc=0;
	//
	unsafe{
	for(uint8_t i=0;i<SRC_CH_TOLNUM;i++){
		t_ssrc_ctrl[i].psState = &t_ssrc_state[i];
		t_ssrc_ctrl[i].piStack = ssrc_stack[i];
	}
	sample_rate_in = FS_CODE_44;
	sample_rate_out = FS_CODE_48;	
	}//unsafe
	for(uint8_t i=0;i<NUM_MEDIA_INPUTS;i++){
    	ssrc_init(sample_rate_in, sample_rate_out, 
	         	  &t_ssrc_ctrl[i], 1,
            	  SRC_SAMPLEIN_NUM, OFF);
   }
	//-------------------------------------------------------------------------
	//FIFO Init
	kfifo_t t_kfifo[NUM_MEDIA_INPUTS];
	unsafe{
    for(uint8_t i=0;i<NUM_MEDIA_INPUTS;i++){
    	t_kfifo[i].buffer= fifo_buff[i];
    	t_kfifo[i].size = 512;
    	t_kfifo[i].rptr=0;
    	t_kfifo[i].wptr=0;
    }
    }//unsafe
	// Add Eth TYPE Filter
	if(!is_hp) i_eth_cfg.add_ethertype_filter(0, 0x0800);   	//ETH Fun
	//TIME				
	unsigned time_tmp;
	timer sys_timer; 
	sys_timer :> time_tmp;
	//-------------------------------------------------------------------------
	debug_printf("audio begin \n");
     memset(&g_static_route, 0, sizeof(g_static_route));
	//-------------------------------------------------------------------------
	// main loop
	//-------------------------------------------------------------------------
	uint8_t i;
	unsafe{
	while(1){
		select{
			//======================================================================================================
			case i_ethaud_cfg[unsigned a].set_audio_ethinfo(audio_ethinfo_t *t_audio_ethinfo):
				memcpy(g_t_val->macaddress,t_audio_ethinfo->macaddress,6);	
				memcpy(g_t_val->ipgate_macaddr,t_audio_ethinfo->ipgate_macaddr,6);
				memcpy(g_t_val->ipaddr,t_audio_ethinfo->ipaddr,4);
				memcpy(g_t_val->ipmask,t_audio_ethinfo->ipmask,4);
				memcpy(g_t_val->ipgate,t_audio_ethinfo->ipgate,4);
				// Add Eth Eth MAC Filter
				{
				ethernet_macaddr_filter_t macaddr_filter;
				// Add Local MAC Address
				for(i=0;i<6;i++)
					macaddr_filter.addr[i]=g_t_val->macaddress[i];
				// i_eth_cfg.add_macaddr_filter(0, is_hp, macaddr_filter);
				}
				//---------------------------------------------------------------------------------		
				// Reset gobal val
				g_t_val->standby=1;
				break;
			case i_ethaud_cfg[unsigned a].set_audio_desip_infolist(audio_txlist_t *t_audio_txlist,uint8_t ch):
				memcpy(&g_t_val->t_audio_txlist[ch],t_audio_txlist,sizeof(audio_txlist_t));	
                for(uint8_t i=0;i<g_t_val->t_audio_txlist[ch].num_info; i++){
                    if(g_static_route.flag&ipaddr_maskcmp(g_t_val->ipaddr, g_static_route.dst_ip, g_static_route.dst_mask)) {
                        /* Build an ethernet header. */
                        memcpy(g_t_val->t_audio_txlist[ch].t_des_info[i].mac, g_static_route.dst_mac, 6);
                    } else if(!(ipaddr_maskcmp(g_t_val->ipaddr,g_t_val->t_audio_txlist[ch].t_des_info[i].ip,g_t_val->ipmask)))
                        memcpy(g_t_val->t_audio_txlist[ch].t_des_info[i].mac,g_t_val->ipgate_macaddr,6);
                }
				if(g_t_val->t_audio_txlist[ch].num_info<22){
					g_t_val->tx_page_size = ETH_AUDIO_PAGE_1MS;                    
                }
				else{
					g_t_val->tx_page_size = ETH_AUDIO_PAGE_4MS;
                }
                g_t_val->tx_sendtime = 100000*(g_t_val->tx_page_size/ETH_AUDIO_PAGE_1MS);
				break;
			case i_ethaud_cfg[unsigned a].set_audio_type(enum AUDIO_TYPE_E audio_type[NUM_MEDIA_INPUTS]):
				for(uint8_t i=0; i<NUM_MEDIA_INPUTS; i++)
					g_t_val->audio_type[i] = audio_type[i];
				break;
			case i_ethaud_cfg[unsigned a].set_audio_txen(uint8_t audio_txen,unsigned timestamp[NUM_MEDIA_INPUTS]):
				g_t_val->audio_txen = audio_txen;
                for(uint8_t i=0;i<NUM_MEDIA_INPUTS;i++){
                    g_t_val->aux_timestamp[i] = timestamp[i];
                }
				debug_printf("EN %x\n",g_t_val->audio_txen);
				break;
			case i_ethaud_cfg[unsigned a].set_audio_txvol(uint8_t audio_val[NUM_MEDIA_INPUTS]):
				//memcpy(g_t_val->audio_txvol,audio_val,NUM_MEDIA_INPUTS);
				for(i=0; i<NUM_MEDIA_INPUTS; i++)
					g_t_val->audio_txvol[i] = audio_val[i];
				break;
			case i_ethaud_cfg[unsigned a].set_audio_priolv(enum AUDIO_PRIOLV_E audio_priolv[NUM_MEDIA_INPUTS]):
				break;
			case i_ethaud_cfg[unsigned a].set_audio_silentlv(uint8_t audio_silentlv[NUM_MEDIA_INPUTS]):
				break;
            case i_ethaud_cfg[unsigned a].set_static_route(uint8_t dst_ip[], uint8_t dst_mask[], uint8_t dst_mac[]):
                memcpy(g_static_route.dst_ip, dst_ip, 4);
                memcpy(g_static_route.dst_mask, dst_mask, 4);
                memcpy(g_static_route.dst_mac, dst_mac, 6);
                g_static_route.flag = 1;
                break;
            case i_ethaud_cfg[unsigned a].send_text_en(uint8_t audio_txen,unsigned timestamp[NUM_MEDIA_INPUTS],
                                                                          unsigned max_send_page[NUM_MEDIA_INPUTS],
                                                                          unsigned have_send_num[NUM_MEDIA_INPUTS]):
                g_t_val->audio_txen = audio_txen;
                for(uint8_t i=0;i<NUM_MEDIA_INPUTS;i++){
                    g_t_val->send_text_en[i] = ((audio_txen>>i)&0x01);
                    g_t_val->aux_timestamp[i] = timestamp[i];
                    g_t_val->max_send_page[i] = max_send_page[i];
                    have_send_num[i] = g_t_val->have_send_num[i];
                    g_t_val->have_send_num[i] = 0; 
                }
                break;
            // 音频发送
			default:
			    //uint8_t new_tx_flag[i]={0};
                if(!g_t_val->tx_flag)
                    break;
                g_t_val->tx_flag = 0;
				t_tx_frame = (audio_tx_frame_t *)g_t_val->p_frame;
                // 取数据传送到tx buff
                for(uint8_t ch=0;ch<NUM_MEDIA_INPUTS;ch++){
    				if( kfifo_get(&t_kfifo[ch],(char *)t_tx_frame->samples[ch],g_t_val->tx_page_size*2)==0){ //取16位数据
    					memset(t_tx_frame->samples[ch],0,g_t_val->tx_page_size*2);
                    }
                    //    if(ch==0)
                    //        debug_printf("s %d\n",t_tx_frame->samples[ch][0]);    
                }
                for(uint8_t ch=0;ch<NUM_MEDIA_INPUTS;ch++){   
                    //---------------------------------------------------------------
                    // 取一个tx page的数据
    				do{
                        //-----------------------------------------------------------------------------
                        // 从mp3 buff 里 提取数据 到采样率转换
    					for(i=0; i<SRC_SAMPLEIN_NUM; i++){
    						in_srcbuff[i] = ((uint16_t *)(t_mp3_buf[ch].data))[t_mp3_buf[ch].mp3buf_inc/2]<<16;
    						//in_srcbuff[i] = (t_mp3_buf[ch].data[t_mp3_buf[ch].mp3buf_inc+1]<<24)|(t_mp3_buf[ch].data[t_mp3_buf[ch].mp3buf_inc]<<16);
    						t_mp3_buf[ch].mp3buf_inc+=2;	
    					}
    					ssrc_len = ssrc_process(in_srcbuff,out_srcbuff,&t_ssrc_ctrl[ch]);	//采样率转换
    					
    					//-----------------------------------------------------------------------------
    					int16_t buff_tmp[32];
    					if((g_t_val->audio_format&0x0F) == AUDIOWIDTH_ADPCM)	
    						for(i=0; i<ssrc_len; i++)
    							buff_tmp[i] = adpcm_coder(out_srcbuff[i]>>16,t_coder_adpcm[ch]);
    					else
    						for(i=0; i<ssrc_len; i++)
    							buff_tmp[i] = out_srcbuff[i]>>16;

    					kfifo_put(&t_kfifo[ch],(char *)buff_tmp,ssrc_len*2);    //存放数据
    					//-----------------------------------------------------------------------
    					if(t_mp3_buf[ch].mp3buf_inc+1 > t_mp3_buf[ch].len){  // mp3 buff 播放完毕
    	                	unsigned int samplerate;
    						t_mp3_buf[ch].mp3buf_inc = 0;
    						if_mdo.get_pcmbuff_active(ch,samplerate,t_mp3_buf[ch].data,2304,t_mp3_buf[ch].len);
    						if(t_mp3_buf[ch].len==0){
    							t_mp3_buf[ch].len=2304;
                                memset(t_mp3_buf[ch].data,0x00,t_mp3_buf[ch].len);
                            }

    					}
    				}
    				while((t_kfifo[ch].wptr-t_kfifo[ch].rptr) < g_t_val->tx_page_size*2);
                }// for ch
            break;     
        }//select
	} //while
	}//unsafe
}//main


