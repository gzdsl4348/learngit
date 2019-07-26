#ifndef _FILE_LIST_H_
#define _FILE_LIST_H_

#include <stdint.h>


#define DIR_NAME_SIZE   32

#define MUSIC_NAME_SIZE 64

#define PLAY_PATH_SIZE (MUSIC_NAME_SIZE*2)


#define F_DIR_TBL_BYTE_SIZE     (2*1024)
#define F_MUSIC_TBL_BYTE_SIZE   (8*1024)

// 音乐文件夹最大数
#define F_DIR_MAX_NUM           50//(F_DIR_TBL_BYTE_SIZE/sizeof(dir_info_t))
// 单个音乐文件夹内音乐文件最大数
#define F_MUSIC_MAX_NUM         100//(F_MUSIC_TBL_BYTE_SIZE/sizeof(music_info_t))


//flash的实际存储数据格式
typedef struct
{
    uint8_t sector;
    uint8_t music_num;
    uint8_t music_num_full;    
    uint16_t music_index;
    uint16_t name[DIR_NAME_SIZE/2];
}dir_info_t;


typedef struct
{
    uint32_t num;
    dir_info_t m[F_DIR_MAX_NUM+1];    
} dir_tbl_t;


typedef struct
{
    //unsigned char type;//1-mp3
    uint16_t name[MUSIC_NAME_SIZE/2];
    uint16_t totsec;
}music_info_t;


typedef struct
{
    uint32_t num;
    music_info_t m[F_MUSIC_MAX_NUM+1];//为了判断music_num_full的值
} music_tbl_t;

#endif


