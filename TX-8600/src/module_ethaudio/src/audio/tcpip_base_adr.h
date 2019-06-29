#ifndef _TCP_IP_BaseAdr_H_
#define _TCP_IP_BaseAdr_H_

//=========================================================
//  TCP/IP Adr Def
//=========================================================
//--------------------MAC Header-----------------
#define UDP_DESMAC_ADR        0
//------------------- IP Header ---------------------
#define UDP_IPHEADLEN_ADR     16

#define UDP_IDENT_ADR         18

#define UDP_IPHEAD_SUM_ADR    24

#define UDP_SOURCE_IP_ADR     26
#define UDP_DES_IP_ADR        30  //4Byte-------34

#define IP_HEADER_LEN         20	  // IP Header Len Num

//------------------UDP Header -------------------
#define UDP_SOUPORT_ADR   	34
#define UDP_DESPORT_ADR   	36
#define UDP_HEADERLEN_ADR   38
#define UDP_HEADERSUM_ADR   40
//
//====================================================
// Header Base Address
#define IP_HEADBASE_ADR     14
#define UDP_HEADBASE_ADR    34
#define UDP_DATA_BASE_ADR   42
//====================================================
// UDP Data Base Adress
#define AUDIO_TYPE_ADR          (UDP_DATA_BASE_ADR)  //2Byte    //音频头 0XAAAA
// 音频时间戳
#define AUDIO_TIMESTAMP         (AUDIO_TYPE_ADR+2)  //4Byte
// 音频分区标志
#define AUDIO_AREA_F            (AUDIO_TIMESTAMP+4) //1Byte
// 保留标志
#define AUDIO_STATE_ADR         (AUDIO_AREA_F+1)    //2Byte
//
#define AUDIO_FORMAT_ADR        (AUDIO_STATE_ADR+2)  //1Byte     //音频包格式 采样率/位宽
//
#define AUDIO_CHTOTAL_ADR       (AUDIO_FORMAT_ADR+1)  //1Byte   //音频通道总数
//
#define AUDIO_CHDATA_BASE_ADR   (AUDIO_CHTOTAL_ADR+1)  //1Byte  //音频通道数据起始位
//
#define AUDIO_AUXTYPE_ADR       0   //1Byte         //音频类型 S,P,E
//
#define AUDIO_CHID_ADR          (AUDIO_AUXTYPE_ADR+1)   //1Byte    // 音频通道id
//
#define AUDIO_CHPRIO_ADR        (AUDIO_CHID_ADR+1)   //1Byte       //音频优先级 保留
//
#define AUDIO_CHVOL_ADR         (AUDIO_CHPRIO_ADR+1)   //1Byte     //通道音量
//
#define AUDIO_SILENT_ADR        (AUDIO_CHVOL_ADR+1)   //1Byte      //默音等级
// 
#define AUDIO_DATALEN_ADR       (AUDIO_SILENT_ADR+1)   //2Byte     //音频采样数据长度
//
#define AUDIO_DATABASE_ADR		(AUDIO_DATALEN_ADR+2) 	//Nbyte     //采样数据
//
// 包头定义
#define WAV_PAGE_HEAD       (0)  // 4字节 包头
#define WAV_PAGE_FORMAT     (WAV_PAGE_HEAD+4)   // 1字节 格式
#define WAV_PAGE_SAMPLERATE (WAV_PAGE_FORMAT+1) // 4字节 采样率
#define WAV_PAGE_DATLEN     (WAV_PAGE_SAMPLERATE+4) //2 字节 数据长度     
#define WAV_PAGE_ADPCMINDEX (WAV_PAGE_DATLEN+2) // 1字节
#define WAV_PAGE_DATBASE    (WAV_PAGE_ADPCMINDEX+1) // 数据起始

// 包头长度
#define WAV_PAGEREAD_SIZE   (WAV_PAGE_DATBASE-WAV_PAGE_HEAD)

#define WAV_FORMAT_PCM      0
#define WAV_FORMAT_ADPCM    1

//===========================================================================================
#endif
