//============================================================================================================
// Copyright (c) 2017, For Heng Ltd, All rights reserved
//============================================================================================================
//
//=============================================================================================================
//System Val Define
//=============================================================================================================
//#define  _GLOBAL_VAL_            //GlobalVal Define In Main
//-------------------------------------------------------------------
//
//============================================================================================================
// Include  File
//=============================================================================================================
// Gobal Val And HardWare Def
#include <platform.h>
#include <xs1.h>

#define __GLOBAL_CLIENT_

#include "hardware_def.h"

#include "if_ch_def.h"

#include "ethernet.h"
#include "xtcp.h"

#include "global_val_def.h"

#include "xtcp_user.h"
#include <stdio.h>
#include "sys.h"
#include "string.h"

#include "file.h"
#include "music_decoder_server.h"

#include "eth_audio.h"
#include "eth_audio_config.h"

#include "uart.h"
#include "gpio.h"
#include "debug_print.h"

#include "flash_user.h"
#include <stdlib.h>
#include "sdram.h"

#include "aud_trainsmit_core.h"

#include "sdcard_host.h"

extern void music_decoder(STREAMING_CHANEND(c_sdram));

//--------------------------------------------------------------------------------------------------
// sdram io 
//--------------------------------------------------------------------------------------------------
on tile[0]:out buffered port:32   sdram_dq_ah                 = XS1_PORT_16B;
on tile[0]:out buffered port:32   sdram_cas                   = XS1_PORT_1K;
on tile[0]:out buffered port:32   sdram_ras                   = XS1_PORT_1I;
on tile[0]:out buffered port:8    sdram_we                    = XS1_PORT_1L;
on tile[0]:out port               sdram_clk                   = XS1_PORT_1J;
on tile[0]:clock                  sdram_cb                    = XS1_CLKBLK_5;

enum SDRAM_CLIENT_T {
	SDRAM_USER,
    SDRAM_FLASH,
    SDRAM_FILE_SYSTEM,
    SDRAM_MP3_DECODER,
	SDRAM_CLENT_TOTAL	
};
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// sdcard io
//-------------------------------------------------------------------------------------------------
on tile[0]:buffered out port:32 p_sdclk = XS1_PORT_1E;

on tile[0]:[[bidirectional]] buffered port:8 p_sdcmd = XS1_PORT_1F;
on tile[0]:[[bidirectional]] buffered port:32 p_sddata = XS1_PORT_4D;

//on tile[0]:in port p_sdcarddetect = XS1_PORT_1O;

on tile[0]:clock cb1 = XS1_CLKBLK_3;
on tile[0]:clock cb2 = XS1_CLKBLK_4;

//-------------------------------------------------------------------------------------
// wifi text
//on tile[0]: in port p_wifi_tx = XS1_PORT_1G;
//on tile[0]: in port p_wifi_rx = XS1_PORT_1H;
/*
void text(client uart_tx_buffered_if if_uart_tx,client uart_rx_if if_uart_rx){

    timer systime;
    unsigned time_tmp;
    systime :> time_tmp; 
    debug_printf("send\n");
    uint8_t p[3]="AT\n";
    //for(uint8_t i=0;i<3;i++)
    //    if_uart_tx.write(p[i]);
        debug_printf("send\n");
    select{
        case if_uart_rx.data_ready():
            uint8_t a;
            a = if_uart_rx.read();
            debug_printf("%c",a);
            break;
        case systime when timerafter(time_tmp+100000000):> time_tmp:	//10hz process
            debug_printf("send\n");
            for(uint8_t i=0;i<3;i++)
                if_uart_tx.write(p[i]);
            break;
    }
}
*/


