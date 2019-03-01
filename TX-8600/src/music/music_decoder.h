#ifndef _MUSIC_DECODER_H_
#define _MUSIC_DECODER_H_
#include <stdint.h>
#include <sdram.h>
#include "sys.h"
#include "ff.h"	
#include "music_decoder_types.h"

#if defined(__XC__)
extern "C" {
#endif


#define FRBUFF_USER_FILE    0
#define FRBUFF_USER_DECODER 1

#define MUSIC_FILE_BUFF_SZ      (8*1024)

#define MP3DEC_CHANNAL_NUM  MUSIC_CHANNEL_NUM

#define MP3_DECODER_ERROR_MAX_CNT   50

typedef struct
{
    int decoder_status;
    int decoder_error_cnt;
    
    // mp3文件解码 
    FIL file;    
    uint8_t file_over_flag;
    uint8_t file_close_flag;
    
    // mp3原始数据sdram double buff
    uint32_t file_buff_size[2];     // 0 - 为空
    uint32_t file_buff_offset[2];   // 
    uint8_t file_buff_for_used;     // 0 - 先被消费 1 - 先被消费

    // mp3 frame data
    uint8_t mp3_frame[/*1024*2*/1200];
    uint8_t mp3_frame_full;
    uint32_t mp3_frame_size;
    uint32_t mp3_frame_num;

    // mp3文件信息
    uint32_t totsec ;                    //整首歌时长,单位:秒
    uint32_t bitrate;                    //比特率(位速)
    uint32_t samplerate;                 //采样率 
    uint16_t bps;                        //位数,比如16bit,24bit,32bit
    uint32_t datastart;                  //数据帧开始的位置(在文件里面的偏移)

    uint32_t curnamepos;                 //当前文件偏移
}music_decoderdev_t;


typedef struct
{
    int ch_num;
    music_decoderdev_t ch_dev[MUSIC_CHANNEL_NUM];
}music_decoder_mgr_t;


#if defined(__XC__)
}
#endif

void music_decoder_mgr_init();

void music_file_handle(STREAMING_CHANEND(c_sdram), REFERENCE_PARAM(s_sdram_state, p_sdram_state));

int music_decode_start(unsigned char ch, unsigned char f_name[], unsigned int f_offset);
int music_decode_stop(unsigned char ch, unsigned char change_status);
int music_decode_play(unsigned char ch);
int music_decode_pause(unsigned char ch);
int update_music_decoder_status(music_decoder_status_t s[]);


#endif
