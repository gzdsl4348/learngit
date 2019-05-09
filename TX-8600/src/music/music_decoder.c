#include <string.h>
#include <debug_print.h>
#include <timer.h>
#include "mp3dec.h"
#include "mp3common.h"
#include "coder.h"

#include "mymalloc.h"
#include "ff.h"	

#include "file.h"
#include "music_decoder.h"
#include "mp3_types.h"
#include "swlock.h"
#include "xassert.h"
#include "sdram.h"
#include "sdram_def.h"

static music_decoder_mgr_t gt_mmdm;

static swlock_t g_mp3_lock[MP3DEC_CHANNAL_NUM];

static uint8_t g_file_buff[MUSIC_FILE_BUFF_SZ];

static void read_sdram_file_buff(unsigned c_sdram, s_sdram_state *sdram_state, uint8_t ch, uint8_t flag, uint8_t buff[], uint32_t size)
{
    sdram_read(c_sdram, sdram_state, SDRAM_MP3DECODER_START+(2*ch+flag)*(MUSIC_FILE_BUFF_SZ/4), /*size/4*/size>>2, (unsigned*)&buff[0]);
    sdram_complete(c_sdram, sdram_state);  

}
static void write_sdram_file_buff(unsigned c_sdram, s_sdram_state *sdram_state, uint8_t ch, uint8_t flag, uint8_t buff[], uint32_t size)
{
    sdram_write(c_sdram, sdram_state, SDRAM_MP3DECODER_START+(2*ch+flag)*(MUSIC_FILE_BUFF_SZ/4), /*size/4*/size>>2, (unsigned*)&buff[0]);
    sdram_complete(c_sdram, sdram_state);
}

