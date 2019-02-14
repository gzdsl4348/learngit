#include <string.h>
#include <debug_print.h>
#include <timer.h>
#include "mp3dec.h"
#include "coder.h"

#include "mymalloc.h"
#include "ff.h"	
#include "timing.h"

#include "file.h"
#include "music_decoder.h"
#include "mp3_types.h"
#include "hwlock.h"
#include "xassert.h"
#include "sdram.h"
#include "sdram_def.h"

#define CAN_PUT_FRBUFF(p_dev) (p_dev->decoder_status == MUSIC_DECODER_START && p_dev->frbuffleft == 0)
#define CAN_GET_FRBUFF(p_dev) (p_dev->debuffleft<(MAINBUF_SIZE+(MAINBUF_SIZE/4)) && p_dev->frbuffleft!=0)//当数组内容小于(1.25倍)MAINBUF_SIZE的时候,必须补充新的数据进来.

static music_decoder_mgr_t gt_mmdm;
static uint8_t music_decoder_fclose_flg[MP3DEC_CHANNAL_NUM] = {0};//ADD:20181023

//mp3解码器缓存，MP3Decode使用
//static MP3DecInfo  mdi;                       //  0x7f0   =  2032 
//static SubbandInfo sbi;                       // 0x2204   =  8708
//static IMDCTInfo mi;                          // 0x1b20   =  6944
//static HuffmanInfo hi;                        // 0x1210   =  4624
//static DequantInfo di;                        //  0x348   =   840
//static ScaleFactorInfo sfi;                   //  0x124   =   292
//static SideInfo si;                           //  0x148   =   328
//static FrameHeader fh;                        //   0x38   =    56


typedef struct
{
    MP3DecInfo mdi;
    SubbandInfo sbi;
    IMDCTInfo mi;
    HuffmanInfo hi;
    DequantInfo di;
    ScaleFactorInfo sfi;
    SideInfo si;
    FrameHeader fh;
}mp3decoder_state_t;


static uint8_t mp3decoder_state_clear_flag[MP3DEC_CHANNAL_NUM]={0};
static mp3decoder_state_t g_mp3decoder_state;
static streaming_chanend_t g_mp3decoder_sdram;
static s_sdram_state g_mp3decoder_sdram_state;

static hwlock_t g_mp3_lock;

void mp3_lock_init()
{
    g_mp3_lock = hwlock_alloc();
}


/**
 * 1.SD卡文件列表
 * 2.文件传输
 * 3.mp3解码
 * 三个事件互斥，不能同时进行
 * release_mp3decoder后, 内存可能会被MP3解码线程的MP3Decode占用
 * 使用wait_mp3_dyna_buff_free等待MP3Decode释放内存占用
 */
int wait_mp3_dyna_buff_free()
{
    return 0;
}



static void start_mp3decoder(int ch)
{
    if(ch >=MP3DEC_CHANNAL_NUM) return;
    
    mp3decoder_state_clear_flag[ch] = 1;
}

