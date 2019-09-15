#define __GLOBAL_HARDWARE_MODULE_
#define __GLOBAL_CLIENT_

#include <xs1.h>
#include <platform.h>
// extern h
#include "ethernet.h"
// User File
#include "eth_audio.h"
#include "eth_audio_config.h"
#include "audio_buffmanage.h"

#include "audio_tx.h"

#include "debug_print.h"

//--------------------------------------------------------------------------------------------
// Eth Audio Moule 
void eth_audio(client ethernet_cfg_if i_eth_cfg,
                 streaming chanend c_tx_hp,
			     server ethaud_cfg_if i_ethaud_cfg[n_ethaud_cfg],
				 static const unsigned n_ethaud_cfg,
                 client music_decoder_output_if if_mdo)
{
	//-----------------------------------------------------------------------------
	par{
		audio_tx(if_mdo,c_tx_hp);
		audio_buffmanage_process(i_eth_cfg,i_ethaud_cfg,n_ethaud_cfg);
		//clock_inc();
	}
}


