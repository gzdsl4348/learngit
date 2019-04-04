#include <string.h>
#include <debug_print.h>
#include "mymalloc.h"
#include "ff.h"
#include "mp3dec.h"
#include "file_list.h"
#include "mystring.h"

#define FILE_LIST_DEBUG_ENBLE 1

#if FILE_LIST_DEBUG_ENBLE
#define DBG_PRINTF(...) debug_printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#endif

#define F_START_MUSIC_SECTOR    1


extern unsigned int get_mp3_totsec(TCHAR *pname);

extern void fl_erase_flielist(int secoter_index);
extern void fl_read_flielist(int secoter_index, unsigned char buff[], int br);
extern void fl_write_flielist(int secoter_index, const unsigned char buff[], int bw);


TCHAR ROOT_PATH[] = {0, 0};
//报告文件的类型
//fname:文件名
//返回值:0XFF,表示无法识别的文件类型编号.
//       1, mp3文件
unsigned char mf_typetell(TCHAR *fname)
{
    const TCHAR mp31[] = {'m','p','3', 0};
    const TCHAR mp32[] = {'M','P','3', 0};
    TCHAR *attr = 0;//后缀名
    unsigned char i=0;
    while(i<250)
    {
        i++;
        if(*fname=='\0')break;//偏移到了最后了.
        fname++;
    }
    if(i==250)return 0XFF;//错误的字符串.
    for(i=0; i<5; i++) //得到后缀名
    {
        fname--;
        if(*fname=='.')
        {
            fname++;
            attr=fname;
            break;
        }
    }

    if(attr == 0) return 0xFF;

    if(wstrcmp(mp31, attr)==0 ||
       wstrcmp(mp32, attr)==0)
        return 1;

    return 0XFF;//没找到
}
void printfstr(TCHAR *str)
{

    if(sizeof(TCHAR)==1)
    {
        DBG_PRINTF("%s", str);
        return;
    }
    while(*str)
    {
        if(*str>0xff)
        {
            DBG_PRINTF("%c%c", (*str)>>8, *str);
        }
        else
        {
            DBG_PRINTF("%c", *str);
        }
        str++;
    }
}