static HMP3Decoder in_mp3decoder(int ch)
{
    MP3DecInfo *p_mdi =  NULL;
    
    if(ch >=MP3DEC_CHANNAL_NUM) return NULL;
    if(mp3decoder_state_clear_flag[ch])
    {
        mp3decoder_state_clear_flag[ch] = 0;
        memset(&g_mp3decoder_state, 0, sizeof(g_mp3decoder_state));
        sdram_write(g_mp3decoder_sdram, &g_mp3decoder_sdram_state, SDRAM_MP3DECODER_START+ch*(sizeof(g_mp3decoder_state)/4), sizeof(g_mp3decoder_state)/4, (unsigned*)&g_mp3decoder_state);
        sdram_complete(g_mp3decoder_sdram, &g_mp3decoder_sdram_state);         
    }
    
    //read sdram
    sdram_read(g_mp3decoder_sdram, &g_mp3decoder_sdram_state, SDRAM_MP3DECODER_START+ch*(sizeof(g_mp3decoder_state)/4), sizeof(g_mp3decoder_state)/4, (unsigned*)&g_mp3decoder_state);
    sdram_complete(g_mp3decoder_sdram, &g_mp3decoder_sdram_state);
    
    p_mdi = &g_mp3decoder_state.mdi;//&mdi[ch];
    p_mdi->SubbandInfoPS = &g_mp3decoder_state.sbi;//&sbi[ch];
    p_mdi->IMDCTInfoPS = &g_mp3decoder_state.mi;//&mi[ch];
    p_mdi->HuffmanInfoPS = &g_mp3decoder_state.hi;
    p_mdi->DequantInfoPS = &g_mp3decoder_state.di;
    p_mdi->ScaleFactorInfoPS = &g_mp3decoder_state.sfi;
    p_mdi->SideInfoPS = &g_mp3decoder_state.si;
    p_mdi->FrameHeaderPS = &g_mp3decoder_state.fh;

    return p_mdi; 
}
static void out_mp3decoder(int ch)
{
    if(ch >=MP3DEC_CHANNAL_NUM) return;
    //write sdram
    sdram_write(g_mp3decoder_sdram, &g_mp3decoder_sdram_state, SDRAM_MP3DECODER_START+ch*(sizeof(g_mp3decoder_state)/4), sizeof(g_mp3decoder_state)/4, (unsigned*)&g_mp3decoder_state);
    sdram_complete(g_mp3decoder_sdram, &g_mp3decoder_sdram_state);   

    //memset(&g_mp3decoder_state, 0, sizeof(g_mp3decoder_state));
}