static unsigned int get_mp3_datastart(unsigned char *buff, unsigned int buff_size)
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
static unsigned char mp3_get_info(TCHAR *pname, MP3_Info *p_info)
{
    MP3FrameInfo frame_info;
    MP3_FrameXing* fxing;
    MP3_FrameVBRI* fvbri;
    FIL*fmp3;
    unsigned char *buf;
    unsigned int buf_size;
    unsigned int br;
    unsigned char res;
    int offset=0;
    unsigned int p;
    short samples_per_frame;    //一帧的采样个数
    unsigned int totframes;             //总帧数

    fmp3=mymalloc(sizeof(FIL));

    //共用5K内存
    buf = g_file_buff;
    buf_size = sizeof(g_file_buff);

    if(fmp3&&buf)//内存申请成功
    {
        f_open(fmp3,(const TCHAR*)pname,FA_READ);//打开文件
        res=f_read(fmp3,(char*)buf,buf_size,&br);
        if(res==0)//读取文件成功,开始解析ID3V2/ID3V1以及获取MP3信息
        {
            p_info->datastart = get_mp3_datastart(buf, buf_size);

            /************************************************************************/
            /*MP3解码申请内存*/
            HMP3Decoder decoder = mymalloc(sizeof(MP3DecInfo));//2032
            memset(decoder, 0, sizeof(MP3DecInfo));

            FrameHeader *fh = mymalloc(sizeof(FrameHeader));//56
            memset(fh, 0, sizeof(FrameHeader));
            ((MP3DecInfo*)decoder)->FrameHeaderPS =(void *)fh;

            /************************************************************************/

            f_lseek(fmp3, p_info->datastart);   //偏移到数据开始的地方
            f_read(fmp3, (char*)buf,buf_size,&br);    //读取5K字节mp3数据
            while(br > MAINBUF_SIZE)
            {
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
                    res = 0;
                    break;
                }
                else 
                {
                    res=0XFE; //未找到同步帧
                    offset++;
                    br -= offset;
                    memmove(buf, buf+offset, br);
                }
                /************************************************************************/
            }                
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
    int res = mp3_get_info(pname, &mp3_info);
    //debug_printf("mp3_get_info %d\n", res);
    if(res != 0)
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

#if 1
    for(uint8_t ch=0; ch<MP3DEC_CHANNAL_NUM; ch++)
        swlock_init(&g_mp3_lock[ch]);    
#endif
}
int music_decode_start(unsigned char ch, unsigned char f_name[], unsigned int f_offset)
{
    int res = 0;
    int mp3_tag_offset = 0;
    unsigned char tag[10];
    music_decoderdev_t * p_dev = NULL;

    UINT br;
    
    if(ch >= gt_mmdm.ch_num)
    {
        res = 1;
        return res;
    }
    
    p_dev = &gt_mmdm.ch_dev[ch];
    if(p_dev->file_close_flag) 
    {
        f_close(&p_dev->file);
        memset(&p_dev->file, 0, sizeof(FIL));
        p_dev->file_close_flag = 0;
    }
    
    //清空数据
    memset(p_dev, 0, sizeof(music_decoderdev_t));

    res = f_open(&p_dev->file, (const TCHAR*)f_name, FA_READ);
    if(res != FR_OK)
    {
        return res;
    }

    res = f_read(&p_dev->file, tag, 10, &br);
    mp3_tag_offset = get_mp3_datastart(tag, 10);
    if(mp3_tag_offset)
    {
        f_offset += (mp3_tag_offset+10);
    }
	

    res = f_lseek(&p_dev->file, f_offset);
    if(res != FR_OK)
    {
        return res;
    }

    debug_printf("music_decode_start [%d] succeed\n", ch);
    
    p_dev->decoder_status = MUSIC_DECODER_START;

    debug_printf("\nmusic start fun\n\n");
    
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
    if(gt_mmdm.ch_dev[ch].decoder_status != MUSIC_DECODER_STOP) 
    {
        gt_mmdm.ch_dev[ch].file_close_flag = 1;
        
        if(change_status)
            gt_mmdm.ch_dev[ch].decoder_status = MUSIC_DECODER_STOP;
    }
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
    music_decoderdev_t * p_dev = NULL;
    uint8_t need2notify = 0;
    for(uint8_t ch=0; ch<gt_mmdm.ch_num; ch++)
    {
        p_dev = &gt_mmdm.ch_dev[ch];
        // MUSIC_DECODER_START/MUSIC_DECODER_STOP 不会通知更新
        if(p_dev->decoder_status != s[ch].status &&
           p_dev->decoder_status != MUSIC_DECODER_START &&
           p_dev->decoder_status != MUSIC_DECODER_STOP)
        {
            music_decode_stop(ch, 0);
            s[ch].is_new = 1;
            need2notify = 1;

            s[ch].status = p_dev->decoder_status;
            p_dev->decoder_status= MUSIC_DECODER_STOP;//ADD:20181023
        }
        else
        {
            s[ch].status = p_dev->decoder_status;
        }
    }
    
    return need2notify;
}




void music_file_handle(STREAMING_CHANEND(c_sdram), REFERENCE_PARAM(s_sdram_state, p_sdram_state))
{
    UINT br = 0;
    uint8_t ch = 0;
    uint8_t res = 0;
    
    music_decoderdev_t * p_dev = NULL;
    
    for(ch=0; ch<gt_mmdm.ch_num; ch++)
    {
        p_dev = &gt_mmdm.ch_dev[ch];
        
        if(p_dev->decoder_status == MUSIC_DECODER_START && 
           p_dev->file_over_flag==0 &&
           (p_dev->file_buff_size[0]==0||p_dev->file_buff_size[1]==0))
        {
            res = f_read(&p_dev->file, g_file_buff, MUSIC_FILE_BUFF_SZ, &br);
            if(br == 0)
            {
                p_dev->file_over_flag = 1;
                
                debug_printf("f_read over %d %d\n", ch, res);
                
                if(res == FR_NOT_READY) return;
            }
            else
            {
                //debug_printf("f_read[%d] %d\n", ch, br);
                // 进同步锁
                swlock_acquire(&g_mp3_lock[ch]);
                if(p_dev->file_buff_size[0]==0)
                {
                    //写SDRAM
                    write_sdram_file_buff(c_sdram, p_sdram_state, ch, 0, g_file_buff, MUSIC_FILE_BUFF_SZ);
                    
                    p_dev->file_buff_size[0] = br;
                    p_dev->file_buff_offset[0] = 0;
                    
                    if(p_dev->file_buff_size[1]==0)
                        p_dev->file_buff_for_used = 0;
                    else
                        p_dev->file_buff_for_used = 1;
                }
                else if(p_dev->file_buff_size[1]==0)
                {
                    //写SDRAM
                    write_sdram_file_buff(c_sdram, p_sdram_state, ch, 1, g_file_buff, MUSIC_FILE_BUFF_SZ);
                    
                    p_dev->file_buff_size[1] = br;
                    p_dev->file_buff_offset[1] = 0;
                
                    if(p_dev->file_buff_size[0]==0)
                        p_dev->file_buff_for_used = 1;
                    else
                        p_dev->file_buff_for_used = 0;                
                }

                // 出同步锁
                swlock_release(&g_mp3_lock[ch]);
                
                
            }

        }

        
        if(p_dev->file_over_flag    == 1 &&
           p_dev->file_buff_size[0] == 0 &&
           p_dev->file_buff_size[1] == 0)
        {
            p_dev->file_over_flag = 0;
            p_dev->decoder_status = MUSIC_DECODER_FILE_END;
            debug_printf("music_file_handle MUSIC_DECODER_FILE_END [%d] %d\n", ch, p_dev->mp3_frame_num);
        }
            
        if(p_dev->file_close_flag) 
        {
            f_close(&p_dev->file);
            memset(&p_dev->file, 0, sizeof(FIL));
            p_dev->file_close_flag = 0;
        }        
        
    }
    
}

/*********************************************************************************************************************************/
#define _USE_IN_MUSIC_DECOEDR_SERVER_TASK_
uint32_t get_mp3_frame(unsigned char ch, uint32_t *length, uint32_t *frame_num, uint32_t *samplerate)
{
    if(ch >= gt_mmdm.ch_num || gt_mmdm.ch_dev[ch].mp3_frame_full==0) 
    {  
#if 1
        if(gt_mmdm.ch_dev[ch].decoder_status == MUSIC_DECODER_START)
            debug_printf("get_mp3_frame empty [%d]\n", ch);
#endif
        return 0;
    }
    //debug_printf("get_mp3_frame[%d] length:%d\n", ch, *length);

    *length     = gt_mmdm.ch_dev[ch].mp3_frame_size;
    *frame_num  = gt_mmdm.ch_dev[ch].mp3_frame_num;
    *samplerate = gt_mmdm.ch_dev[ch].samplerate;
    
    return (uint32_t)gt_mmdm.ch_dev[ch].mp3_frame;
}

void clear_mp3_frame(unsigned char ch)
{
    if(ch >= gt_mmdm.ch_num || gt_mmdm.ch_dev[ch].mp3_frame_full==0) 
    {
        return;
    }
    gt_mmdm.ch_dev[ch].mp3_frame_full = 0;
}

/*********************************************************************************************************************************/
#define _USE_IN_MUSIC_DECODER_HANDLE_TASK_

// 计算mp3_frame_size
static int get_mp3_frame_size(MP3DecInfo *mp3DecInfo)
{     
    /*
     * mpeg1.0 
     *   layer1      : 帧长= (48000*bitrate kbps)/sampling_freq + padding
     *   layer2&3    : 帧长= (144000*bitrate kbps)/sampling_freq + padding
     * mpeg2.0 
     *   layer1      : 帧长= (24000*bitrate kbps)/sampling_freq + padding
     *   layer2&3    : 帧长= (72000*bitrate kbps)/sampling_freq + padding
     */
    FrameHeader *fh = mp3DecInfo->FrameHeaderPS;
    if(fh->ver==MPEG1)
    {
        if(fh->layer == 1)
            return (48*mp3DecInfo->bitrate)/mp3DecInfo->samprate + fh->paddingBit;
        else    
            return (144*mp3DecInfo->bitrate)/mp3DecInfo->samprate + fh->paddingBit;
    }
    else
    {
        if(fh->layer == 1)
            return (24*mp3DecInfo->bitrate)/mp3DecInfo->samprate + fh->paddingBit;
        else
            return (72*mp3DecInfo->bitrate)/mp3DecInfo->samprate + fh->paddingBit;
    }
}

static void set_file_buff_offset(music_decoderdev_t * p_dev, int offset)
{
    if((p_dev->file_buff_offset[p_dev->file_buff_for_used]+offset) < MUSIC_FILE_BUFF_SZ)
    {
        p_dev->file_buff_offset[p_dev->file_buff_for_used] += offset;
    }
    else
    {
        //debug_printf("file_buff switch %d %d\n", offset, p_dev->file_buff_offset[p_dev->file_buff_for_used]);
        
        offset = offset - (p_dev->file_buff_size[p_dev->file_buff_for_used] - p_dev->file_buff_offset[p_dev->file_buff_for_used]);
        
        p_dev->file_buff_size[p_dev->file_buff_for_used] = 0;
        p_dev->file_buff_offset[p_dev->file_buff_for_used] = 0;
        
        if(p_dev->file_buff_size[!p_dev->file_buff_for_used])
        {
            p_dev->file_buff_offset[!p_dev->file_buff_for_used] = offset;
            p_dev->file_buff_for_used = !p_dev->file_buff_for_used;
        }
    }
}
void music_decoder(STREAMING_CHANEND(c_sdram))
{
    music_decoderdev_t * p_dev = NULL;
    uint8_t ch = 0;
    uint8_t file_buff[MUSIC_FILE_BUFF_SZ*2];
    uint32_t file_buff_left = 0;
    uint8_t  *readptr;
    int offset = 0;

    MP3DecInfo mp3decinfo;
    FrameHeader fh;
   
    memset(&mp3decinfo, 0, sizeof(MP3DecInfo));
    memset(&fh, 0, sizeof(FrameHeader));
    mp3decinfo.FrameHeaderPS =(void *)&fh;

    s_sdram_state sdram_state;
    sdram_init_state(c_sdram, &sdram_state);

    while(1)
    {
        //TUDO:应该每个通道需要计时，哪个通道的计时最少，优先解码哪通道
        for(ch=0; ch<MUSIC_CHANNEL_NUM; ch++)
        {
            p_dev = &gt_mmdm.ch_dev[ch];

            if(p_dev->decoder_status != MUSIC_DECODER_START) continue;

            //mp3 frame需要填充时才调用
            if(p_dev->mp3_frame_full) continue;

            if(p_dev->file_buff_size[0]==0 && p_dev->file_buff_size[1]==0) continue;

            // 进出同步锁间操作, 耗时0.1ms
            // 进同步锁
            swlock_acquire(&g_mp3_lock[ch]);

            //读SDRAN MUSIC_FILE_BUFF_SZ*2
            read_sdram_file_buff(c_sdram, &sdram_state, ch, p_dev->file_buff_for_used, file_buff, MUSIC_FILE_BUFF_SZ);
            file_buff_left = p_dev->file_buff_size[p_dev->file_buff_for_used]-p_dev->file_buff_offset[p_dev->file_buff_for_used];
            
            if(p_dev->file_buff_for_used==1 && p_dev->file_buff_size[0])
            {
                read_sdram_file_buff(c_sdram, &sdram_state, ch, 0, file_buff+MUSIC_FILE_BUFF_SZ, MUSIC_FILE_BUFF_SZ);
                file_buff_left += (p_dev->file_buff_size[0]-p_dev->file_buff_offset[0]);
            }
            else if(p_dev->file_buff_for_used==0 && p_dev->file_buff_size[1])
            {
                read_sdram_file_buff(c_sdram, &sdram_state, ch, 1, file_buff+MUSIC_FILE_BUFF_SZ, MUSIC_FILE_BUFF_SZ);
                file_buff_left += (p_dev->file_buff_size[1]-p_dev->file_buff_offset[1]);
            }
#if 0
            if(p_dev->mp3_frame_num>=30 && p_dev->mp3_frame_num<33)
            {
                debug_printf("[%d] used:%d [%d %d] [%d %d] left:%d\n", 
                             p_dev->mp3_frame_num, p_dev->file_buff_for_used,
                             p_dev->file_buff_size[0], p_dev->file_buff_offset[0],
                             p_dev->file_buff_size[1], p_dev->file_buff_offset[1],
                             file_buff_left);
            }
#endif
            readptr = file_buff + p_dev->file_buff_offset[p_dev->file_buff_for_used];

            offset = MP3FindSyncWord(readptr, file_buff_left);//在readptr位置,开始查找同步字符
            if(offset < 0)//没有找到同步字符,跳出帧解码循环
			{
                //没找到帧同步字符
                //TUDO:设置状态标志，可以找下一帧
                debug_printf("MP3FindSyncWord failed [%d]\n", ch);
                debug_printf("[%d] used:%d [%d %d] [%d %d] left:%d\n", 
                             p_dev->mp3_frame_num, p_dev->file_buff_for_used,
                             p_dev->file_buff_size[0], p_dev->file_buff_offset[0],
                             p_dev->file_buff_size[1], p_dev->file_buff_offset[1],
                             file_buff_left);

                if(p_dev->file_over_flag)
                {
                    p_dev->file_over_flag = 0;
                    p_dev->decoder_status = MUSIC_DECODER_FILE_END;
                }
                else if(p_dev->decoder_status != MUSIC_DECODER_STOP && p_dev->decoder_error_cnt++ > MP3_DECODER_ERROR_MAX_CNT)
                {
                    p_dev->decoder_status = MUSIC_DECODER_ERROR1;  
                }
                else
                {
                    // 处理file_buff_offset数据
                    set_file_buff_offset(p_dev, file_buff_left/2);
                }
                
			}
            else//找到同步字符了
			{
                readptr += offset;//MP3读指针偏移到同步字符处.
                
                if((UnpackFrameHeader(&mp3decinfo, readptr)==-1) || (mp3decinfo.layer!=3))
                {
                    debug_printf("UnpackFrameHeader failed [%d] %d\n", ch, mp3decinfo.layer);
                    debug_printf("file_buff_offset:%d offset:%d frame_num:%d %d\n", p_dev->file_buff_offset[p_dev->file_buff_for_used], offset, p_dev->mp3_frame_num, p_dev->file_over_flag);
                                            
                    if(p_dev->file_over_flag)
                    {
                        p_dev->file_over_flag = 0;
                        p_dev->decoder_status = MUSIC_DECODER_FILE_END;
                    }
                    else if(p_dev->decoder_status != MUSIC_DECODER_STOP && p_dev->decoder_error_cnt++ > MP3_DECODER_ERROR_MAX_CNT)
                    {
                        p_dev->decoder_status = MUSIC_DECODER_ERROR2;  
                        debug_printf("MP3 MUSIC_DECODER_ERROR2\n");
                    }
                    else
                    {
                        // 处理file_buff_offset数据
                        set_file_buff_offset(p_dev, offset+2);
                    }
                }
                else
                {
                    p_dev->mp3_frame_size = get_mp3_frame_size(&mp3decinfo);
                    if(file_buff_left >= p_dev->mp3_frame_size)
                    {
                        offset += p_dev->mp3_frame_size;
                        
                        // 处理file_buff_offset数据
                        set_file_buff_offset(p_dev, offset);
                        
                        if(p_dev->bitrate != mp3decinfo.bitrate)//更新码率
                        {
                            p_dev->bitrate = mp3decinfo.bitrate; 
                            p_dev->samplerate = mp3decinfo.samprate;
                        }
                        
                        // put mp3 frame buff
                        memcpy(p_dev->mp3_frame, readptr, p_dev->mp3_frame_size);
                        p_dev->mp3_frame_num++;
                        p_dev->mp3_frame_full = 1;
                    }
                }
            }
            // 出同步锁
            swlock_release(&g_mp3_lock[ch]);
        }

    }
}

/*********************************************************************************************************************************/