//遍历文件
//used ram:1536 + 2148(mp3_get_info) = 3684
//path:路径
//mark:0,查找文件夹;1,查找目标文件;2,新增目标文件,在旧的基础上添加新的目标文件
//buff:存储列表的buff
//buff_size:buff的大小
//num:返回文件数目
//返回值:执行结果
char mf_scan_files(TCHAR *path, char mark, unsigned char *buff, int buff_size, int *num, uint8_t *buff_full)
{
    int i = 0;
    FRESULT res;
    unsigned char type;
    unsigned int offset = 0;
    dir_info_t *pt_di = NULL;
    music_info_t *pt_mi = NULL;

    TCHAR *fn;   /* This function is assuming non-Unicode cfg. */

    DIR *dir = mymalloc(sizeof(DIR));
    FILINFO *fno = mymalloc(sizeof(FILINFO));

    TCHAR *music_path = mymalloc(160);

#if _USE_LFN
    TCHAR *lfname = mymalloc(_MAX_LFN * 2 + 1);
    fno->lfsize = _MAX_LFN * 2 + 1;
    fno->lfname = &lfname[0];
#endif
    if(dir==NULL || fno==NULL || music_path==NULL || lfname==NULL)
    {
        debug_printf("mf_scan_files malloc failed\n");
#if _USE_LFN
        myfree(lfname);
#endif
        myfree(music_path);
        myfree(dir);
        myfree(fno);
        return 0xff;        
    }
    if(mark == 0 || mark == 1)
    {
        *num = 0;
        memset(buff, 0, buff_size);
    }
    else if(mark == 2)
    {
        offset = sizeof(music_info_t)*(*num);
        memset(buff+offset, 0, buff_size-offset);
    }
    
    res = f_opendir(dir,(const TCHAR*)path); //打开一个目录
    if (res == FR_OK)
    {
        if(buff_full) *buff_full = 0;

        while(1)
        {
            res = f_readdir(dir, fno);                   //读取目录下的一个文件
            if (res != FR_OK || fno->fname[0] == 0) break;  //错误了/到末尾了,退出

#if _USE_LFN
            fn = *fno->lfname ? fno->lfname : fno->fname;
#else
            fn = fno->fname;
#endif                                                /* It is a file. */
            //debug_printf("  + ");
            //printfstr(fn);
            //debug_printf("  :%d %d\n", (fno->fattrib&(AM_DIR)), (fno->fattrib&(AM_HID)));
            if(fno->fattrib&(AM_HID)) continue;//跳过隐藏文件

            if((fno->fattrib&(AM_DIR))&&mark==0)//是一个文件夹且mark=0
            {

                if(wstrlen(fn) > (DIR_NAME_SIZE/2)) continue;//限制文件名长度

                if(offset+sizeof(dir_info_t) < buff_size)
                {
                    pt_di = (dir_info_t*)&buff[offset];
                    
                    memset(pt_di->name, 0, sizeof(pt_di->name));//APP端需要在截止符后清空
                    wstrcpy(pt_di->name, fn);
                    
                    offset += sizeof(dir_info_t);
                    (*num)++;
                }
                else
                {
                    debug_printf(" buff_size over\n");
                    break;
                }

            }
            else if((fno->fattrib&(AM_ARC))&&mark==1) //是一个归档文件且mark=1
            {
                if(wstrlen(fn) > (MUSIC_NAME_SIZE/2)) continue;//限制文件名长度

                type=mf_typetell(fn);    //获得类型

                if(type == 1)//mp3文件
                {
                    if(offset+sizeof(music_info_t) < buff_size)
                    {
                        TCHAR g[] = {'/',0};
                        pt_mi = (music_info_t*)&buff[offset];
                        //pt_mi->type = type;

                        wstrcpy(music_path, (const TCHAR*)path);
                        wstrcat(music_path, g);
                        wstrcat(music_path, (const TCHAR*)fn);
                        pt_mi->totsec = get_mp3_totsec((TCHAR*)music_path);

                        if(pt_mi->totsec == 0) continue;

                        memset(pt_mi->name, 0, sizeof(pt_mi->name));//APP端需要在截止符后清空
                        wstrcpy(pt_mi->name, fn);
                        offset += sizeof(music_info_t);
                        (*num)++;
                    }
                    else
                    {
                        if(buff_full) *buff_full = 1;
                        debug_printf(" buff_size over\n");
                        break;
                    }
                }
                else 
                {
                    continue;  //不需要的类型
                }
            }
            else if((fno->fattrib&(AM_ARC))&&mark==2) //是一个归档文件且mark=2
            {
                if(wstrlen(fn) > (MUSIC_NAME_SIZE/2)) continue;//限制文件名长度

                type=mf_typetell(fn);    //获得类型

                if(type == 1)//mp3文件
                {
                    if(offset+sizeof(music_info_t) < buff_size)
                    {
                        //排除相同文件名称
                        for(i=0; i<(*num); i++)
                        {
                            if(wstrcmp(fn, ((music_info_t*)&buff[0])[i].name) == 0) break;
                        }
                        if(i != (*num)) continue;
                        
                        TCHAR g[] = {'/',0};
                        pt_mi = (music_info_t*)&buff[offset];
                        //pt_mi->type = type;
                        
                        wstrcpy(music_path, (const TCHAR*)path);
                        wstrcat(music_path, g);
                        wstrcat(music_path, (const TCHAR*)fn);
                        pt_mi->totsec = get_mp3_totsec((TCHAR*)music_path);

                        if(pt_mi->totsec == 0) continue;
                        
                        memset(pt_mi->name, 0, sizeof(pt_mi->name));//APP端需要在截止符后清空
                        wstrcpy(pt_mi->name, fn);
                        offset += sizeof(music_info_t);
                        (*num)++;
                    }
                    else
                    {
                        if(buff_full) *buff_full = 1;
                        debug_printf(" buff_size over\n");
                        break;
                    }
                }
                else 
                {
                    continue;  //不需要的类型
                }
            }            
            else 
            {
                continue;      //继续找下一个
            }

        }
    }
#if _USE_LFN
    myfree(lfname);
#endif
    myfree(music_path);
    myfree(dir);
    myfree(fno);
    return res;
}


void printf_dir_info(dir_info_t tbl[], int num)
{
    for(int i=0; i<num; i++)
    {
        DBG_PRINTF("-");
        printfstr(tbl[i].name);
        DBG_PRINTF("-%d-%d\n", tbl[i].sector, tbl[i].music_num);
    }
}


