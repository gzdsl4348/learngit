#include <stdint.h>
#include <string.h>
#include "debug_print.h"
#include "mymalloc.h"
#include "ff.h"
#include "mystring.h"

extern void printfstr(TCHAR *str);

extern FATFS fatfs;

//fwmode : bit0 - 0 不覆盖
//                1 覆盖
static uint8_t f_copy(uint8_t *psrc,uint8_t *pdst, uint8_t *pcurpct, uint8_t *pexit, uint32_t totsize,uint32_t cpdsize,uint8_t fwmode)
{
    uint8_t res;
    uint16_t br = 0;
    uint16_t bw = 0;
    FIL *fsrc = 0;
    FIL *fdst = 0;
    uint8_t *fbuf = 0;
    uint8_t curpct = 0;
    *pcurpct = 0;

    fsrc=(FIL*)mymalloc(sizeof(FIL));//申请内存
    fdst=(FIL*)mymalloc(sizeof(FIL));
    fbuf=(uint8_t*)mymalloc(16*1024);

    if(fsrc==NULL||fdst==NULL||fbuf==NULL)res=100;//前面的值留给fatfs
    else
    {
        if(fwmode&0x01)
            fwmode=FA_CREATE_ALWAYS;//覆盖存在的文件
        else
            fwmode=FA_CREATE_NEW;//不覆盖

        res=f_open(fsrc,(const TCHAR*)psrc,FA_READ|FA_OPEN_EXISTING);   //打开只读文件
        if(res==0)res=f_open(fdst,(const TCHAR*)pdst,FA_WRITE|fwmode);  //第一个打开成功,才开始打开第二个
        if(res==0)//两个都打开成功了
        {
            if(totsize==0)//仅仅是单个文件复制
            {
                totsize=fsrc->fsize;
                cpdsize=0;
                curpct=0;
            }
            else
                curpct=(cpdsize*100)/totsize;  //得到新百分比
            // 文件容量判断
            debug_printf("cl tol %d size %d\n",fatfs.n_fatent-2,fsrc->fsize/512);
            if((fatfs.free_clust-2)<(fsrc->fsize/512/64)){//>(fatfs.n_fatent-2)){
                res = 0xFF;
            }
            else{
                *pcurpct = curpct;          //更新百分比
                while(res==0)//开始复制
                {
                    res=f_read(fsrc,fbuf,16*1024,(UINT*)&br);  //源头读出512字节
                    if(res||br==0)break;

                    res=f_write(fdst,fbuf,(UINT)br,(UINT*)&bw); //写入目的文件

                    cpdsize+=bw;

                    if(curpct != (cpdsize*100)/totsize)//是否需要更新百分比
                    {
                        curpct=(cpdsize*100)/totsize;
                        *pcurpct = curpct;//更新百分比

                        if(*pexit)
                        {
                            res=0XFF;//强制退出
                            *pexit=0;
                            break;
                        }
                    }
                    if(res||bw<br)break;
                }
            }
            f_close(fsrc);
            f_close(fdst);
        }
    }
    myfree(fsrc);//释放内存
    myfree(fdst);
    myfree(fbuf);
    return res;

}
static uint8_t fname_rename(TCHAR *fname, int cnt)
{
    
    TCHAR *attr = 0;//后缀名, 包含'.'
    unsigned char i=0;
    TCHAR cnt_str[4] = {'-','0'+((cnt/10)%10),'0'+(cnt%10),0}; 
    TCHAR tmp[10] = {0};
    
    while(i<250)
    {
        i++;
        if(*fname=='\0')break;//偏移到了最后了.
        fname++;
    }
    if(i==250)return 0XFF;//错误的字符串.

    attr=fname;
    
    for(i=0;i<5;i++)//得到后缀名
    {
        fname--;
        if(*fname=='.')
        {
            attr=fname;
            break;
        }
    }
    
    wstrcpy(tmp, attr);
    wstrcpy(attr, cnt_str);
    wstrcpy(attr+3, tmp);
    
    return 0;
}
uint8_t mf_copy(uint8_t *psrc,uint8_t *pdst,uint8_t *pcurpct,uint8_t *pexit,uint32_t totsize,uint32_t cpdsize,uint8_t mode)
{
    uint8_t res = 0;
    uint8_t rename_cnt = 1;
    
    if(mode == 0)//F_COPY_NO_COVER
    {
        res = f_copy(psrc, pdst, pcurpct, pexit, totsize, cpdsize, 0);
    }
    else if(mode == 1)//F_COPY_COVER
    {
        res = f_copy(psrc, pdst, pcurpct, pexit, totsize, cpdsize, 1);
    }
    else if(mode == 2)//F_COPY_AUTO_RENAME
    {
        uint8_t *dst_name = mymalloc(128*2);
        res = f_copy(psrc, pdst, pcurpct, pexit, totsize, cpdsize, 0);
        while(res == FR_EXIST && rename_cnt<100)
        {
            wstrcpy((TCHAR*)dst_name, (TCHAR*)pdst);
            if(fname_rename((TCHAR*)dst_name, rename_cnt)) 
            {
                res = FR_EXIST;
                break;
            }
#if 0
            debug_printf("fname_rename: [");
            printfstr(dst_name);
            debug_printf("]\n");
#endif
            res = f_copy(psrc, dst_name, pcurpct, pexit, totsize, cpdsize, 0);
            rename_cnt++;
        }

        myfree(dst_name);
    }
    return res;
}

