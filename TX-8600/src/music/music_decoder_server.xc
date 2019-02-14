#include <xs1.h>
#include <string.h>
#include <debug_print.h>
#include "music_decoder.h"
#include "music_decoder_server.h"

#define SAMPLERATE_TICK_44100 2612245
#define SAMPLERATE_TICK_48000 2400000

extern int get_ch_flag();
extern int get_ch_samplerate(unsigned char ch);
extern unsigned int get_pcmbuff_ptr(unsigned char ch);
//extern void clean_pcmbuff_flag(unsigned char ch);
extern void clean_pcmbuff(unsigned char ch, unsigned int ptr);

[[combinable]]
void music_decoder_server(server interface music_decoder_output_if if_mdo)
{
    timer tmr;
    unsigned timeout = 0;
    unsigned music_tick = SAMPLERATE_TICK_48000;
    unsigned ch_flag = 0;
    unsigned char *unsafe p_pcmbuff = NULL;
    
    tmr :> timeout;
    timeout += music_tick;

    while(1) {
        select {
            case if_mdo.get_pcmbuff_active(unsigned char ch, unsigned int &samplerate, unsigned char pcmbuff[n], unsigned n, unsigned &length) -> unsigned r_ch_flag:
            {
                unsafe{     
                    
                p_pcmbuff = (unsigned char *unsafe)get_pcmbuff_ptr(ch);
                
                if(p_pcmbuff != NULL)
                {
                    samplerate = get_ch_samplerate(ch);
                    
                    memcpy(pcmbuff, p_pcmbuff, MUSIC_PCM_BUFF_SZ);
                    
                    clean_pcmbuff(ch, (unsigned int)p_pcmbuff);
                    
                    length = MUSIC_PCM_BUFF_SZ;
                    
                    break;
                }
                else
                {
                    length = 0;
                }

                }
                r_ch_flag = ch_flag = get_ch_flag();
	    
		        break;
            }
        }
    }
}

