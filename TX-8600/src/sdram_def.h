#ifndef _SDRAM_DEF_H_
#define _SDRAM_DEF_H_

/**
 * SDRAM 使用情况:
 * 
 *       0      - 43*4KB    存储GBK转码表
 *       44*4KB - 200*4KB    空闲
 *       200*4KB - 3MB       FLASH读写缓存
 *       用户数据存储是从FLASH_DAT_SECTOR_BASE*4KB开始
 *                       END_FL_SECTOR*4KB结束, 小于3MB
 *       
 *       3MB   - 6MB      备份缓存
 *       6MB   - 7MB      音乐文件列表
 *       7MB   - 7.5MB    MP3解码缓存
 *               
 *       7.5MB   -           用户使用临时缓存
 *
 * 注意:SDRAM的0MB-3MB来自flash
 */


#define SDRAM_FLASH_SECTOR_MAX_NUM  ((3*1024)/4)     //3MB

#define SDRAM_GBK_UNICODE_TBL_START (0)              //0MB 共172KB

#define SDRAM_DATA_BACKUP_START     (3*1024*1024/4)  //3MB, 共3MB

#define SDRAM_FILE_LIST_START       (6*1024*1024/4)  //6MB, 共512KB

#define SDRAM_FILE_LIST_SECTOR_SIZE (10*1024/4)     //10K一个sector

#define SDRAM_MP3DECODER_START      (((6*1024+512)*1024)/4) //6.5MB, 共1MB


// 批处理文件临时buff
#define USER_FILE_BAT_TMPBUF_BASE   (((7*1024+512)*1024)/4) //7.5MB, 工512KB

// 20K 存放批量操作音乐文件
// 10K 存放设备搜索信息
// 60K 存放xtcp tx rx fifo

// 批量处理音乐buff
#define USER_MUSICNAME_TMP_BASE     (USER_FILE_BAT_TMPBUF_BASE) //20K
// 设备搜索临时buff
#define USER_DIV_SEARCH_BASE        (USER_MUSICNAME_TMP_BASE+(20*1024/4)) //10K
// TX发送FIFO 
#define USER_XTCP_TXFIFO_BASE       (USER_DIV_SEARCH_BASE+(30*1024/4))  //30K
// RX发送FIFO
#define USER_XTCP_RXFIFO_BASE       (USER_XTCP_TXFIFO_BASE+(30*1024/4)) //30K

#endif