uint8_t mf_mkdir(uint8_t*pname)
{
#if 0
    debug_printf("mf_mkdir [%c%c%c%c%c%c] \n", pname[0],pname[2],pname[4],pname[6],pname[8],pname[10]);
#endif
    return f_mkdir((const TCHAR *)pname);
}



uint8_t mf_rename(uint8_t *oldname, uint8_t* newname)
{
    return  f_rename((const TCHAR *)oldname, (const TCHAR *)newname);
}


static uint8_t  f_delfile(const TCHAR *path, TCHAR *dir_buff, uint16_t start_index, uint16_t *offset, uint16_t *dir_num)
{
    FRESULT res;
    DIR   dir;     /* 文件夹对象 */ //36  bytes
    FILINFO fno;   /* 文件属性 */   //32  bytes

    uint8_t has_dir = 0;
    
    TCHAR *p;
    TCHAR *file = mymalloc((_MAX_LFN + 2)*2);
    TCHAR *lname = mymalloc((_MAX_LFN + 2)*2);

    fno.lfsize = _MAX_LFN;
    fno.lfname = lname;    //必须赋初值

    res = f_opendir(&dir, path);
    *offset = 0;
    
    //持续读取文件夹内容
    while((res == FR_OK) && (FR_OK == f_readdir(&dir, &fno)))
    {
        //若是"."或".."文件夹，跳过
        if(fno.fname[0] == 0)              break;      //若读到的文件名为空
        if(fno.fname[0] == '.')            continue;   //"."若读到的文件名为当前文件夹, ".."若读到的文件名为上一级文件夹

        memset(file, 0, (_MAX_LFN + 2)*2);
        p = wstrcpy(file, path);
        *(p++) = '/';
        wstrcpy(p, (*fno.lfname) ? fno.lfname : fno.fname);

        if (fno.fattrib & AM_DIR)//若是文件夹，递归删除
        {
            wstrcpy(dir_buff+start_index+*offset, file);
            *offset += (wstrlen(file)+1);
            has_dir = 1;
            (*dir_num)++;
        }
        else//若是文件，直接删除
        {
            res = f_unlink(file);
        }
    }

    //删除本身
    if(!has_dir)    
    {
        res = f_unlink((const TCHAR *)path);
        
        res = (res==FR_DENIED)?0:res;
        
        (*dir_num)--;
    }
    
    myfree(file);
    myfree(lname);

    return res;
}



uint8_t mf_unlink(uint8_t *pname)
{
    TCHAR *dir_buff = NULL;
    
    uint16_t name_index = 0;
    uint16_t start_index = 0;
    uint16_t offset = 0;
    uint16_t dir_num = 1;
    uint8_t res = 0;
    


    dir_buff = mymalloc((_MAX_LFN + 2)*2);
    if(dir_buff==NULL)
    {
        return 0xff;
    }
    
    wstrcpy(dir_buff, (TCHAR*)pname);
    start_index = wstrlen(dir_buff)+1;
    name_index = 0;

    while(1)
    {
#if 0
        debug_printf("deldir: %02d %02d %02d [", name_index, start_index, dir_num);
        printfstr(dir_buff+name_index);
        debug_printf("]\n");
#endif
        
        res = f_delfile(dir_buff+name_index, dir_buff, start_index, &offset, &dir_num);
        
        if(dir_num==0 || res!=0) break;
        
        if(offset)
        {
            //增加文件夹
            start_index += offset;
            
            name_index = start_index-1;
            while(dir_buff[name_index-1] != 0) name_index--;
        }
        else
        {
            //删除文件夹
            //start_index左移一个dir_name
#if 0
            start_index--;
            while(dir_buff[start_index-1] != 0)start_index--;
#else
            start_index = name_index;
#endif

            //name_index左移一个dir_name
            name_index--;
            while(dir_buff[name_index-1] != 0) name_index--;
        }
    }

    myfree(dir_buff);

    return res;
}


//=======================================================================================================================
// 日志管理
static uint8_t log_filename[64];
//------------------------------------------------------------------------------
// 建立日志文件
uint8_t mf_open_log(char *file_newname,char *file_oldname)
{
    FIL *logfile = 0;
    uint8_t res;
    uint16_t bw=0;
    uint8_t utf_16le_type[2]={0xFF,0xFE};
    //
    logfile=(FIL*)mymalloc(sizeof(FIL));//申请内存
    // 删除日志文件
    f_unlink((const TCHAR*)file_oldname);
    // 创建新日志    
    res = f_open(logfile,(const TCHAR*)file_newname,FA_WRITE|FA_READ|FA_OPEN_ALWAYS);   //打开创建文件
    f_write(logfile,(const TCHAR*)utf_16le_type, 2, &bw);
    memcpy(log_filename,file_newname,64);    //备份文件名  
    f_close(logfile);
    myfree(logfile);
    //
    return res;
}
//------------------------------------------------------------------------------
// 插入日志条目
uint8_t mf_add_loginfo(char *file_name,unsigned len){
    FIL *logfile = 0;
    uint8_t res;
    uint16_t br=0;
    uint16_t bw=0;
    logfile=(FIL*)mymalloc(sizeof(FIL));//申请内存
    //
    res = f_open(logfile,(const TCHAR*)log_filename,FA_WRITE|FA_READ|FA_OPEN_ALWAYS);   //打开创建文件
    //
    f_lseek(logfile,logfile->fptr+logfile->fsize);
    res = f_write(logfile,(const TCHAR*)file_name, len, &bw);
    debug_printf("loginfo er %d \n",res);
    //
    f_close(logfile);
    myfree(logfile);
    return res;
}


