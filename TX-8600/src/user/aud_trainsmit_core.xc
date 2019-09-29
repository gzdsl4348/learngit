#include <xs1.h>
#include <platform.h>
#include "aud_trainsmit_core.h"
#include "string.h"
#include "xtcp.h"
#include "eth_page_build.h"
#include "checksum.h"
#include "debug_print.h"

//static uint8_t audio_ch_state[MAX_APP_AUD_TRAINSMIT_NUM];   //寻呼通道状态
//static uint8_t audio_ch_timer[MAX_APP_AUD_TRAINSMIT_NUM];

//audts_divlist_s s_audts_divlist[MAX_APP_AUD_TRAINSMIT_NUM];

/*
xtcp_ipconfig_t host_ip;
uint8_t host_mac[6];
static char rxbuff[1500];
static uint16_t iptmp;
static uint16_t udptmp;
static uint16_t aud_ip_sumtmp;
static uint16_t aud_udp_sumtmp;

void aud_trainsmit_page_build(uint8_t ch){
    uint16_t ip_len,udp_len,i;
    //---------------------Set DesMac Adress------------------------
    for(i=0; i<6; i++)
        rxbuff[i] = 0x00;
    //-------------------Set IP Adress-----------------------
    for(i=0; i<4 ; i++)
        rxbuff[UDP_SOURCE_IP_ADR + i] = host_ip.ipaddr[i];
    memcpy(&rxbuff[6],host_mac,6);
    //音频时间戳组包
    rxbuff[AUDIO_TIMESTAMP] = s_audts_divlist[ch].timestamp;
    rxbuff[AUDIO_TIMESTAMP+1] = s_audts_divlist[ch].timestamp>>8;
    rxbuff[AUDIO_TIMESTAMP+2] = s_audts_divlist[ch].timestamp>>16;
    rxbuff[AUDIO_TIMESTAMP+3] = s_audts_divlist[ch].timestamp>>24;
    //音频功率分区
    rxbuff[AUDIO_AREA_F] = 0;
    //音频类型
	rxbuff[AUDIO_CHDATA_BASE_ADR+AUDIO_AUXTYPE_ADR] = s_audts_divlist[ch].prio; 
    //-----------------IP Header Length------------------------
    ip_len = (rxbuff[UDP_IPHEADLEN_ADR]<<8)|rxbuff[UDP_IPHEADLEN_ADR+1];
    //-----------------UDP Header Length------------------------
    udp_len = (rxbuff[UDP_HEADERLEN_ADR]<<8)|rxbuff[UDP_HEADERLEN_ADR+1];
    
    //=================================================================
    //-------------------CheckSum Calculate-----------------------
    uint16_t len_ip_tmp,len_udp_tmp;
    len_ip_tmp = IP_HEADER_LEN;
    len_udp_tmp = (rxbuff[UDP_HEADERLEN_ADR]<<8)|rxbuff[UDP_HEADERLEN_ADR+1];
    // Init Sum Ram
    //
    rxbuff[UDP_IPHEAD_SUM_ADR] = 0;                  // IP Check Sum
    rxbuff[UDP_IPHEAD_SUM_ADR+1] = 0;                // IP Check Sum
    //
    rxbuff[UDP_HEADERSUM_ADR] = 0;                   // IP CheckSum
    rxbuff[UDP_HEADERSUM_ADR+1] = 0;                 // IP CheckSum
    //
    //-----------------------------------------------------------------
    //Calculata Audio Data CheckSum
    //None
    //----------------------------------------------------------------
    //Calculata IP Header CheckSum

    uint16_t sum = 0;
    //
    for(i=0; i<4; i++)
        rxbuff[UDP_DES_IP_ADR + i] = 0x00;
    //----------------------------------------------------------------
    sum = chksum_16bit(sum,&rxbuff[IP_HEADBASE_ADR],len_ip_tmp);
    aud_ip_sumtmp = ~sum;
    //-----------------------------------------------------------------------
    //
    sum = len_udp_tmp+17;	// Add UDP Type To CheckSum
    sum = chksum_16bit(sum,&rxbuff[UDP_SOURCE_IP_ADR],len_udp_tmp+8);
    aud_udp_sumtmp = ~sum;
    //----------------------------------------------------------------
    iptmp = aud_ip_sumtmp;
    udptmp = aud_udp_sumtmp;
}

void aud_trainsmit_build(uint8_t txbuff[],uint8_t des_ip[],uint8_t des_mac[],uint8_t area_contorl)
{
	uint16_t sum_tmp;
	for(uint8_t i=0;i<4;i++)
		txbuff[UDP_DES_IP_ADR+i] = des_ip[i];
	for(uint8_t i=0;i<6;i++)
		txbuff[i] = des_mac[i];
    //---------------------------------------------------------------
    txbuff[AUDIO_AREA_F] = area_contorl;
   	sum_tmp = aud_udp_sumtmp;
    aud_udp_sumtmp = ~(chksum_16bit(sum_tmp,&txbuff[AUDIO_AREA_F],1));
	//----------------------------------------------------------------
    //Calculata IP Header CheckSum
	sum_tmp = aud_ip_sumtmp;
    sum_tmp = chksum_16bit(sum_tmp,&txbuff[UDP_DES_IP_ADR],4);
	//Get Sum
    txbuff[UDP_IPHEAD_SUM_ADR] = sum_tmp>>8;
    txbuff[UDP_IPHEAD_SUM_ADR+1] = sum_tmp;
	//----------------------------------------------------------------
    //Calculata UDP Header CheckSum
    sum_tmp = aud_udp_sumtmp;
	sum_tmp = chksum_16bit(sum_tmp,&txbuff[UDP_DES_IP_ADR],4);
	//Get Sum
	txbuff[UDP_HEADERSUM_ADR] = sum_tmp>>8;
    txbuff[UDP_HEADERSUM_ADR+1] = sum_tmp;
	
}
*/