void printf_music_info(music_info_t tbl[], int num)
{
    for(int i=0; i<num; i++)
    {
        DBG_PRINTF("  ");
        printfstr(tbl[i].name);
        DBG_PRINTF(":%d\n", tbl[i].totsec);
    }
    DBG_PRINTF(" \n");
}

void debug_clean_flash_filelist()
{
    fl_erase_flielist(0);
}

void debug_display_sdram_filelist(uint8_t *dir_tbl, uint8_t *music_tbl)
{
    unsigned int dir_num = 0;
    unsigned int music_num = 0;

    dir_info_t *pt_fldi = (dir_info_t *)(dir_tbl+sizeof(int));

    fl_read_flielist(0, dir_tbl, F_DIR_TBL_BYTE_SIZE);
    dir_num = *(unsigned int*)dir_tbl;
    if(dir_num==0 || dir_num>F_DIR_MAX_NUM)
    {
        DBG_PRINTF("fl_filelist don`t init\n");
        return;
    }
    DBG_PRINTF("fl_filelist :%d %d \n", dir_num, mem_perused());
    for(int i=0; i<dir_num; i++)
    {
        DBG_PRINTF("[%d] dir:", pt_fldi[i].sector);
        printfstr(pt_fldi[i].name);
        DBG_PRINTF(" \n");
        fl_read_flielist(pt_fldi[i].sector, music_tbl, F_MUSIC_TBL_BYTE_SIZE);

        music_num = *(unsigned int*)music_tbl;
        if(music_num>F_MUSIC_MAX_NUM || music_num==0)
        {
            DBG_PRINTF("this file is empty\n\n");
            continue;
        }
        printf_music_info((music_info_t*)(music_tbl+sizeof(int)), music_num);
    }
}

void sd_scan_test()
{
    int num = 0;

    uint8_t *tbl = mymalloc(F_DIR_TBL_BYTE_SIZE);

    mf_scan_files(ROOT_PATH, 0, tbl, F_DIR_TBL_BYTE_SIZE, &num, NULL);

    printf_dir_info((dir_info_t*)tbl, num);

    memset(tbl, 0, F_DIR_TBL_BYTE_SIZE);

    mf_scan_files(ROOT_PATH, 1, tbl, F_DIR_TBL_BYTE_SIZE, &num, NULL);

    printf_music_info((music_info_t*)tbl, num);

    myfree(tbl);
}


//index 0 不可用
//TUDO:容易出现bug，并容易误解
#define SECTOR_RECORD_INIT(rceord)   memset(rceord, 0, F_DIR_MAX_NUM+1);rceord[0] = 1;
#define SET_SECTOR_RECORD(record, i)  record[i] = 1;

uint8_t find_idle_sector(uint8_t info[], int max_num)
{

    for(int i=0; i<max_num+1; i++)
    {
        if(info[i] == 0) return i;
    }

    return 0xFF;
}

