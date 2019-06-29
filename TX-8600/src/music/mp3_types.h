#ifndef _MP3_TYPES_H_
#define _MP3_TYPES_H_

#if defined(__XC__)
extern "C" {
#endif

//MP3 Xing帧信息(没有全部列出来,仅列出有用的部分)
typedef struct 
{
    unsigned char id[4];            //帧ID,为Xing/Info
    unsigned char flags[4];     //存放标志
    unsigned char frames[4];        //总帧数
    unsigned char fsize[4];     //文件总大小(不包含ID3)
}MP3_FrameXing;

//MP3 VBRI帧信息(没有全部列出来,仅列出有用的部分)
typedef struct 
{
    unsigned char id[4];            //帧ID,为Xing/Info
    unsigned char version[2];       //版本号
    unsigned char delay[2];     //延迟
    unsigned char quality[2];       //音频质量,0~100,越大质量越好
    unsigned char fsize[4];     //文件总大小
    unsigned char frames[4];        //文件总帧数 
}MP3_FrameVBRI;
//MP3_INFO
typedef struct 
{
    unsigned int totsec ;                   //整首歌时长,单位:秒

    unsigned int bitrate;                   //比特率
    unsigned int samplerate;                //采样率
    unsigned short outsamples;              //PCM输出数据量大小(以16位为单位),单声道MP3,则等于实际输出*2(方便DAC输出)

    unsigned char wav_bitwidth;
    unsigned char wav_chanal_num;
    unsigned char wav_format;
    unsigned wav_datlen;
    
    unsigned int datastart;                 //数据帧开始的位置(在文件里面的偏移)
}MP3_Info;

#if defined(__XC__)
}
#endif

#endif