/*----------------------------------------------------------------------------------------------------
*/
//=============================================================================================================
// The Main Code-Contrl [//(I2S LAN_Audio) //TCP //]
//=============================================================================================================
int main()
{
    //=======================================================================================================
    //Process Interface And Chan Def
    ETH_IF_CH_DEF       //eth
    XTCP_IF_CH_DEF      //tcp
    UART_IF_CH_DEF      //uart

    streaming chan c_tx_hp;
    streaming chan c_rx_hp;
    
    fl_manage_if if_fl_manage[1];
    
  	chan c_faction;
    file_server_if if_fs;
    music_decoder_output_if if_mdo;

    image_upgrade_if i_image;

	ethaud_cfg_if if_ethaud_cfg[1];

    sd_host_if sdif[1];
    //-------------------------------------
    //wifi uart
    //
    streaming chan c_sdram[SDRAM_CLENT_TOTAL];
    
    aud_trainsmit_if i_aud_trainsmit;
    //
    //======================================================================================================
    // Main Loop 16Core Process
    //=====================================================================================================
    par{    
        //--------------------------------------------------------------------------------------------------
        // flash process
        //--------------------------------------------------------------------------------------------------
        on tile[0]:flash_process(i_image,c_sdram[SDRAM_FLASH]);	
        
        on tile[0]:sdram_server(c_sdram, SDRAM_CLENT_TOTAL,
                                 sdram_dq_ah,
                                 sdram_cas,
                                 sdram_ras,
                                 sdram_we,
                                 sdram_clk,
                                 sdram_cb,
                                 2, 128, 16, 8, 12, 2, 64, 4096, 4); //Uses IS42S16400D 64Mb part supplied on SDRAM slice
        //6KB
        on tile[0]:user_flash_manage(if_fl_manage,1,c_sdram[SDRAM_USER]);
        //----------------------------------------------------------------------------------
        // 165KB
        on tile[0]:{
            [[combine]]par{
                music_decoder_server(if_mdo);
            }
        }
        on tile[0]: file_server(if_fs, c_faction);
        
        on tile[0]:
        {
            set_core_fast_mode_on();
            file_process(c_sdram[SDRAM_FILE_SYSTEM], c_faction,sdif[0]);
        }
        
        on tile[0]: [[distribute]] sd_host_native(sdif,1,p_sdclk,p_sdcmd,p_sddata,cb1,cb2);
        on tile[0]:
        {
            set_core_high_priority_on();
            music_decoder(c_sdram[SDRAM_MP3_DECODER]);
        }
        //---------------------------------------------------------------------------------
        //  55KB
		on tile[0]: xtcp_uip(i_xtcp_user,XTCP_CLENT_TOTAL,null,
   	      	        		  i_eth_cfg[ETH_XTCP_CFG],  	// eth cfg client 2
    		        		  i_eth_rx_lp[ETH_RX_XTCP_DATA],   // eth rx client 1
    	 	        		  i_eth_tx_lp[ETH_TX_XTCP_DATA],	// eth tx client 1
     			        	  i_smi,		  // smi
      			         	  0);             // smi phy addr
        //#if ENABLE_AUD_TRAINSMIT
        //on tile[0]:aud_trainsmit_core(c_rx_hp,i_aud_trainsmit,i_eth_tx_lp[ETH_TX_AUD_TRAINSMIT]);
        //#endif
        //--------------------------------------------------------------------------------------------------
        // user process
        //--------------------------------------------------------------------------------------------------
        // 204KB
        on tile[1]: 
        xtcp_uesr(i_xtcp_user[XTCP_USER],if_ethaud_cfg[0],if_fl_manage[0],if_fs,i_uart_tx[UART_USER],i_image,i_aud_trainsmit);
        // 29KB
        on tile[1]: eth_audio(i_eth_cfg[ETH_AUDIO_CFG],
                              c_tx_hp,
                              if_ethaud_cfg, 1,
                              if_mdo);
        //--------------------------------------------------------------------------------------------------
        // system process
        //--------------------------------------------------------------------------------------------------
        on tile[1]:
        {
            mii_ethernet_rt_mac(i_eth_cfg,ETH_CFGCLENT_TOTAL,
                             i_eth_rx_lp, ETH_RX_CLENT_TOTAL,
                             i_eth_tx_lp, ETH_TX_CLENT_TOTAL,
                             null, c_tx_hp,
                             p_rxclk, p_rxer, p_rxd, p_rxdv,
                             p_txclk, p_txen, p_txd,
                             eth_rxclk, eth_txclk,
                             RX_BUFSIZE_WORDS,
                             TX_BUFSIZE_WORDS,
                             ETHERNET_DISABLE_SHAPER);        
        }
        //
        //---------------------------------------------------------------------------------------------
        // uart process
        //---------------------------------------------------------------------------------------------
        on tile[1]: {
			[[combine]]par{
		    // UART TX MODULE	
		    uart_tx_buffered(i_uart_tx[UART_USER],null,800,115200,UART_PARITY_NONE,8,1,p_uart_tx,i_smi, p_smi_mdio, p_smi_mdc);
			}
		}        
    }//Par
    //=======================================================================================================
    return 0;
}