static void debug_dir_list(char tag[] ,dir_tbl_t *dir_tbl)
{
    int i, j, stop;
    for(i=0; i<dir_tbl->num; i++)
    {
        stop = 0;
        for(j=0; j<(sizeof(dir_tbl->m[0].name)/sizeof(dir_tbl->m[0].name[0])); j++)
        {
            if(dir_tbl->m[i].name[j] == 0) stop = 1;
            if(stop && dir_tbl->m[i].name[j]) debug_printf("%s dir_info [%d] error\n", tag, i);
        }
    } 
}
//清理名称字符串截止符后的字符串
static void clean_dir_info_name(dir_tbl_t *dir_tbl)
{
    int i, j, stop;
    for(i=0; i<dir_tbl->num; i++)
    {
        stop = 0;
        for(j=0; j<(sizeof(dir_tbl->m[0].name)/sizeof(dir_tbl->m[0].name[0])); j++)
        {
            if(dir_tbl->m[i].name[j] == 0) stop = 1;
            if(stop && dir_tbl->m[i].name[j]) dir_tbl->m[i].name[j]=0;
        }
    }
}
// 0-succeed 1-failed
static int scan_music_filelist(TCHAR *path, dir_info_t *dir_info, music_tbl_t *music_tbl)
{
    int music_num = 0;
    int res = 0;
    //搜索对应文件夹的音乐列表
    res = mf_scan_files(path, 1, (uint8_t*)&music_tbl->m[0], sizeof(music_tbl->m), &music_num, &dir_info->music_num_full);
    if(res != 0)
    {
        return 1;
    }
    if(music_num > F_MUSIC_MAX_NUM)
    {
        music_num = F_MUSIC_MAX_NUM;
        dir_info->music_num_full = 1;
    }
    dir_info->music_num = music_num;
    music_tbl->num = music_num;

    return 0;
}
/*
flash
sector[0]:文件夹数(int)+文件夹列表(dir_info_t列表)
sector[1]:音乐数(int)+音乐列表(music_info_列表)
sector[2]:音乐数(int)+音乐列表(music_info_列表)
    .
    .
    .
sector[30]

1.如果文件夹里音乐文件数为0，也会写进flash记录

*/
//used ram:8*1024+2*2*1024+34 + 3684(mf_scan_files) < 16*1024 
void sd_scan_music_file(uint8_t *specify_path)
{
    int i, j, res;
    int dir_num = 0;
    
    uint8_t del_dir_num = 0;
    uint8_t add_dir_num = 0;

    dir_tbl_t *dir_tbl_sdram = mymalloc(F_DIR_TBL_BYTE_SIZE);
    dir_tbl_t *dir_tbl       = mymalloc(F_DIR_TBL_BYTE_SIZE);
    music_tbl_t *music_tbl   = mymalloc(F_MUSIC_TBL_BYTE_SIZE);
    uint8_t *sector_record   = mymalloc(F_DIR_MAX_NUM+1);
    
    if(dir_tbl_sdram==NULL || dir_tbl==NULL || music_tbl==NULL || sector_record==NULL)
    {
        debug_printf("sd_scan_music_file malloc failed\n");
        goto FUN_END;
    }

    SECTOR_RECORD_INIT(sector_record);

    //读sdram音乐文件夹列表sector0
    fl_read_flielist(0, (uint8_t*)dir_tbl_sdram, F_DIR_TBL_BYTE_SIZE);
    
    //读SD卡音乐文件夹列表
    res = mf_scan_files(ROOT_PATH, 0, (uint8_t*)dir_tbl->m, sizeof(dir_tbl->m), &dir_num, NULL);
    if(res != 0)
    {
        goto FUN_END;
    }

    //sd卡为空
    if(dir_num == 0)
    {
        //删除flash的文件夹列表, 擦除sector
        DBG_PRINTF("sd card is empty\n");
        memset(dir_tbl, 0, F_DIR_TBL_BYTE_SIZE);
        fl_write_flielist(0, (uint8_t*)dir_tbl, F_DIR_TBL_BYTE_SIZE);
        goto FUN_END;
    }
    
    debug_printf("p_fld_num 0x%x 0x%x\n\n", dir_tbl_sdram->num, dir_num);

    dir_num = (F_DIR_MAX_NUM>dir_num) ? dir_num : F_DIR_MAX_NUM;
    dir_tbl->num = dir_num;
    dir_tbl_sdram->num = (F_DIR_MAX_NUM>dir_tbl_sdram->num) ? dir_tbl_sdram->num : 0;
    
    clean_dir_info_name(dir_tbl_sdram);
    clean_dir_info_name(dir_tbl);
    //debug_dir_list("sdram", dir_tbl_sdram);
    //debug_dir_list("sdcard", dir_tbl);
    
    //sdram列表为空，第一次初始化
    if(dir_tbl_sdram->num==0 || dir_tbl_sdram->num>F_DIR_MAX_NUM)
    {
        dir_tbl->num = dir_num;

        for(i=0; i<dir_num; i++)
        {
            dir_tbl->m[i].sector = i+F_START_MUSIC_SECTOR;//sector[0]存储文件夹列表
            //dir_tbl->m[i].user_index = 0;
            //搜索对应文件夹的音乐列表
            if(scan_music_filelist(dir_tbl->m[i].name, &dir_tbl->m[i], music_tbl)!=0) goto FUN_END;
            
            //music_tbl写入sdram sector[dir_tbl->m[i].sector]
            fl_write_flielist(dir_tbl->m[i].sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);
        }

        //dir_tbl写入sdram sector[0]
        fl_write_flielist(0, (uint8_t*)dir_tbl, F_DIR_TBL_BYTE_SIZE);

#if FILE_LIST_DEBUG_ENBLE
        //debug_display_sdram_filelist((uint8_t*)dir_tbl, (uint8_t*)music_tbl);
#endif

        goto FUN_END;
    }

    
    DBG_PRINTF("start cmp dir list\n\n");
    
    //比较音乐文件夹列表，如有删除则删除对应sector列表、新增则添加到空的sector列表
    add_dir_num = dir_num;
    for(i=0; i<dir_tbl_sdram->num; i++)
    {
        char is_found_dir = 0;
        //dir_tbl列表 去除已存在的，剩下是新增的
        for(j=0; j<dir_num; j++)
        {
            if(wstrcmp(dir_tbl_sdram->m[i].name, dir_tbl->m[j].name) == 0)
            {
                is_found_dir = 1;
                memset(&dir_tbl->m[j], 0, sizeof(dir_info_t));
                SET_SECTOR_RECORD(sector_record, dir_tbl_sdram->m[i].sector);//标记已使用的sector
                add_dir_num--;//最后剩余新增的文件夹数
                break;
            }
        }

        //删除不存在的,空余的sector可以不处理
        if(is_found_dir == 0)
        {
            DBG_PRINTF("del dir:%s\n", dir_tbl_sdram->m[i].name);
            memset(&dir_tbl_sdram->m[i], 0, sizeof(dir_info_t));
            del_dir_num++;
        }
    }
    
    DBG_PRINTF("del_dir_num:%d add_dir_num:%d\n\n", del_dir_num, add_dir_num);

    //整理好dir_tbl_sdram删除的音乐文件夹列表
    if(del_dir_num)
    {
        int right = 0;
        for(int left=0; left<dir_tbl_sdram->num; left++)
        {
            if(dir_tbl_sdram->m[left].name[0]==0)//找到空的信息块
            {
                if(right==0) right = left+1;

                while(right < dir_tbl_sdram->num)
                {
                    if(dir_tbl_sdram->m[right].name[0] != 0)//找到信息块并向前填充
                    {
                        dir_tbl_sdram->m[left] = dir_tbl_sdram->m[right];
                        memset(&dir_tbl_sdram->m[right], 0, sizeof(dir_info_t));
                        break;
                    }
                    right++;
                }
            }
            //标记已使用的sector,如果为空，则会标记sector0
            SET_SECTOR_RECORD(sector_record, dir_tbl_sdram->m[left].sector);
        }
        dir_tbl_sdram->num -= del_dir_num;
    }
    
    //找到新增的音乐文件夹，找到空余的sector
    if(add_dir_num)
    {
        int add_num = add_dir_num;
        int index = 0;

        for(i=dir_tbl_sdram->num; i<F_DIR_MAX_NUM && add_num; i++)
        {
            //获取新增的信息块
            while(index < dir_num)
            {
                if(dir_tbl->m[index].name[0] != 0)
                {
                    dir_tbl_sdram->m[i] = dir_tbl->m[index];

                    //找到空闲的index
                    dir_tbl_sdram->m[i].sector = find_idle_sector(sector_record, F_DIR_MAX_NUM);
                    SET_SECTOR_RECORD(sector_record, dir_tbl_sdram->m[i].sector);
                    DBG_PRINTF("scan_music_filelist add in %d %d %d %d : ", mem_perused(), (uint32_t)music_tbl, (uint32_t)music_tbl->m, sizeof(music_tbl->m));
                    printfstr(dir_tbl_sdram->m[i].name);
                    DBG_PRINTF("\n");
                    //读SD卡对应新增文件夹的音乐文件
                    if(scan_music_filelist(dir_tbl_sdram->m[i].name, &dir_tbl_sdram->m[i], music_tbl)!=0) goto FUN_END;
                    DBG_PRINTF("scan_music_filelist add out\n");
                    //更新到sdram, 写music_tbl到sector[dir_tbl_sdram->m[i].sector]
                    fl_write_flielist(dir_tbl_sdram->m[i].sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);

                    add_num--;
                    index++;
                    break;

                }
                index++;
            }
        }

        dir_tbl_sdram->num += add_dir_num;

    }

    if(specify_path)
    {
        // 处理单个音乐文件夹, 处理剩余音乐文件夹
        for(i=0; i<(dir_tbl_sdram->num-add_dir_num); i++)
        {
            if(wstrcmp(dir_tbl_sdram->m[i].name, (wchar*)specify_path) != 0) continue;
            
            //读SD卡对应文件夹音乐列表
            if(scan_music_filelist(dir_tbl_sdram->m[i].name, &dir_tbl_sdram->m[i], music_tbl)!=0) goto FUN_END;            
            
            //更新到sdram，写music_tbl到sector[dir_tbl_sdram->m[i].sector]
            fl_write_flielist(dir_tbl_sdram->m[i].sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);
            DBG_PRINTF("update specify dir:%s sector:%d\n\n", dir_tbl_sdram->m[i].name, dir_tbl_sdram->m[i].sector);
            break;
        }
    }
    else
    {
        // 处理完增删的音乐文件夹, 处理剩余音乐文件夹
        for(i=0; i<(dir_tbl_sdram->num-add_dir_num); i++)
        {
            //读SD卡对应文件夹音乐列表
            if(scan_music_filelist(dir_tbl_sdram->m[i].name, &dir_tbl_sdram->m[i], music_tbl)!=0) goto FUN_END;            
            
            //更新到sdram，写music_tbl到sector[dir_tbl_sdram->m[i].sector]
            fl_write_flielist(dir_tbl_sdram->m[i].sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);
            DBG_PRINTF("update dir:%s sector:%d\n\n", dir_tbl_sdram->m[i].name, dir_tbl_sdram->m[i].sector);
        }
    }


    //结束:dir_tbl_sdram写进sdram
    DBG_PRINTF("update dir_tbl_sdram\n");
    fl_write_flielist(0, (uint8_t*)dir_tbl_sdram, F_DIR_TBL_BYTE_SIZE);
    
#if FILE_LIST_DEBUG_ENBLE
    //debug_display_sdram_filelist((uint8_t*)dir_tbl, (uint8_t*)music_tbl);
#endif

FUN_END:
    myfree(dir_tbl_sdram);
    myfree(dir_tbl);
    myfree(music_tbl);
    myfree(sector_record);
}



