#include <xs1.h>
#include <string.h>
#include <debug_print.h>
#include "music_decoder.h"
#include "music_decoder_server.h"

#define SAMPLERATE_TICK_44100 2612245
#define SAMPLERATE_TICK_48000 2400000

extern uint32_t get_mp3_frame(unsigned char ch, uint32_t &length, uint32_t &frame_num, uint32_t &samplerate);
extern void clear_mp3_frame(unsigned char ch);

[[combinable]]
void music_decoder_server(server interface music_decoder_output_if if_mdo)
{
    uint8_t *unsafe p_mp3_frame = NULL;
    
    while(1) {
        select {
            case if_mdo.get_mp3_frame(uint8_t ch, uint8_t mp3_frame[], uint32_t &length, uint32_t &frame_num, uint32_t &samplerate):
            {
                unsafe {
                    uint32_t l, n, s;
                    p_mp3_frame = (uint8_t *unsafe)get_mp3_frame(ch, l, n, s);
                    if(p_mp3_frame)
                    {
                        memcpy(mp3_frame, p_mp3_frame, l);
                        length = l;
                        frame_num = n;
                        samplerate = s;
                        clear_mp3_frame(ch);
                    }
                    else
                    {
                        length = 0;
                        frame_num = 0;
                        samplerate = 0;
                    }
                }
                break;
            }
        }
    }
}

