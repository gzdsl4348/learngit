#include <timer.h>
#include <debug_print.h>
#include "mymalloc.h"

#include "kfifo.h"

#include "ff.h"
#include "file.h"
#include "music_decoder.h"
#include "file_operation.h"

#include "mystring.h"

extern uint8_t disk_status(uint8_t ch_num);
extern void sd_scan_music_file(uint8_t *specify_path);
extern void update_music_filelist(uint8_t path[], uint8_t is_del);
extern unsigned char mf_typetell(TCHAR *fname);
static int file_upload_start(uint8_t *fname);

extern void scan_musictosec_init();
extern void scan_musictosec_clear();

FATFS fatfs;
int my_fatfs_init()
{
    int res= f_mount(0, &fatfs);
    if(res != FR_OK)
    {
        g_fopr_mgr.sdcard_status = SD_CARD_NO_FOUND;
        scan_musictosec_clear();
        //debug_printf("f_mount error:%d\n", res);
    }
    else
    {
        
        debug_printf("start sd_scan_music_file\n");
        sd_scan_music_file(NULL);
        debug_printf("end sd_scan_music_file\n");
        g_fopr_mgr.sdcard_status = SD_CARD_OK;        
        scan_musictosec_init();
#if 0
        TCHAR name[4] = {'1','/','1',0};
        
        debug_printf("              mf_unlink %d\n", mf_unlink(name));
#endif
#if 0
        uint8_t fpct=0;
        uint8_t fexit=0;

        TCHAR fsrc[20] = {'1','/','1',0};
        TCHAR fdes[20] = {'1','/','2',0};
        debug_printf("              mf_copy %d\n", mf_copy(fsrc, fdes, &fpct, &fexit, 0, 0, F_COPY_AUTO_RENAME));
#endif
        

    }

    return res;

}

void get_sdcard_size(unsigned long *tol_mb,unsigned long *free_mb){
    *tol_mb = (fatfs.n_fatent-2)*fatfs.csize/2048;  // 族总数*族块数*块字节数 = byte， byte/1024/1024= mb
    *free_mb =  (fatfs.free_clust-2)*fatfs.csize/2048;   
}

void sdcard_hot_swap_check()
{
    if(disk_status(0) != 0)
    {
        //debug_printf("SD CARD not found\n");
        g_fopr_mgr.sdcard_status = SD_CARD_NO_FOUND;
    }
    
    if(g_fopr_mgr.sdcard_status == SD_CARD_NO_FOUND)
    {
        my_fatfs_init();
    }
}

f_opr_mgr_t g_fopr_mgr = {SD_CARD_NO_FOUND};

void fpor_mgr_init()
{
    g_fopr_mgr.sdcard_status = SD_CARD_NO_FOUND;
    //g_fopr_mgr.item[0].state = FOS_IDLE;
    g_fopr_mgr.item[0].event = FOE_IDLE;
    g_fopr_mgr.item[0].result = FOR_IDLE;    
}

void set_sdcard_status(int val)
{
    g_fopr_mgr.sdcard_status = val;
}

int get_sdcard_status()
{
    return g_fopr_mgr.sdcard_status;
}

static uint8_t g_fcopy_pct = 0;
static uint8_t g_fcopy_exit = 0;

uint8_t get_fcopy_progress()
{
    return g_fcopy_pct;
}

void fcopy_forced_stop()
{
    g_fcopy_exit = 1;
}


