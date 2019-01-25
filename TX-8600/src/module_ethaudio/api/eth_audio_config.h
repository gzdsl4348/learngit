#ifndef  __ETH_AUDIO_CON_H_
#define  __ETH_AUDIO_CON_H_

#ifdef __XC__
extern "C" {
#endif
//===============================================================
// max volume level
#define MIX_VOL		50
//--------Audio ETH PORT ----------
#define	ETH_AUDIO_PORT		8802

#define NUM_MEDIA_INPUTS    64

//--------Audio Eth MAX TX Chanend---------
#define MAX_SENDCHAN_NUM    64

//--------Audio Eth Wait Packet--------
#define AUDIO_WAIT_PAGE     2
//--------Audio Max Buffer Timeing----------
#define Eth_AUDIO_MAXPAGE   4
//--------Recive EthAudio Data Fifo Size----
#define FIFO_SIZE   ETH_AUDIO_PAGE*Eth_AUDIO_MAXPAGE
//--------SRC Tolnum-----------------------
#define SRC_CH_TOLNUM 4

#define SRC_SAMPLEIN_NUM 16


#define AUX_TYPE_NUM  5
#define AUX_RXCH_NUM  5

#define AUX_TYPE_S  0x00
#define AUX_TYPE_P  0x10
#define AUX_TYPE_I  0x20
#define AUX_TYPE_E  0x30
#define AUX_TYPE_M  0x40

//==============================================================
//--------------Def Ethernet Audio Type 
//----------------------------------------------------------
enum AUDIO_TYPE_E{
		AUX_S1=0x00,
        AUX_S2,
        AUX_S3,
        AUX_S4,
        AUX_S5,

        AUX_P1=0x10,
        AUX_P2,
        AUX_P3,
        AUX_P4,
        AUX_P5,

        AUX_I1=0x20,
        AUX_I2,
        AUX_I3,
        AUX_I4,
        AUX_I5,

        AUX_E1=0x30,
        AUX_E2,
        AUX_E3,
        AUX_E4,
        AUX_E5,

        AUX_M5=0x45,
};

//==============================================================
//--------------Def Audio Prio Level
//----------------------------------------------------------
enum AUDIO_PRIOLV_E{
	AUDIO_LOWLV = 0X00,
	AUDIO_HIGHLV = 0x80
};

//==============================================================
//--------------Def Audio Muticast or Unicast EN
//----------------------------------------------------------
enum AUDIO_CASTMODE_E{
	AUDIO_UNICAST = 1,
	AUDIO_MUTICAST
};

//==============================================================
//--------------Def Audio chanal num 
//----------------------------------------------------------
#define AUDIO_CH0 0
#define AUDIO_CH1 1
#define AUDIO_CH2 2
#define AUDIO_CH3 3
#define AUDIO_CH4 4
#define AUDIO_CH5 5
#define AUDIO_CH6 6
#define AUDIO_CH7 7

//==============================================================
//--------------Def Audio chanal enable or disable 
//----------------------------------------------------------
#define AUDIO_CH0_EN 0x01
#define AUDIO_CH1_EN 0x02
#define AUDIO_CH2_EN 0x04
#define AUDIO_CH3_EN 0x08
#define AUDIO_CH4_EN 0x10
#define AUDIO_CH5_EN 0x20
#define AUDIO_CH6_EN 0x40
#define AUDIO_CH7_EN 0x80

//==============================================================
//--------------Def Audio Format State
//----------------------------------------------------------
// Bit Width
#define AUDIOWIDTH_8BIT     0
#define AUDIOWIDTH_16BIT    1
#define AUDIOWIDTH_24BIT    2
#define AUDIOWIDTH_32BIT    3
#define AUDIOWIDTH_ADPCM    4
#define AUDIOWIDTH_MP3      5

//Sample Rate
#define SAMPLE_RATE_8K       0
#define SAMPLE_RATE_16K      1
#define SAMPLE_RATE_24K      2
#define SAMPLE_RATE_32K      3
#define SAMPLE_RATE_44_1K    4
#define SAMPLE_RATE_48K      5
#define SAMPLE_RATE_88_1K    6
#define SAMPLE_RATE_96K      7
#define SAMPLE_RATE_192K     8


//--------Audio Eth Data Packet----------
#define ETH_AUDIO_PAGE_4MS  192   //48K 4ms
#define ETH_AUDIO_PAGE_1MS  48    //48K 1ms

#define ETH_AUDIO_COUNT_1MS 6    //48K 1ms
#define ETH_AUDIO_COUNT_4MS 24   //48K 4ms


//#define ETH_AUDIO_PAGE    96   //24K 4ms
//#define ETH_AUDIO_PAGE    24   //24K 1ms

//#define ETH_AUDIO_PAGE    64   //16K 4ms
//#define ETH_AUDIO_PAGE    16   //16K 1ms

//#define ETH_AUDIO_PAGE    32   //8K  4ms
//#define ETH_AUDIO_PAGE    8    //8K  1ms

#define ETH_AUDIO_PAGE	ETH_AUDIO_PAGE_4MS	//MAX PAGE SIZE  

//#define TX_SENDTIME (100000*ETH_AUDIO_PAGE/ETH_AUDIO_PAGE_1MS)

//---------------------------------------------
//ADPCM TYPE 
#define ADPCM_TYPE 0xA55A5A00

#ifdef __XC__
}
#endif
#endif	//__ETH_AUDIO_CON_H_

