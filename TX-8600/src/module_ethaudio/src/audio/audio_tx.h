#ifndef __AUDIO_TX_H
#define __AUDIO_TX_H

#include "ethernet.h"
#include "eth_audio_config.h"

typedef struct audio_tx_frame_t {
    int16_t samples[NUM_MEDIA_INPUTS][ETH_AUDIO_PAGE];     //I2S Input Buff
    int16_t adpcm_sample[NUM_MEDIA_INPUTS];
	uint8_t adpcm_index[NUM_MEDIA_INPUTS];
}audio_tx_frame_t;

typedef struct double_txbuf_t {
	audio_tx_frame_t t_frame;
}double_txbuf_t;

void clock_inc();

void audio_tx(client ethernet_rx_if ? i_eth_rx_lp,
			    client ethernet_tx_if ? i_eth_tx_lp,
		        streaming chanend ? c_rx_hp,
                streaming chanend ? c_tx_hp);

#endif	//__AUDIO_TX_H