void fopr_handle()
{
    int error = 0;
    //if(g_fopr_mgr.sdcard_status != SD_CARD_OK) return;
    f_opr_item_t *pitem = &g_fopr_mgr.item[0];
    f_opr_music_t *p_fopr_music = &pitem->data.music;
    f_opr_file_t *p_fopr_file = &pitem->data.file;
    f_opr_upload_t *p_for_upload = &pitem->data.upload;

    switch(pitem->log_event){
        case FOR_LOGMK:{
            error = mf_open_log(p_fopr_file->fsrc,p_fopr_file->fdes);
            // 日志信息无需返回提示
            pitem->log_event = FOR_LOGIDLE;
            break; 
        }
        case FOR_LOGADD:{
            error = mf_add_loginfo(p_fopr_file->log_info,p_fopr_file->len);
            // 日志信息无需返回提示
            pitem->log_event = FOR_LOGIDLE;
            break;  
        }
    }
    
    if(pitem->result != FOR_IDLE)
        return;

    switch(pitem->event)
    {
        case FOE_IDLE:
            return;
        case FOE_MUSIC_START:
        {
            error = music_decode_start(p_fopr_music->ch, p_fopr_music->fname, p_fopr_music->foffset);
            break;
        }
        case FOE_FMKDIR:
        {
            error = mf_mkdir(p_fopr_file->fsrc);
            update_music_filelist(p_fopr_file->fsrc, 0);
            break;
        }
        case FOE_FDELETE:
        {
            error = mf_unlink(p_fopr_file->fsrc);
            update_music_filelist(p_fopr_file->fsrc, 1);
            break;
        }
        case FOE_FRENAME:
        {
            //debug_printf("\n\n rename %d\n\n",error);
            error = mf_rename(p_fopr_file->fsrc, p_fopr_file->fdes);
            update_music_filelist(p_fopr_file->fdes, 0);
            break;
        }
        case FOE_FCOPY:
        {
            error = mf_copy(p_fopr_file->fsrc, p_fopr_file->fdes, &g_fcopy_pct, &g_fcopy_exit, 0, 0, pitem->f_copy_mode);
            if(error != 0)// 操作失败, 删除目标文件
            {
                mf_unlink(p_fopr_file->fdes);
            }
            update_music_filelist(p_fopr_file->fdes, 0);
            break;
        }        
        case FOE_FMOVE:
        {
            error = mf_copy(p_fopr_file->fsrc, p_fopr_file->fdes, &g_fcopy_pct, &g_fcopy_exit, 0, 0, pitem->f_copy_mode);
            if(error != 0)// 操作失败, 删除目标文件
            {
                mf_unlink(p_fopr_file->fdes);
                update_music_filelist(p_fopr_file->fdes, 1);
            }
            else        // 操作成功, 删除源文件
            {
                error = mf_unlink(p_fopr_file->fsrc);
                update_music_filelist(p_fopr_file->fdes, 0);
                update_music_filelist(p_fopr_file->fsrc, 1);
            }
            break;
        }  
        case FOE_FUPLOAD:
        {
            if(p_for_upload->state == FOU_STATE_START)
            {
                error = file_upload_start(p_for_upload->fname);

                //debug_printf("file_upload_start %s %d\n", p_for_upload->fname, error);
                if(error == 0)
                {
                    p_for_upload->state = FOU_STATE_PUT_DATA;
                    p_for_upload->reply_data = FOU_REPLY_SUCCEED;
                }
                else
                {
                    p_for_upload->state = FOU_STATE_END;
                    p_for_upload->reply_data = FOU_REPLY_FAILED;
                }
                p_for_upload->reply_type = FOU_REPLY_START;
                return;
            }
            break;
        }         

    }

    if(error != FR_OK)
    {
        pitem->error_code = error;
        pitem->result = FOR_FAILED;
    }
    else
    {   
        pitem->error_code = error;
        pitem->result = FOR_SUCCEED;
    }
}
//需要改用double buff
kfifo_t upload_fifo;
static uint8_t upload_buff_switch = 0;

static f_opr_upload_t *gp_for_upload = &g_fopr_mgr.item[0].data.upload;

static FIL *pf_upload = NULL;

//后台调用
static void realloc_upload_buff()
{
    myfree(upload_fifo.buffer);
    myfree(pf_upload);
    upload_fifo.buffer = NULL;
    pf_upload = NULL;
}