static void find_path(TCHAR *dst, const TCHAR *src, int level)
{
	TCHAR *d;
    if (*src == '/' || *src == '\\')  /* Strip heading separator if exist */
      src++;
    do
    {
		d = dst;
        while(*src!=0 && *src!='/' && *src!='\\') *d++ = *src++;
        *d = 0;
        level--;
		src++;
    }while(level>0);
}
/*
* 二层路径: 用于复制/移动1-2/删除文件/上传文件
* 一层路径: 用于新建音乐文件夹(空文件夹)、删除音乐文件夹、重命名音乐文件夹
*               需要检测原有音乐文件列表, 及需要生成SD卡音乐文件列表
*/
void update_music_filelist(uint8_t path[], uint8_t is_del)
{
    int i = 0;
    
    int dir_sector = -1;
    int dir_index = -1;
    int music_index = -1;
    int music_totsec = 0;
    
    uint8_t *dir_name = mymalloc(255*2);
    uint8_t *music_name = mymalloc(255*2);
    
    dir_tbl_t *dir_tbl_sdram = mymalloc(F_DIR_TBL_BYTE_SIZE);
    music_tbl_t *music_tbl = mymalloc(F_MUSIC_TBL_BYTE_SIZE);

    if(dir_tbl_sdram==NULL || music_tbl==NULL || dir_name==NULL || music_name==NULL)
    {
        debug_printf("update_music_filelist malloc failed\n");
        goto FUN_END;
    }
        
    find_path((TCHAR*)dir_name, (TCHAR*)path, 1);
    find_path((TCHAR*)music_name, (TCHAR*)path, 2);

    if(wstrlen((TCHAR*)dir_name) != 0) i++;
    if(wstrlen((TCHAR*)music_name) != 0) i++;
    debug_printf("update_music_filelist %d level\n", i);
    if(i == 0)          // 错误路径
    {
        goto FUN_END;
    }
    else if(i == 1)     // 一层路径
    {
        goto filelist_layer_1;
    }
    else                // 二层路径
    {
        goto filelist_layer_2;
    }
    
filelist_layer_1:   //一层路径处理逻辑        
    myfree(dir_tbl_sdram);
    myfree(music_tbl);
    dir_tbl_sdram = NULL;
    music_tbl = NULL;
    sd_scan_music_file(dir_name);
    goto FUN_END;    
    
filelist_layer_2:   //二层路径处理逻辑  
    
    //读取sdram音乐文件夹列表
    fl_read_flielist(0, (uint8_t*)dir_tbl_sdram, F_DIR_TBL_BYTE_SIZE);
    
    //查找对应音乐文件的音乐文件夹sector
    for(i=0; i<dir_tbl_sdram->num; i++)
    {
        if(wstrcmp((TCHAR*)dir_tbl_sdram->m[i].name, (TCHAR*)dir_name) == 0)
        {
            dir_sector = dir_tbl_sdram->m[i].sector;
            dir_index = i;
            break;
        }
    }
    if(dir_sector == -1) goto FUN_END;
    
    //读取对应音乐文件夹的音乐文件夹列表
    fl_read_flielist(dir_sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);
    
    DBG_PRINTF("update_music_filelist sector[%d] music_num[%d]\n", dir_sector, *(unsigned int*)music_tbl);
    
    //查找对应文件的音乐文件index
    for(i=0; i<music_tbl->num; i++)
    {
        if(wstrcmp((TCHAR*)music_tbl->m[i].name, (TCHAR*)music_name) == 0)
        {
            music_index = i;
            break;
        }
    }
    
    music_totsec = get_mp3_totsec((TCHAR*)path);
    DBG_PRINTF("music_totsec[%d] music_index[%d]\n", music_totsec, music_index);
    //音乐列表中找不到该音乐信息
    if(music_index == -1)
    {
        //新增一首音乐信息
        if(music_totsec) 
        {
            i = music_tbl->num;
            if(music_tbl->num < F_MUSIC_MAX_NUM)
            {
                music_tbl->m[i].totsec = music_totsec;
                memset(music_tbl->m[i].name, 0, sizeof(music_tbl->m[i].name));
                wstrcpy((TCHAR*)music_tbl->m[i].name, (TCHAR*)music_name);
                if(++music_tbl->num)
                    dir_tbl_sdram->m[dir_index].music_num_full = 0;
                dir_tbl_sdram->m[dir_index].music_num = music_tbl->num;
                DBG_PRINTF("  add new music_totsec dir_music_num:%d\n", dir_tbl_sdram->m[dir_index].music_num);
            }
            else
            {
                dir_tbl_sdram->m[dir_index].music_num_full = 1;
            }
        }
    }
    else
    {
        //修改已有音乐信息
        if(music_totsec) 
        {
            music_tbl->m[music_index].totsec = music_totsec;
            DBG_PRINTF("  update music_info_t music_totsec:%d\n", music_totsec);
        }
        //删除已有音乐信息
        else
        {
            i = music_tbl->num-music_index-1;
            if(i > 0)
                memmove(&music_tbl->m[music_index], &music_tbl->m[music_index+1], sizeof(music_info_t)*i);
            music_tbl->num--;
            dir_tbl_sdram->m[dir_index].music_num = music_tbl->num;
            
            //当操作的音乐文件夹已满文件数时, 删除音乐文件需要重新轮训列表
            if(dir_tbl_sdram->m[dir_index].music_num_full && is_del)
            {
                mf_scan_files((TCHAR *)dir_name, 2, (uint8_t*)&music_tbl->m[0], sizeof(music_tbl->m), (int*)&music_tbl->num, &dir_tbl_sdram->m[dir_index].music_num_full);
                if(music_tbl->num > F_MUSIC_MAX_NUM)
                {
                    music_tbl->num = F_MUSIC_MAX_NUM;
                    dir_tbl_sdram->m[dir_index].music_num_full = 1;
                }
                dir_tbl_sdram->m[dir_index].music_num = music_tbl->num;          
            }
            DBG_PRINTF("  del old music_totsec dir_music_num:%d\n", dir_tbl_sdram->m[dir_index].music_num);
        }
    }
    //更新sdram数据列表
    fl_write_flielist(0, (uint8_t*)dir_tbl_sdram, F_DIR_TBL_BYTE_SIZE);
    fl_write_flielist(dir_sector, (uint8_t*)music_tbl, F_MUSIC_TBL_BYTE_SIZE);

FUN_END:
    myfree(dir_tbl_sdram);
    myfree(music_tbl);
    myfree(dir_name);
    myfree(music_name);
}


