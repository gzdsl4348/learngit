#ifndef __AUDIO_TX_H
#define __AUDIO_TX_H

#include "ethernet.h"
#include "eth_audio_config.h"
#include "music_decoder_server.h"

void audio_tx(  client music_decoder_output_if if_mdo,
                client ethernet_rx_if ? i_eth_rx_lp,
			    client ethernet_tx_if ? i_eth_tx_lp,
		        streaming chanend ? c_rx_hp,
                streaming chanend ? c_tx_hp);

#endif	//__AUDIO_TX_H

