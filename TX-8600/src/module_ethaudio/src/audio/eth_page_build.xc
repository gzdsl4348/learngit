#include "eth_page_build.h"
#include "xclib.h"
#include "checksum.h"
#include "debug_print.h"

static uint16_t ip_sumtmp;
static uint16_t udp_sumtmp;

void aud_udpdata_init(uint8_t txbuff[],uint8_t mac_address[])
{
    unsafe{
	//=================================================================
    //Set Local MAC Address
    //-----------------------------------------------------------------
	for(uint8_t i=0;i<6;i++)
		txbuff[6+i] = mac_address[i];	
    //=================================================================
    //TCP/IP Internet Protocol
    //-----------------------------------------------------------------
    // Type = 0800 / Version = 0x4500
    ((uint32_t *)txbuff)[3]= 0x00450008;    //Buff 12,13,14,15
    txbuff[15] = 0x00;  //Bit
    txbuff[20] = 0x00;  //Flag Bit 4-7
    txbuff[21] = 0x00;  //Fragment Offset
    txbuff[22] = 128;   //Time Live
    txbuff[23] = 17;
    //=================================================================
    //UDP - Datagram Protocol
    //-----------------------------------------------------------------
    //Set Audio Source Port
    txbuff[34] = ETH_AUDIO_PORT>>8;
    txbuff[35] = ETH_AUDIO_PORT&0x00FF;
    //Set Audio Destinatiao Port
    txbuff[36] = ETH_AUDIO_PORT>>8;
    txbuff[37] = ETH_AUDIO_PORT&0x00FF;
    //-----------------------------------------------------------------
    }
}


