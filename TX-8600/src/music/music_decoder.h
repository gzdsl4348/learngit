#ifndef _MUSIC_DECODER_H_
#define _MUSIC_DECODER_H_
#include "sys.h"
#include "ff.h"	
#include "music_decoder_types.h"

#if defined(__XC__)
extern "C" {
#endif


#define FRBUFF_USER_FILE    0
#define FRBUFF_USER_DECODER 1

#define MUSIC_FRBUFF_SZ     4*1024
#define MUSIC_DEBUFF_SZ     5*1024	//MP3解码时,文件buf大小
#define MUSIC_PCM_BUFF_SZ   2304

#define MP3DEC_CHANNAL_NUM  MUSIC_CHANNEL_NUM

#define MP3_DECODER_ERROR_MAX_CNT   100

typedef struct
{
    // mp3文件解码 
    FIL file;
    int decoder_status;
    int decoder_error_cnt;
    int frpos;
    int frbuffleft;                  //frbuff还剩余的有效数据
    u8 frbuff[MUSIC_FRBUFF_SZ];   //5*1024 byte
    
    
    int depos;                      //MP3解码读偏移
    int debuffleft;                 //debuff还剩余的有效数据
    u8 debuff[MUSIC_DEBUFF_SZ];   //5*1024 byte

    u32 curnamepos;                 //当前文件偏移

    // pcm数据 
    u8 pcmbuff1[MUSIC_PCM_BUFF_SZ]; //单通道数据， 2304 byte
    
    u8 pcmbuff2[MUSIC_PCM_BUFF_SZ];
    u8 pcmbuff1_isfill;  
    u8 pcmbuff2_isfill;

    //05-07旧版本,pcmbuff_switch的解析
    //0 - buff1 is front buff2 is back
    //1 - buff1 is back  buff2 is front

    //0 - buff2 no frist get 
    //1 - buff2 is frist get
    u8 pcmbuff_switch;
    


    // mp3文件信息 - 这部分需要在前期准备mp3文件列表时，写到flash保存
    u32 totsec ;                    //整首歌时长,单位:秒
    u32 bitrate;                    //比特率(位速)
    u32 samplerate;                 //采样率 
    u16 bps;                        //位数,比如16bit,24bit,32bit
    u32 datastart;                  //数据帧开始的位置(在文件里面的偏移)
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

void music_file_handle();

int music_decode_start(unsigned char ch, unsigned char f_name[], unsigned int f_offset);
int music_decode_stop(unsigned char ch, unsigned char change_status);
int music_decode_play(unsigned char ch);
int music_decode_pause(unsigned char ch);
int update_music_decoder_status(music_decoder_status_t s[]);


#endif