/*
void aud_trainsmit_core(streaming chanend c_rx_hp,server aud_trainsmit_if if_aud_trainsmit,client ethernet_tx_if i_eth_tx){
	// Eth Recive Packet Info
	ethernet_packet_info_t packet_info;
    memset(audio_ch_state,0xFF,MAX_APP_AUD_TRAINSMIT_NUM);
    //text_debug("aud init\n");

    timer timer_cnt;
    unsigned timer_tmp;
    timer_cnt:>timer_tmp;

    while(1){
		select{        
            case if_aud_trainsmit.aud_ch_chk()->uint8_t state:
                state = 0xFF;
                for(uint8_t i=0;i<MAX_APP_AUD_TRAINSMIT_NUM;i++){
                    if(audio_ch_state[i]=0xFF){
                        state = i;
                        break;
                    }
                }
                break;
            case if_aud_trainsmit.divlist_set(audts_divlist_s audts_divlist,xtcp_ipconfig_t set_host_ip,uint8_t set_host_mac[6]):
                memcpy(&s_audts_divlist[audts_divlist.id],&audts_divlist,sizeof(audts_divlist_s));
                memcpy(&host_ip,&set_host_ip,sizeof(xtcp_ipconfig_t));
                memcpy(host_mac,set_host_mac,6);
                #if 0
                text_debug("set divlist %d\n",s_audts_divlist[audts_divlist.id].num);
                for(uint8_t i=0;i<s_audts_divlist[audts_divlist.id].num;i++){
                    text_debug("mac %x %x %x %x %x %x ip %d %d %d %d\n",s_audts_divlist[audts_divlist.id].divinfo[i].mac[0],
                        s_audts_divlist[audts_divlist.id].divinfo[i].mac[1],
                        s_audts_divlist[audts_divlist.id].divinfo[i].mac[2],
                        s_audts_divlist[audts_divlist.id].divinfo[i].mac[3],
                        s_audts_divlist[audts_divlist.id].divinfo[i].mac[4],
                        s_audts_divlist[audts_divlist.id].divinfo[i].mac[5],
                        s_audts_divlist[audts_divlist.id].divinfo[i].ip[0],
                        s_audts_divlist[audts_divlist.id].divinfo[i].ip[1],
                        s_audts_divlist[audts_divlist.id].divinfo[i].ip[2],
                        s_audts_divlist[audts_divlist.id].divinfo[i].ip[3]
                    );
                }
                #endif
                break;
            case ethernet_receive_hp_packet(c_rx_hp, rxbuff, packet_info):
                uint8_t id = rxbuff[AUDIO_CHDATA_BASE_ADR+AUDIO_AUXTYPE_ADR];
                if(id>=MAX_APP_AUD_TRAINSMIT_NUM)
                    break;
                //转发处理
                aud_trainsmit_page_build(id);
                for(uint8_t i=0;i<s_audts_divlist[id].num;i++){
                    
                    aud_trainsmit_build(rxbuff,s_audts_divlist[id].divinfo[i].ip,s_audts_divlist[id].divinfo[i].mac,s_audts_divlist[id].divinfo[i].area_info);
                    #if 0
                    text_debug("mac %x %x %x %x %x %x ip %d %d %d %d\n",
                        rxbuff[0],
                        rxbuff[1],
                        rxbuff[2],
                        rxbuff[3],
                        rxbuff[4],
                        rxbuff[5],
                        rxbuff[UDP_DES_IP_ADR+0],
                        rxbuff[UDP_DES_IP_ADR+1],
                        rxbuff[UDP_DES_IP_ADR+2],
                        rxbuff[UDP_DES_IP_ADR+3]
                    );
                    #endif
                    i_eth_tx.send_packet(rxbuff,packet_info.len,ETHERNET_ALL_INTERFACES);
                    //text_debug("aud send\n");
                }
                //重置超时判断
                audio_ch_timer[id]=0;
                break;             
            case timer_cnt when timerafter(timer_tmp+10000000):> timer_tmp: //10hz process
                for(uint8_t i=0;i<MAX_APP_AUD_TRAINSMIT_NUM;i++){
                    audio_ch_timer[i]++;
                    if(audio_ch_timer[i]>10){
                        audio_ch_state[i]=0;
                    }
                }
            
                break;
        }
    }
}
*/
   


