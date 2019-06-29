#include <xs1.h>
#include <string.h>
#include <debug_print.h>
#include "music_decoder.h"
#include "music_decoder_server.h"
#include "tcpip_base_adr.h"
#include "adpcm.h"

#define SAMPLERATE_TICK_44100 2612245
#define SAMPLERATE_TICK_48000 2400000

extern uint32_t get_mp3_frame(unsigned char ch, uint32_t &length, uint32_t &frame_num, uint32_t &samplerate,uint32_t &music_type);
extern void clear_mp3_frame(unsigned char ch);

struct adpcm_state s_adpcm_state[MUSIC_CHANNEL_NUM];

[[combinable]]
void music_decoder_server(server interface music_decoder_output_if if_mdo)
{
    uint8_t *unsafe p_mp3_frame = NULL;
    memset(s_adpcm_state,0x00,sizeof(s_adpcm_state));
    //
    while(1) {
        select {
            case if_mdo.get_mp3_frame(uint8_t ch, uint8_t mp3_frame[], uint32_t &length, uint32_t &frame_num, uint32_t &samplerate,
                                      uint8_t &music_type,uint8_t wav_format):
            {
                unsafe {
                    uint32_t l, n, s,t;
                    p_mp3_frame = (uint8_t *unsafe)get_mp3_frame(ch, l, n, s,t);
                    if(p_mp3_frame)
                    {
                        frame_num = n;
                        samplerate = s;
                        music_type = t;
                        // WAV 增加帧头
                        if(t){
                            // 获取帧头标识
                            mp3_frame[WAV_PAGE_HEAD]=0x71;
                            mp3_frame[WAV_PAGE_HEAD+1]=0x61;
                            mp3_frame[WAV_PAGE_HEAD+2]=0x76;
                            mp3_frame[WAV_PAGE_HEAD+3]=0x00;
                            // 获取采样率
                            mp3_frame[WAV_PAGE_SAMPLERATE] = samplerate;
                            mp3_frame[WAV_PAGE_SAMPLERATE+1] = samplerate>>8;                            
                            mp3_frame[WAV_PAGE_SAMPLERATE+2] = samplerate>>16;
                            mp3_frame[WAV_PAGE_SAMPLERATE+3] = samplerate>>24;
                            // WAV一个包传512字节 特殊处理
                            if(samplerate==44100)
                                samplerate = 88200; 
                            else if(samplerate==48000)
                                samplerate = 96000; 
                            else if(samplerate==16000)
                                samplerate = 31000;
                            else{;}
                            // 获取格式与数据                           
                            if(wav_format==0){ //PCM处理
                                // 获取包总长
                                length = l+WAV_PAGEREAD_SIZE;
                                // 获取字节长度 大端
                                mp3_frame[WAV_PAGE_DATLEN] = l>>8;
                                mp3_frame[WAV_PAGE_DATLEN+1] = l;
                                // 获取格式                            
                                mp3_frame[WAV_PAGE_FORMAT]=WAV_FORMAT_PCM;
                                // 获取PCM数据
                                memcpy(mp3_frame+WAV_PAGE_DATBASE, p_mp3_frame, l);
                            }
                            else{   //ADPCM处理
                                // 获取包总长
                                length = l/4+WAV_PAGEREAD_SIZE;
                                // 获取字节长度 大端
                                mp3_frame[WAV_PAGE_DATLEN] = (l/4)>>8;
                                mp3_frame[WAV_PAGE_DATLEN+1] = l/4;
                                //
                                // 获取ADPCM解码信息
                                mp3_frame[WAV_PAGE_ADPCMINDEX] = s_adpcm_state[ch].index;
                                // 获取格式                            
                                mp3_frame[WAV_PAGE_FORMAT]=WAV_FORMAT_ADPCM;
                                // 获取ADPCM数据
                                unsigned dat_base=WAV_PAGE_DATBASE;
                                uint8_t xor_f=0;
                                int sample;
                                uint16_t buff[1152];                                
                                memcpy(buff, p_mp3_frame, l);
                                for(unsigned i=0;i<l/2;i++){                                
                                    sample = p_mp3_frame[i*2]+(p_mp3_frame[i*2+1]<<8);
                                    switch(xor_f){
                                        case 0:    
                                            mp3_frame[dat_base] = (adpcm_coder(buff[i],s_adpcm_state[ch])<<4);
                                            xor_f=1;
                                            break;
                                        case 1:
                                            mp3_frame[dat_base] |= (adpcm_coder(buff[i],s_adpcm_state[ch]));
                                            
                                            dat_base++;
                                            xor_f=0;
                                            break;
                                    }
                                }
                            }
                            // 
                        }
                        // MP3 处理
                        else{
                            length = l;
                            memcpy(mp3_frame, p_mp3_frame, l);
                        }
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

