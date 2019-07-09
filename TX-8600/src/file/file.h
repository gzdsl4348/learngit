#ifndef _FILE_SYSTEM1_H_
#define _FILE_SYSTEM1_H_

#include <xs1.h>
#include <platform.h>
#include <stdint.h>
#include "music_decoder.h"
#ifdef __XC__
#include "flash_user.h"
#include "sdram.h"
#endif

#define WAV_FILE_ENABLE 1

typedef enum
{
    SD_CARD_OK = 0,
    SD_CARD_NO_FOUND,
}F_SDCARD_STATUS_E;

//typedef enum
//{
//    FOS_IDLE = 0,
//    FOS_USED_FOR_FRONT,
//    FOS_USED_FOR_BACK,
//}F_OPR_STATE_E;


typedef enum
{
    FOE_IDLE = 0,
    FOE_MUSIC_START,
    
    FOE_FMKDIR,
    FOE_FDELETE,
    FOE_FRENAME,
    FOE_FCOPY,
    FOE_FMOVE,
    FOE_FDELDIR,
    
    FOE_FUPLOAD,

    FOE_FORCED_STOP,
    
}F_OPR_EVENT_E;

typedef enum
{
    FOR_LOGIDLE = 0xff,
    FOR_LOGMK = 0,
    FOR_LOGADD,
}F_LOG_RESULT_E;

typedef enum
{
    FOR_IDLE = 0xff,
    FOR_SUCCEED = 0,
    FOR_FAILED,    
    FOR_BUSY,
}F_OPR_RESULT_E;

typedef enum
{
    F_COPY_NO_COVER,    //不覆盖
    F_COPY_COVER,       //覆盖
    F_COPY_AUTO_RENAME, //不覆盖但重命名
}F_COPY_MODE_E;

typedef struct
{
    uint8_t ch;
    uint32_t foffset;
    uint8_t fname[128*2];
}f_opr_music_t;

typedef struct
{
    uint8_t fsrc[128*2];
    uint8_t fdes[128*2];
    uint8_t log_info[256*2];
    unsigned len;
}f_opr_file_t;

typedef enum
{
    FOU_STATE_IDLE = 0,
    FOU_STATE_START,
    FOU_STATE_PUT_DATA,
    FOU_STATE_FORCED_STOP,
    //FOU_STATE_WRITE_DATA_ERROR,
    FOU_STATE_END    
}F_OPR_UPLOAD_STATE_E;


typedef enum
{
    FOU_REPLY_IDLE = 0,
    FOU_REPLY_START,
    FOU_REPLY_GET_DATA,
    FOU_REPLY_END,
    FOU_REPLY_FORCD_STOP,
}F_OPR_UPLOAD_REPLY_TYPE_E;

typedef enum
{
    FOU_REPLY_SUCCEED = 0,
    FOU_REPLY_FAILED
}F_OPR_UPLOAD_REPLY_DATA_E;

typedef struct
{
    uint8_t fname[128*2];
    char state;
    char reply_type;
    char reply_data;
    char lsat_buff;
}f_opr_upload_t;


typedef struct
{
    //int id;
    //char state;
    char log_event;
    
    char event;
    char result;
    
    char f_copy_mode;
    
    int error_code;
    
    union{
        f_opr_music_t music;
        f_opr_file_t file;
        f_opr_upload_t upload;
    }data;
}f_opr_item_t;



typedef struct
{
    int sdcard_status;
    f_opr_item_t item[1];
}f_opr_mgr_t;

typedef struct 
{
   /* music_status 返回解码通道状态, is_new为通知标志位, MUSIC_DECODER_STOP与MUSIC_DECODER_START不会置位is_new
    * event/result/error_code 异步操作都会通过这几个状态返回结果, 如果result等于FOR_IDLE, event是无效的
    *
    */
    music_decoder_status_t music_status[MUSIC_CHANNEL_NUM];
    char sdcard_status;     //F_SDCARD_STATUS_E
    char event;             //F_OPR_EVENT_E
    char result;            //F_OPR_RESULT_E
    char error_code;        
    
    char uplaod_reply_type;
    char uplaod_reply_data;
}file_server_notify_data_t;

#ifdef __XC__

typedef interface file_server_if
{
    //F_COPY_MODE_E mode 
    int file_copy(uint8_t fsrc[n1], unsigned n1, uint8_t fdes[n2], unsigned n2, uint8_t mode);
    int file_move(uint8_t fsrc[n1], unsigned n1, uint8_t fdes[n2], unsigned n2,uint8_t mode);
    
    int file_mkdir(uint8_t f_name[n], unsigned n);
    int file_delete(uint8_t f_name[n], unsigned n);
    int file_rename(uint8_t fsrc[n1], unsigned n1, uint8_t fdes[n2], unsigned n2);

    int get_sdcard_size(unsigned     &tol_mb,unsigned &free_mb);
  
    // for file_copy file_move
    int get_fcopy_progress(uint8_t &progress);
    int fcopy_forced_stop();
    
    int file_upload_start(uint8_t f_name[n], unsigned n);
    int file_upload_put_data(uint8_t buff[], unsigned buff_size, uint8_t last_buff, int &fifo_size);
    int file_upload_forced_stop();
    int file_upload_get_fifo_size();
    int music_start(uint8_t ch, uint8_t f_name[n], static const unsigned n, unsigned f_offset);
    int music_stop(uint8_t ch);
    int music_stop_all();

    //
    int log_mklog(uint8_t new_fname[newlen],unsigned newlen,uint8_t old_fname[oldlen],unsigned oldlen);

    int log_loginfo_add(uint8_t log_info[len],unsigned len);

    //返回状态
    [[clears_notification]] void get_notify(file_server_notify_data_t &data);
    [[notification]] slave void notify2user();    

}file_server_if;

[[combinable]]
void file_server(server file_server_if if_fs, chanend c_faction);

void file_process(streaming chanend c_sdram, chanend c_faction);

void get_sdcard_size(unsigned     *unsafe tol_mb,unsigned *unsafe free_mb);

#endif


int my_fatfs_init();
void sdcard_hot_swap_check();

void set_sdcard_status(int val);
int get_sdcard_status();

void fpor_mgr_init();
uint8_t get_fcopy_progress();
void fcopy_forced_stop();
void fopr_handle();



int file_upload_forced_stop();
void upload_handle(int interval_ms);

extern f_opr_mgr_t g_fopr_mgr;




#endif
