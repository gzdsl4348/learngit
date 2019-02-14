#include <string.h>

#include "audio_tx.h"
#include "eth_audio_config.h"
#include "gobal_val.h"
#include "eth_page_build.h"

#include "debug_print.h"


double_txbuf_t t_txbuf={0};
uint8_t txbuff[1552];

on tile[1]: out port p_mclk = XS1_PORT_1A;

on tile[1]: out buffered port:8  p_48kclk = XS1_PORT_1G;

on tile[1]: clock mclk = XS1_CLKBLK_3;

void audio_tx(client ethernet_rx_if ? i_eth_rx_lp,
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
	//-----------------------------------------------------------------------------
	// time init
	unsigned time_tmp;
	unsigned sendtime;
    uint16_t iptmp,udptmp;
	timer sys_timer; 
	sys_timer :> time_tmp;
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
            case sys_timer when timerafter(time_tmp+g_t_val->tx_sendtime):> void:  
                time_tmp += g_t_val->tx_sendtime;
				//ip info build
				if( g_t_val->audio_txen == 0)
					break;
				//----------------------------------------------------------------
				// page build
				for(uint8_t ch=0;ch<NUM_MEDIA_INPUTS;ch++){
                    if(((g_t_val->audio_txen>>ch)&1)==0)//通道是否使能
                        continue;
        			uint16_t len;
                    //aud_udpdata_init(txbuff,g_t_val->macaddress);
    				len = audio_page_build(txbuff,&t_txbuf.t_frame,g_t_val->ipaddr,
						                 g_t_val->audio_format,
						                 (g_t_val->audio_txen&(1<<ch)),g_t_val->tx_page_size,
						                 g_t_val->audio_type,
						                 g_t_val->audio_txvol[ch],
						                 g_t_val->aux_timestamp[ch],
						                 iptmp,udptmp
						                 );
    				//---------------------------------------------------------
    				for(uint8_t i=0; i<g_t_val->t_audio_txlist[ch].num_info; i++){
						audpage_sum_build(txbuff,
						                  g_t_val->t_audio_txlist[ch].t_des_info[i].ip,
						                  g_t_val->t_audio_txlist[ch].t_des_info[i].mac,
						                  g_t_val->t_audio_txlist[ch].t_des_info[i].area_contorl);
                        if(!isnull(c_tx_hp))
                            ethernet_send_hp_packet(c_tx_hp,txbuff,len,ETHERNET_ALL_INTERFACES);
                        else if(!isnull(i_eth_tx_lp))
    					    i_eth_tx_lp.send_packet(txbuff,len,ETHERNET_ALL_INTERFACES); // Send Packet
					    
    				}// for end ip info build
                    //--------------------------------------------------------------------------------------------------
                    if(g_t_val->send_text_en[ch]){
                        g_t_val->have_send_num[ch]++;
                        if(g_t_val->have_send_num[ch]>=g_t_val->max_send_page[ch]){
                            g_t_val->send_text_en[ch]=0;
                            g_t_val->audio_txen ^= (1<<ch);
                            debug_printf("\neth audio send over ch%d: %d\n\n",ch,g_t_val->have_send_num[ch]);
                        }
                    } 
                    //-------------------------------------------------------------------------------------------------
                }
				//----------------------------------------------------------------------------
				//change buff
				//c_tx_flag <: g_t_val->tx_flag;
                g_t_val->tx_flag = 1;
                g_t_val->p_frame = (uint32_t)(&t_txbuf.t_frame);
				//------------------------------------------------------------------------------
				break;
		}//select
	}//while
	}//unsafe
}