//===========================================================================
//Audio Package Build
//===========================================================================
uint16_t audio_page_build(uint8_t txbuff[],
                              audio_tx_frame_t *unsafe t_audio_frame,
                              uint8_t ipaddress[],
                              uint8_t &audio_format,
                              uint8_t ch_enable_f,
                              uint8_t &audio_pagesize,
                              uint8_t audio_type[],
                              uint8_t &volume,
                              unsigned timestamp,
                              uint16_t &iptmp,
                              uint16_t &udptmp)
{
    //==========================================================================
    //--------------------------- RAM Define--------------------------
    uint16_t audiodata_len=0;
    uint16_t i;
    uint16_t len=0;
    uint16_t sum=0;
    uint8_t  total_num=0;
	uint8_t  ch_num=0;
    uint8_t  bitwidth;
	static uint16_t ident=0;
    //==========================================================================
    unsafe{
    //---------------------Set DesMac Adress------------------------
    for(i=0; i<6; i++)
        txbuff[i] = 0x00;
    //-------------------Set IP Adress-----------------------
    for(i=0; i<4 ; i++)
        txbuff[UDP_SOURCE_IP_ADR + i] = ipaddress[i];
    //------------------- Set Inden ---------------------------
    txbuff[UDP_IDENT_ADR] = (ident>>8);
    txbuff[UDP_IDENT_ADR+1] = ident;
    ident++;
    //==========================================================================
    //-----------------------Audio Data Build------------------------
    //==========================================================================
	//-------Audio Data Tpye 0xAAAA ----------------
    txbuff[AUDIO_TYPE_ADR]=0xAA;
    txbuff[AUDIO_TYPE_ADR+1]=0xAA;
    //音频时间戳组包
    txbuff[AUDIO_TIMESTAMP] = timestamp;
    txbuff[AUDIO_TIMESTAMP+1] = timestamp>>8;
    txbuff[AUDIO_TIMESTAMP+2] = timestamp>>16;
    txbuff[AUDIO_TIMESTAMP+3] = timestamp>>24;
    //音频功率分区
    txbuff[AUDIO_AREA_F] = 0x00;
    // 音频标志 保留
    txbuff[AUDIO_STATE_ADR] = 0x00;
    // 音频格式
    bitwidth = audio_format&0x0F;
    txbuff[AUDIO_FORMAT_ADR] = audio_format;
	// Get Audio Data Base Adr
	len = AUDIO_CHDATA_BASE_ADR;
	//
	for(i=0;i<NUM_MEDIA_INPUTS;i++){
		if(ch_enable_f&1){
			txbuff[len+AUDIO_AUXTYPE_ADR] = audio_type[i] ;           //音频类型
			txbuff[len+AUDIO_CHID_ADR] = i;				//音频通道ID
			txbuff[len+AUDIO_CHPRIO_ADR] = 100;				//音频优先级
			txbuff[len+AUDIO_CHVOL_ADR] = volume;		    //音频音量
			txbuff[len+AUDIO_SILENT_ADR] = 0;				//默音等级
            // 采样数据长度
			txbuff[len+AUDIO_DATALEN_ADR] = audio_pagesize>>8;	//H
			txbuff[len+AUDIO_DATALEN_ADR+1] = audio_pagesize;	//L
            len += AUDIO_DATABASE_ADR;
            //-------------------------------------------------------
            // ADPCM数据多3个字节
			if((txbuff[AUDIO_FORMAT_ADR]&0x0f)==AUDIOWIDTH_ADPCM){
				txbuff[len] = t_audio_frame->adpcm_sample[i]>>8;
				len++;
				txbuff[len] = t_audio_frame->adpcm_sample[i];
				len++;
				txbuff[len] = t_audio_frame->adpcm_index[i];
				len++;
			}	
		    //-----------------Get Chx Audio Data -------------------------------
            switch(bitwidth){
            	case AUDIOWIDTH_8BIT:
					break;
				case AUDIOWIDTH_16BIT:
					for(uint16_t count=0; count<audio_pagesize; count++){
						txbuff[len] = t_audio_frame->samples[i][count];
						txbuff[len+1] = t_audio_frame->samples[i][count]>>8; 
						len +=2;
					}
					break;
				case AUDIOWIDTH_24BIT:					
					break;
				case AUDIOWIDTH_32BIT:
					break;
				case AUDIOWIDTH_ADPCM:	
					for(uint16_t count=0; count<audio_pagesize; count+=2){
						txbuff[len] = ((uint8_t)t_audio_frame->samples[i][count]<<4);
                    	txbuff[len] |= (uint8_t)t_audio_frame->samples[i][count+1];;
						len ++;
					}
					break;
            }
			//-------------------------------------------------------------------
			total_num++;
		}    // if ch_enable_f&1
		ch_num++;
		ch_enable_f=ch_enable_f>>1;		
			
	}// for i<ETH_AUDIO_TX_NUM
	txbuff[AUDIO_CHTOTAL_ADR]=total_num;
    //=======================================================================
    //----------------------Get Package Lenth------------------------
    //=======================================================================
    //-----------------IP Header Length------------------------
    txbuff[UDP_IPHEADLEN_ADR]=(len-IP_HEADBASE_ADR)>>8;
    txbuff[UDP_IPHEADLEN_ADR+1]=(len-IP_HEADBASE_ADR);
    //-----------------UDP Header Length------------------------
    txbuff[UDP_HEADERLEN_ADR]=((len-UDP_HEADBASE_ADR)>>8);
    txbuff[UDP_HEADERLEN_ADR+1]=(len-UDP_HEADBASE_ADR);
    //-----------------Audio Data Length----------------------
    // None
    //=================================================================
    //-------------------CheckSum Calculate-----------------------
    // Init Sum Ram
    //
    txbuff[UDP_IPHEAD_SUM_ADR] = 0;                  // IP Check Sum
    txbuff[UDP_IPHEAD_SUM_ADR+1] = 0;                // IP Check Sum
    //
    txbuff[UDP_HEADERSUM_ADR] = 0;                   // IP CheckSum
    txbuff[UDP_HEADERSUM_ADR+1] = 0;                 // IP CheckSum
    //
    //-----------------------------------------------------------------
    //Calculata Audio Data CheckSum
    //None
    //----------------------------------------------------------------
    //Calculata IP Header CheckSum
    uint16_t len_tmp;
    len_tmp = IP_HEADER_LEN;
    sum = 0;
    //
    for(i=0; i<4; i++)
        txbuff[UDP_DES_IP_ADR + i] = 0x00;
    //----------------------------------------------------------------
    sum = chksum_16bit(sum,&txbuff[IP_HEADBASE_ADR],len_tmp);
    ip_sumtmp = ~sum;
    //-----------------------------------------------------------------------
    len_tmp = (txbuff[UDP_HEADERLEN_ADR]<<8)|txbuff[UDP_HEADERLEN_ADR+1];
    //
    sum = len_tmp+17;	// Add UDP Type To CheckSum
    sum = chksum_16bit(sum,&txbuff[UDP_SOURCE_IP_ADR],len_tmp+8);
    udp_sumtmp = ~sum;
    //----------------------------------------------------------------
    iptmp = ip_sumtmp;
    udptmp = udp_sumtmp;
    }//unsafe
    return (len);
}

void audpage_sum_build(uint8_t txbuff[],uint8_t des_ip[],uint8_t des_mac[],uint8_t area_contorl)
{
	uint16_t sum_tmp;
	for(uint8_t i=0;i<4;i++)
		txbuff[UDP_DES_IP_ADR+i] = des_ip[i];
	for(uint8_t i=0;i<6;i++)
		txbuff[i] = des_mac[i];
    //---------------------------------------------------------------
    txbuff[AUDIO_AREA_F] = area_contorl;
   	sum_tmp = ip_sumtmp;
    sum_tmp = ~(chksum_16bit(sum_tmp,&txbuff[AUDIO_AREA_F],1));
	//----------------------------------------------------------------
    //Calculata IP Header CheckSum
	sum_tmp = ip_sumtmp;
    sum_tmp = chksum_16bit(sum_tmp,&txbuff[UDP_DES_IP_ADR],4);
	//Get Sum
    txbuff[UDP_IPHEAD_SUM_ADR] = sum_tmp>>8;
    txbuff[UDP_IPHEAD_SUM_ADR+1] = sum_tmp;
	//----------------------------------------------------------------
    //Calculata UDP Header CheckSum
    sum_tmp = udp_sumtmp;
	sum_tmp = chksum_16bit(sum_tmp,&txbuff[UDP_DES_IP_ADR],4);
	//Get Sum
	txbuff[UDP_HEADERSUM_ADR] = sum_tmp>>8;
    txbuff[UDP_HEADERSUM_ADR+1] = sum_tmp;
	
}
							  