unsigned int get_mp3_datastart(unsigned char *buff, unsigned int buff_size)
{
    if(buff_size >= 10)
    {
        if (memcmp(buff, "ID3",3) == 0)
        {
            return ((buff[6] & 0x7F)<< 21)|((buff[7] & 0x7F) << 14) | ((buff[8] & 0x7F) << 7) | (buff[9] & 0x7F);
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

//获取MP3基本信息
//used ram:28+2032+56 ~36+2048+64 = 2148
//pname:MP3文件路径
//pctrl:MP3控制信息结构体
//返回值:0,成功
//    其他,失败
unsigned char mp3_get_info(TCHAR *pname, MP3_Info *p_info)
{
    MP3FrameInfo frame_info;
    MP3_FrameXing* fxing;
    MP3_FrameVBRI* fvbri;
    FIL*fmp3;
    unsigned char *buf;
    unsigned int br;
    unsigned char res;
    int offset=0;
    unsigned int p;
    short samples_per_frame;    //一帧的采样个数
    unsigned int totframes;             //总帧数

    fmp3=mymalloc(sizeof(FIL));

    //共用5K内存
    buf = gt_mmdm.ch_dev[0].debuff;

    if(fmp3&&buf)//内存申请成功
    {
        f_open(fmp3,(const TCHAR*)pname,FA_READ);//打开文件
        res=f_read(fmp3,(char*)buf,5*1024,&br);
        if(res==0)//读取文件成功,开始解析ID3V2/ID3V1以及获取MP3信息
        {
            p_info->datastart = get_mp3_datastart(buf, 5*1024);

            /************************************************************************/
            /*MP3解码申请内存*/
            HMP3Decoder decoder = mymalloc(sizeof(MP3DecInfo));//2032
            memset(decoder, 0, sizeof(MP3DecInfo));

            FrameHeader *fh = mymalloc(sizeof(FrameHeader));//56
            memset(fh, 0, sizeof(FrameHeader));
            ((MP3DecInfo*)decoder)->FrameHeaderPS =(void *)fh;

            /************************************************************************/

            f_lseek(fmp3, p_info->datastart);   //偏移到数据开始的地方
            f_read(fmp3, (char*)buf,5*1024,&br);    //读取5K字节mp3数据
            offset=MP3FindSyncWord(buf,br); //查找帧同步信息

            if(offset>=0&&MP3GetNextFrameInfo(decoder,&frame_info,&buf[offset])==0)//找到帧同步信息了,且下一阵信息获取正常
            {
                p=offset+4+32;
                fvbri=(MP3_FrameVBRI*)(buf+p);
                if(strncmp("VBRI",(char*)fvbri->id,4)==0)//存在VBRI帧(VBR格式)
                {
                    if (frame_info.version==MPEG1)samples_per_frame=1152;//MPEG1,layer3每帧采样数等于1152
                    else samples_per_frame=576;//MPEG2/MPEG2.5,layer3每帧采样数等于576
                    totframes=((unsigned int)fvbri->frames[0]<<24)|((unsigned int)fvbri->frames[1]<<16)|((u16)fvbri->frames[2]<<8)|fvbri->frames[3];//得到总帧数
                    p_info->totsec=totframes*samples_per_frame/frame_info.samprate;//得到文件总长度
                }
                else    //不是VBRI帧,尝试是不是Xing帧(VBR格式)
                {
                    if (frame_info.version==MPEG1)  //MPEG1
                    {
                        p=frame_info.nChans==2?32:17;
                        samples_per_frame = 1152;   //MPEG1,layer3每帧采样数等于1152
                    }
                    else
                    {
                        p=frame_info.nChans==2?17:9;
                        samples_per_frame=576;      //MPEG2/MPEG2.5,layer3每帧采样数等于576
                    }
                    p+=offset+4;
                    fxing=(MP3_FrameXing*)(buf+p);
                    if(strncmp("Xing",(char*)fxing->id,4)==0||strncmp("Info",(char*)fxing->id,4)==0)//是Xng帧
                    {
                        if(fxing->flags[3]&0X01)//存在总frame字段
                        {
                            totframes=((unsigned int)fxing->frames[0]<<24)|((unsigned int)fxing->frames[1]<<16)|((u16)fxing->frames[2]<<8)|fxing->frames[3];//得到总帧数
                            p_info->totsec=totframes*samples_per_frame/frame_info.samprate;//得到文件总长度
                        }
                        else    //不存在总frames字段
                        {
                            p_info->totsec=fmp3->fsize/(frame_info.bitrate/8);
                        }
                    }
                    else        //CBR格式,直接计算总播放时间
                    {
                        p_info->totsec=fmp3->fsize/(frame_info.bitrate/8);
                    }
                }
                p_info->bitrate=frame_info.bitrate;         //得到当前帧的码率
                p_info->samplerate=frame_info.samprate;     //得到采样率.
                if(frame_info.nChans==2)p_info->outsamples=frame_info.outputSamps; //输出PCM数据量大小
                else p_info->outsamples=frame_info.outputSamps*2; //输出PCM数据量大小,对于单声道MP3,直接*2,补齐为双声道输出
            }
            else res=0XFE; //未找到同步帧
            /************************************************************************/
            /*MP3内存释放*/
            myfree(fh);
            myfree(decoder);
            /************************************************************************/

        }
        f_close(fmp3);
    }
    else res=0XFF;

    myfree(fmp3);
    return res;
}

//获取mp3文件的播放时长，耗时约为6ms
//pname:文件名
//返回值:0 - 文件无效或失败
//       其他值为mp3文件总时长
unsigned int get_mp3_totsec(TCHAR *pname)
{
    MP3_Info mp3_info;
    if(mp3_get_info(pname, &mp3_info) != 0)
    {
        return 0;
    }
    
    if(mp3_info.samplerate!=48000 && mp3_info.samplerate!=44100)
        return 0;
    else
        return mp3_info.totsec;
}

/*********************************************************************************************************************************/
#define _USE_IN_FILE_SYSTEM_TASK_

void music_decoder_mgr_init()
{
    memset(&gt_mmdm, 0, sizeof(gt_mmdm));   
    gt_mmdm.ch_num = MUSIC_CHANNEL_NUM;
    
    memset(music_decoder_fclose_flg, 0, sizeof(music_decoder_fclose_flg));
}

int music_decode_start(unsigned char ch, unsigned char f_name[], unsigned int f_offset)
{
    int res = 0;
    unsigned char tag[10];

    UINT br;
    
    if(ch >= gt_mmdm.ch_num)
    {
        res = 1;
        return res;
    }
    
    if(music_decoder_fclose_flg[ch]) 
    {
        debug_printf(" 1 set music_decoder_fclose_flg[%d] = 0\n", ch);
        f_close(&gt_mmdm.ch_dev[ch].file);
        memset(&gt_mmdm.ch_dev[ch].file, 0, sizeof(FIL));
        music_decoder_fclose_flg[ch] = 0;
    }
    
    res = f_open(&gt_mmdm.ch_dev[ch].file, (const TCHAR*)f_name, FA_READ);
    if(res != FR_OK)
    {
        return res;
    }

    res = f_read(&gt_mmdm.ch_dev[ch].file, tag, 10, &br);
	f_offset += get_mp3_datastart(tag, 10);

    res = f_lseek(&gt_mmdm.ch_dev[ch].file, f_offset);
    if(res != FR_OK)
    {
        return res;
    }

    //清空数据
    //TUDO:直接清除，同一通道快速切换下一曲时有几率导致清掉有用的pcmbuff
    memset(&gt_mmdm.ch_dev[ch].decoder_status, 0, sizeof(music_decoderdev_t)-sizeof(FIL));
    start_mp3decoder(ch);
    
    gt_mmdm.ch_dev[ch].decoder_status = MUSIC_DECODER_START;
    gt_mmdm.ch_dev[ch].decoder_error_cnt = 0;
    
    return FR_OK;
}

//f_close 只是内存数据操作，没有硬件操作
int music_decode_stop(unsigned char ch, unsigned char change_status)
{
    int res = 0;
    if(ch >= gt_mmdm.ch_num)
    {
        res = -1;
        return res;
    }
    
    //f_close(&gt_mmdm.ch_dev[ch].file);
    //memset(&gt_mmdm.ch_dev[ch].file, 0, sizeof(FIL));
    music_decoder_fclose_flg[ch] = 1;
    
	if(change_status)
    	gt_mmdm.ch_dev[ch].decoder_status = MUSIC_DECODER_STOP;
    
    return 0;
}

int music_decode_play(unsigned char ch)
{
    return 0;
}

int music_decode_pause(unsigned char ch)
{
    return 0;
}

int update_music_decoder_status(music_decoder_status_t s[])
{
    uint8_t need2notify = 0;
    for(uint8_t ch=0; ch<gt_mmdm.ch_num; ch++)
    {
        // MUSIC_DECODER_START/MUSIC_DECODER_STOP 不会通知更新
        if(gt_mmdm.ch_dev[ch].decoder_status != s[ch].status &&
           gt_mmdm.ch_dev[ch].decoder_status != MUSIC_DECODER_START &&
           gt_mmdm.ch_dev[ch].decoder_status != MUSIC_DECODER_STOP)
        {
            music_decode_stop(ch, 0);
            s[ch].is_new = 1;
            need2notify = 1;
            //debug_printf("set need2notify=1 [%d] %d %d\n", ch, gt_mmdm.ch_dev[ch].decoder_status, s[ch].status);
            
            s[ch].status = gt_mmdm.ch_dev[ch].decoder_status;
            gt_mmdm.ch_dev[ch].decoder_status= MUSIC_DECODER_STOP;//ADD:20181023
        }
        else
        {
            s[ch].status = gt_mmdm.ch_dev[ch].decoder_status;
        }
    }
    
    return need2notify;
}




void music_file_handle()
{
    UINT br = 0;
    music_decoderdev_t * p_dev = NULL;
    
    for(int ch=0; ch<gt_mmdm.ch_num; ch++)
    {
        p_dev = &gt_mmdm.ch_dev[ch];
        if(CAN_PUT_FRBUFF(p_dev))//TODO:判断会出现bug
        {
            
            f_read(&p_dev->file, p_dev->frbuff, MUSIC_FRBUFF_SZ, &br);
            if(br == 0)
            {
                //读文件结束,直接结算导致丢弃约0.5s的音频
                p_dev->decoder_status = MUSIC_DECODER_FILE_END;
                
                debug_printf("f_read over %d\n", ch);
                continue;
            }
            p_dev->frbuffleft = br;
            p_dev->frpos = 0;
            //debug_printf("f_read %d %d\n", index, br);
        }

        if(music_decoder_fclose_flg[ch]) 
        {
            debug_printf(" 2 set music_decoder_fclose_flg[%d] = 0\n", ch);
            f_close(&p_dev->file);
            memset(&p_dev->file, 0, sizeof(FIL));
            music_decoder_fclose_flg[ch] = 0;
        }        
        
    }
    
}

/*********************************************************************************************************************************/
#define _USE_IN_MUSIC_DECOEDR_SERVER_TASK_

int get_ch_flag()
{
    unsigned int ch_flag = 0;
    music_decoderdev_t *p_dev;
    
    for(int ch=0; ch<gt_mmdm.ch_num; ch++)
    {
        p_dev = &gt_mmdm.ch_dev[ch];
        if(p_dev->pcmbuff1_isfill==1 || p_dev->pcmbuff2_isfill==1)
        {
            ch_flag |= (1<<ch);
        }
    }

    return ch_flag;
}

int get_ch_samplerate(unsigned char ch)
{
    if(ch >= gt_mmdm.ch_num) return 0;
    
    return gt_mmdm.ch_dev[ch].samplerate;
}

int get_pcmbuff(unsigned char ch, unsigned char pcmbuff[])
{
    
    if(ch >= gt_mmdm.ch_num) return 1;
    
    u8 *pc = NULL;
    music_decoderdev_t *p_dev= &gt_mmdm.ch_dev[ch];

    if(p_dev->decoder_status != MUSIC_DECODER_START) 
        return 2;
    
    if(p_dev->pcmbuff1_isfill==1)
    {
        pc = p_dev->pcmbuff1;
    }
    
    if(p_dev->pcmbuff2_isfill==1 && p_dev->pcmbuff_switch==1)
    {
        pc = p_dev->pcmbuff2;
    }
    
    if(pc == NULL)
    {
        return 2;
    }
    
    memcpy(pcmbuff, pc, MUSIC_PCM_BUFF_SZ);

    if(pc == p_dev->pcmbuff1)
    {
        p_dev->pcmbuff1_isfill = 0;
    }
    else if(pc == p_dev->pcmbuff2) 
    {
        p_dev->pcmbuff2_isfill = 0;
    }

    return 0;
}
unsigned int get_pcmbuff_ptr(unsigned char ch)
{
    if(ch >= gt_mmdm.ch_num) return NULL;
    
    u8 *pc = NULL;
    
    music_decoderdev_t *p_dev= &gt_mmdm.ch_dev[ch];

    if(p_dev->decoder_status != MUSIC_DECODER_START) 
        return NULL;
    
    if(p_dev->pcmbuff1_isfill==1)
    {
        pc = p_dev->pcmbuff1;
    }
    
    if(p_dev->pcmbuff_switch==1 && p_dev->pcmbuff2_isfill==1)
    {
        pc = p_dev->pcmbuff2;
    }
    return (unsigned int)pc;
}

void clean_pcmbuff(unsigned char ch, unsigned int ptr)
{
    if(ch >= gt_mmdm.ch_num) return;
    music_decoderdev_t *p_dev= &gt_mmdm.ch_dev[ch];

    if(ptr == (unsigned int)p_dev->pcmbuff1)
    {
        p_dev->pcmbuff1_isfill = 0;
    }
    else if(ptr == (unsigned int)p_dev->pcmbuff2) 
    {
        p_dev->pcmbuff2_isfill = 0;
    }
}

/*********************************************************************************************************************************/
#define _USE_IN_MUSIC_DECODER_HANDLE_TASK_

static void frbuff2debuff(music_decoderdev_t * p_dev)
{
    UINT br = MUSIC_DEBUFF_SZ-p_dev->debuffleft;
        
    //读取文件缓存
    memmove(p_dev->debuff, p_dev->debuff+p_dev->depos, p_dev->debuffleft);//移动depos所指向的数据到debuff里面,数据量大小为:debuffleft
    
    if(br > p_dev->frbuffleft)//debuff大于frbuff 的 空余空间
    {
        br = p_dev->frbuffleft;
    }
    
    memcpy(p_dev->debuff+p_dev->debuffleft, &p_dev->frbuff[p_dev->frpos], br);
    
    p_dev->debuffleft += br;
    p_dev->depos = 0;
    
    p_dev->frpos += br;
    p_dev->frbuffleft -= br;

}

//填充PCM数据到发送buff
//pcm_buf:PCM数据首地址
//size:pcm数据量(16位为单位)
//nch:声道数(1,单声道,2立体声)
static inline void pcm_fill_buff(music_decoderdev_t * p_dev, u16 *pcm_buf, u16 words_size, u8 nch)
{
    u16 *pw = NULL;
    int i = 0;
    //可以确定，一定会有空的buff
    if(p_dev->pcmbuff1_isfill==0)
    {
        pw = (u16*)p_dev->pcmbuff1;
    }
    else if(p_dev->pcmbuff2_isfill==0)
    {
        pw = (u16*)p_dev->pcmbuff2;
    }
    else
    {
        return;
    }
    
	if(nch==2)
    {
        for(i=0; i<(words_size>>1); i++)
        {
            //p[i] = Audio_Mixer(&buf[2*i]);
            pw[i] = pcm_buf[2*i];
        }
    }
	else//单声道
	{
		for(i=0; i<words_size; i++)
		{
			pw[i] = pcm_buf[i];
		}
	}
    
    if(pw == (u16*)p_dev->pcmbuff1) 
    {
        p_dev->pcmbuff1_isfill = 1;

        if(p_dev->pcmbuff2_isfill == 0) 
            p_dev->pcmbuff_switch = 0;
        else 
            p_dev->pcmbuff_switch = 1;
    }
    else if(pw == (u16*)p_dev->pcmbuff2) 
    {
        p_dev->pcmbuff2_isfill = 1;
        
        if(p_dev->pcmbuff1_isfill == 0) 
            p_dev->pcmbuff_switch = 1;
        else 
            p_dev->pcmbuff_switch = 0;
    }
    
}

void printf_buff(uint8_t *buff, uint32_t buff_size)
{
    int i=0;
    while(buff_size--)
    {
        debug_printf(" 0x%x%x", (*buff)>>4, ((*buff)&0x0F));
        i++;
        if(i==16)
        {
            debug_printf("\n");
            i = 0;
        }
        buff++;
    }
    debug_printf("\n");
}


void music_decoder_handle(STREAMING_CHANEND(c_sdram), int start_index, int stop_index)
{
    //unsigned int t;
    HMP3Decoder mp3decoder;
    MP3FrameInfo mp3frameinfo;
    u8 *readptr;
    u16 wbuff[MUSIC_PCM_BUFF_SZ];
    int sync_offset = 0;         //偏移值
    int err = 0;  
    
    g_mp3decoder_sdram = c_sdram;
    sdram_init_state(g_mp3decoder_sdram, &g_mp3decoder_sdram_state);

    music_decoderdev_t * p_dev = NULL;
    debug_printf("  mp3decoder_state_t sizeof = %d\n", sizeof(mp3decoder_state_t));
    while(1)
    {
        //TUDO:应该每个通道需要计时，哪个通道的计时最少，优先解码哪通道
        for(int ch=start_index; ch<(stop_index+1); ch++)
        {
            p_dev = &gt_mmdm.ch_dev[ch];

            if(p_dev->decoder_status != MUSIC_DECODER_START) continue;

            //读取文件缓存
            if(CAN_GET_FRBUFF(p_dev)) 
            {
                frbuff2debuff(p_dev);           
            }

            //pcmbuff需要填充时才调用解码器
            if(p_dev->pcmbuff1_isfill==1 && p_dev->pcmbuff2_isfill==1) continue;
            
            //debuff里有数据才继续执行
            if(p_dev->debuffleft == 0) continue;
            
            readptr = p_dev->debuff+p_dev->depos;
            // 解码
            sync_offset = MP3FindSyncWord(readptr, p_dev->debuffleft);//在readptr位置,开始查找同步字符

            if(sync_offset < 0)//没有找到同步字符,跳出帧解码循环
			{
                //没找到帧同步字符
                //TUDO:设置状态标志，可以找下一帧，亦可以判断为解码结束
                debug_printf("MP3FindSyncWord error:%d\n", sync_offset);
                
                if(p_dev->decoder_status != MUSIC_DECODER_STOP)
                    p_dev->decoder_status = MUSIC_DECODER_ERROR1;
                continue;
			}
            else//找到同步字符了
			{

                readptr += sync_offset;//MP3读指针偏移到同步字符处.
				p_dev->debuffleft -= sync_offset;//buffer里面的有效数据个数,必须减去偏移量

#if 0
                int debuffleft = p_dev->debuffleft;
                u8 * tmp_readptr = readptr;
#endif

                
                mp3decoder = in_mp3decoder(ch);

                if(mp3decoder == NULL) continue;

                if(p_dev->decoder_status != MUSIC_DECODER_START) continue;
                
				err = MP3Decode(mp3decoder, &readptr, &p_dev->debuffleft, (short*)wbuff, 0);//解码一帧MP3数据
				if(err == 0) MP3GetLastFrameInfo(mp3decoder, &mp3frameinfo);//得到刚刚解码的MP3帧信息

                out_mp3decoder(ch);
                
				p_dev->depos = readptr - p_dev->debuff;

                if(err != 0)
                {
                    //设置状态标志，解码结束
                    debug_printf("MP3Decode error:%d\n", err);
#if 0
                    debug_printf("debuff %d:\n", debuffleft);
                    printf_buff(tmp_readptr, debuffleft);

                    debug_printf("frbuff %d:\n ", p_dev->frbuffleft);
                    printf_buff(p_dev->frbuff, 5*1024);
#endif
                    
                    if((p_dev->decoder_status != MUSIC_DECODER_STOP) &&
                       (p_dev->decoder_error_cnt++ >= MP3_DECODER_ERROR_MAX_CNT))
                    {
                        p_dev->decoder_status = MUSIC_DECODER_ERROR2;
                    }
                    
                    continue;
                }
                else
                {
                    //MP3GetLastFrameInfo(mp3decoder, &mp3frameinfo);//得到刚刚解码的MP3帧信息
                    if(p_dev->bitrate != mp3frameinfo.bitrate)//更新码率
                    {
                        p_dev->bitrate = mp3frameinfo.bitrate; 
                        p_dev->samplerate = mp3frameinfo.samprate;
                    }

                    pcm_fill_buff(p_dev, wbuff, mp3frameinfo.outputSamps, mp3frameinfo.nChans);//填充pcm数据   
                }

                //解码线程中时间充裕的情况下，执行该操作可提前补充空的文件缓存
                //读取文件缓存
                if(CAN_GET_FRBUFF(p_dev)) 
                {
                    frbuff2debuff(p_dev);           
                }
                
            }
        }

    }
}

/*********************************************************************************************************************************/


