#ifndef  __AUDIO_BUFFMANAGE_H
#define  __AUDIO_BUFFMANAGE_H

#include <xs1.h>
#include <platform.h>
#include <stdint.h>
#include "eth_audio.h"
#include "audio_tx.h"


typedef struct mp3_buf_t{
    unsigned char data[2304];
    unsigned len;
    unsigned mp3buf_inc;
}mp3_buf_t;


void audio_buffmanage_process(   client ethernet_cfg_if i_eth_cfg, int is_hp,
								 server ethaud_cfg_if i_ethaud_cfg[n_ethaud_cfg],
								 static const unsigned n_ethaud_cfg);

//=============================================================================
#endif // __AUDIO_BUFFMANAGE_H