//后台调用
static int file_upload_start(uint8_t *fname)
{
    int res = 0;
    uint8_t *buff = NULL;
    
    TCHAR *fn = mymalloc(_MAX_LFN * 2 + 1);
    pf_upload = (FIL*)mymalloc(sizeof(FIL));
    buff = (uint8_t*)mymalloc(16*1024);
    
    KFIFO_INIT(upload_fifo, buff, 16*1024);
    
    if(pf_upload==NULL || buff==NULL)
    {
        res = -1;
        goto FUN_ERROR_END;
    }
    utf8_to_unicode((const char*)fname, (char*)fn);
    if((wstrlen(fn)*2+2)>sizeof(gp_for_upload->fname))
    {
        res =0xffff;
    }
    else
    {
        wstrcpy((TCHAR*)gp_for_upload->fname, fn);
        res=f_open(pf_upload, fn, FA_WRITE|FA_CREATE_ALWAYS);
    }
    
    myfree(fn);
    if(res == FR_OK)
    {
        upload_buff_switch = 0;
        return 0;
    }
    
FUN_ERROR_END:
    realloc_upload_buff();
    myfree(fn);
    return res;
}


//前台调用控制接口
int file_upload_forced_stop()
{
    gp_for_upload->state = FOU_STATE_FORCED_STOP;
    gp_for_upload->reply_type = FOU_REPLY_IDLE;
    gp_for_upload->reply_data = FOU_REPLY_SUCCEED;
    
    return 0;
}

//后台轮询调用，处理前段数据
void upload_handle(int interval_ms)
{
    int error = 0;
    UINT bw = 0;
    //UINT btw = 0;
    
    if(g_fopr_mgr.item[0].event != FOE_FUPLOAD) return;
    
    // 写文件操作
    if(gp_for_upload->state == FOU_STATE_PUT_DATA)
    {
        // 判断fifo是否有大于8k的空间
        if(KFIFO_SIZE(upload_fifo) >= 8*1024)
        {
            // 获取fifo数据
            // 写数据到文件
            error = f_write(pf_upload, upload_fifo.buffer+upload_buff_switch*8*1024, 8*1024, &bw);
            upload_buff_switch = !upload_buff_switch;
            upload_fifo.out_index += 8*1024;
            
            //通知上层应用，已用完16*1024的buff
            if(error != 0)
            {
                f_close(pf_upload);
                realloc_upload_buff();
                
                mf_unlink(gp_for_upload->fname);
                
                update_music_filelist(gp_for_upload->fname, 1);
                
                gp_for_upload->state = FOU_STATE_END;
                
                gp_for_upload->reply_data = FOU_REPLY_FAILED;
                gp_for_upload->reply_type = FOU_REPLY_GET_DATA;
            }
            else if(upload_buff_switch == 0)
            {
                gp_for_upload->reply_data = FOU_REPLY_SUCCEED;
                gp_for_upload->reply_type = FOU_REPLY_GET_DATA;
            }
        }
        
        // 最后不够8*1024的数据
        else if(gp_for_upload->lsat_buff)
        {
            error = f_write(pf_upload, upload_fifo.buffer+upload_buff_switch*8*1024, KFIFO_SIZE(upload_fifo), &bw);
            upload_buff_switch = !upload_buff_switch;

            gp_for_upload->lsat_buff = 0;
            
            f_close(pf_upload);
            realloc_upload_buff();
                        
            //通知上层应用，结束
            gp_for_upload->state = FOU_STATE_END;
            
            if(error == 0)
            {
                gp_for_upload->reply_data = FOU_REPLY_SUCCEED;
            }
            else
            {
                mf_unlink(gp_for_upload->fname);
                
                gp_for_upload->reply_data = FOU_REPLY_FAILED;
            }
            
            update_music_filelist(gp_for_upload->fname, 0);
            
            gp_for_upload->reply_type = FOU_REPLY_END;
        }

        
    }
    // 处理前台 强制停止 指令
    else if(gp_for_upload->state == FOU_STATE_FORCED_STOP)
    {
        f_close(pf_upload);
        realloc_upload_buff();
        
        mf_unlink(gp_for_upload->fname);
        
        update_music_filelist(gp_for_upload->fname, 1);
        
        //通知上层应用,已处理
        gp_for_upload->state = FOU_STATE_END;
        gp_for_upload->reply_data = FOU_REPLY_SUCCEED;
        gp_for_upload->reply_type = FOU_REPLY_FORCD_STOP;
    }

}
