#include "fl_buff_decode.h"
#include "user_xccode.h"
#include "protocol_adrbase.h"
#include "flash_adrbase.h"
#include "list_instance.h"
#include "list_contorl.h"
#include "user_unti.h"

#include "debug_print.h"
#include "string.h"

static uint8_t divinfo_len = sizeof(div_node_t);

//===================================================================
// 用户数据 主机信息 缓冲读写部分
//===================================================================
// 写命令 延时写
void hostinfo_fl_write(){
    g_sys_val.need_flash |= NEED_FL_HOSTINFO;
}

void sys_dat_read(uint8_t buff[],uint16_t num,uint16_t base_adr){
    memcpy(buff,tmp_union.buff+base_adr,num);
}

void sys_dat_write(uint8_t buff[],uint16_t num,uint16_t base_adr){
    memcpy(tmp_union.buff+base_adr,buff,num);
}

//----------------------------------------------------------------------------
// 烧写主机信息flash
//---------------------------------------------------------------------------
uint8_t timer_fl_hostinfo_decode(){
    user_fl_sector_read(USER_DAT_SECTOR);
    sys_dat_write((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);
    user_fl_sector_write(USER_DAT_SECTOR);
    //
    g_sys_val.need_flash ^= NEED_FL_HOSTINFO;
    return 1;
}

//===================================================================
// 账户信息      缓冲读写部分
//===================================================================
// 写账户信息
// 读账户详细信息
void account_fl_read(account_all_info_t *account_all_info,uint8_t id){
    user_fl_sector_read(ACCOUT_SECTOR+id);
    sys_dat_read(account_all_info,sizeof(account_all_info_t),FLB_ACCOUNT_DAT_BASE);
}

void account_fl_write(account_all_info_t *account_all_info,uint8_t id){
    unsigned len;
    len = sizeof(account_all_info_t);
    sys_dat_write(account_all_info,len,FLB_ACCOUNT_DAT_BASE);
    user_fl_sector_write(ACCOUT_SECTOR+id);
}

//===================================================================
// 分区数据读写部分
//===================================================================
// 写命令 延时写
void area_fl_write(){
    g_sys_val.need_flash |= NEED_FL_AREALIST;
}
// 即时读
void area_fl_read(){
    user_fl_sector_read(AREA_INFOLIST_SECTOR);
    memcpy(&area_info,tmp_union.buff,sizeof(area_info_t)*MAX_AREA_NUM);    
    
    for(uint8_t i=0;i<MAX_AREA_NUM;i++){
        
        debug_printf("a sn:%x ",area_info[i].area_sn);
    }
    debug_printf("\n");
}
// 写flash 实体
uint8_t timer_fl_arealist_decode(){
    memcpy(tmp_union.buff,&area_info,sizeof(area_info_t)*MAX_AREA_NUM);
    user_fl_sector_write(AREA_INFOLIST_SECTOR);
    g_sys_val.need_flash ^= NEED_FL_AREALIST;
    return 1;
}

//===================================================================
// 设备列表读写部分
//===================================================================
// 写命令 延时写
void divlist_fl_write(){
    g_sys_val.fl_divlist_inc=0; //从0开始烧写
    g_sys_val.need_flash |= NEED_FL_DIVLIST;
}
// 即时读
void divlist_fl_read(){
    uint16_t divlist_inc=0;
    uint16_t buf_adrbase=0;
    uint8_t sector_num=0;
    //
    // 初始化指针
    div_list.div_head_p=null;
    div_list.div_end_p=null;
    while(divlist_inc<MAX_DIV_LIST){
        // 读新sector
        user_fl_sector_read(DIV_INFOLIST_SECTOR+sector_num);
        buf_adrbase = 0;
        // 取数据 100个byte一个设备 ，一个sector放40个设备
        for(uint16_t i=0;(i<30)&&(divlist_inc<MAX_DIV_LIST);i++,divlist_inc++){
            memcpy(&div_list.div_node[divlist_inc],tmp_union.buff+buf_adrbase,divinfo_len);
            buf_adrbase+=130;   // 100个byte一个设备 ，一个sector放30个设备
        }
        sector_num++;
    }//while    
    for(uint8_t i=0;i<MAX_DIV_LIST;i++){
        div_list.div_node[i].next_p = null;
        if(div_list.div_node[i].div_info.id!=0xFF){
            // 防止FLASH 错乱
            if(div_list.div_node[i].div_info.id!=i){
                div_list.div_node[i].div_info.id = 0xFF;
                i++;
                continue;
            }
            div_node_creat_forid(i);
        }
    }
    //-----------------------------------------------------------------------
    #if 0
    div_node_t *tmp_p = div_list.div_head_p;
    while(tmp_p!=null){
       debug_printf("div:%x,%x,%x,%x,%x,%x\n",tmp_p->div_info.mac[0],tmp_p->div_info.mac[1],tmp_p->div_info.mac[2],
                                              tmp_p->div_info.mac[3],tmp_p->div_info.mac[4],tmp_p->div_info.mac[5]);
       tmp_p = tmp_p->next_p;
    }
    #endif
    //-----------------------------------------------------------------------
}

// 写flash 实体
uint8_t timer_fl_divlist_decode(){
    static uint8_t sector_num=0;
    uint16_t buf_adrbase=0;
    uint8_t  tmp=0;
    if(g_sys_val.fl_divlist_inc==0)
        sector_num=0;
    //
    // 写数据缓冲 130个byte一个设备 ，一个sector放30个设备
    for(uint16_t i=0;(i<30)&&(g_sys_val.fl_divlist_inc<MAX_DIV_LIST);i++,g_sys_val.fl_divlist_inc++){
        memcpy(tmp_union.buff+buf_adrbase,&div_list.div_node[g_sys_val.fl_divlist_inc],divinfo_len);
        buf_adrbase+=130;   // 130个byte一个设备 ，一个sector放30个设备
    }
    if(g_sys_val.fl_divlist_inc==MAX_DIV_LIST){    //烧写完毕
        g_sys_val.need_flash ^= NEED_FL_DIVLIST;
        tmp = 1;
    }
    user_fl_sector_write(DIV_INFOLIST_SECTOR+sector_num);
    sector_num++;
    return tmp;
}

//=====================================================================
// 定时与打铃任务读取
//--------------------------------------------------
// 烧写方案信息
//--------------------------------------------------
uint8_t timer_fl_solution_decode(){
    user_fl_sector_read(SOLUSION_DAT_SECTOR);
    sys_dat_write((char*)(&solution_list),sizeof(solution_list_t),FLASH_SOLUSION_LIST);
    user_fl_sector_write(SOLUSION_DAT_SECTOR);
    //
    g_sys_val.need_flash ^= NEED_FL_SOLUTION;
    return 1;
}
//==========================================================================================
// 任务
//--------------------------------------------------------------------
//--------------------------------------------------
// 读定时任务指令
void timer_task_read(task_allinfo_tmp_t     *task_allinfo_tmp,uint16_t id){
    user_fl_sector_read(TIMED_TASK_SECTOR+id);
    sys_dat_read(task_allinfo_tmp,sizeof(task_allinfo_tmp_t),FLB_TASK_DAT_BASE);
}

//--------------------------------------------------
// 写定时任务指令
//--------------------------------------------------
void timer_task_write(task_allinfo_tmp_t *task_allinfo_tmp, uint16_t id){
    sys_dat_write(task_allinfo_tmp,sizeof(task_allinfo_tmp_t),FLB_TASK_DAT_BASE);
    user_fl_sector_write(TIMED_TASK_SECTOR+id);
}
//--------------------------------------------------
// 读即时任务指令
//--------------------------------------------------
void rt_task_read(rttask_dtinfo_t     *rttask_dtinfo,uint16_t id){
    if(id>MAX_RT_TASK_NUM-1)
        return;
    user_fl_sector_read(RT_TASK_SECTOR+id);
    sys_dat_read(rttask_dtinfo,sizeof(rttask_dtinfo_t),FLB_RTTASK_DAT_BASE);
}
//--------------------------------------------------
// 写即时任务指令
//--------------------------------------------------
void rt_task_write(rttask_dtinfo_t *rttask_dtinfo,uint16_t id){
    if(id>MAX_RT_TASK_NUM-1)
        return;
    sys_dat_write(rttask_dtinfo,sizeof(rttask_dtinfo_t),FLB_RTTASK_DAT_BASE);
    user_fl_sector_write(RT_TASK_SECTOR+id);
}

//===================================================================
// 定时flash 数据烧写
//===================================================================
//----------------------------------------------------------
//烧写函数列表
typedef struct timer_fl_fun_t{
    uint8_t (*fun)(void);
}timer_fl_fun_t;

timer_fl_fun_t timer_fl_fun[]={timer_fl_hostinfo_decode,
                               timer_fl_arealist_decode,
                               timer_fl_divlist_decode,
                               timer_fl_solution_decode
                              };
//----------------------------------------------------------

//--------------------------------------------
// 烧写轮询
void timer_flash_dat_process(){
    unsigned bit_tmp = g_sys_val.need_flash;
    for(uint8_t i=0;i<4;i++){
        if(bit_tmp&0x01){
            timer_fl_fun[i].fun();
            break;
        }
        bit_tmp>>=1;
    }
}

