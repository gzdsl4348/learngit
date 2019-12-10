#include "user_lcd.h"
#include "user_xccode.h"
#include "sys_config_dat.h"
#include "user_unti.h"
#include "list_contorl.h"
#include "task_decode.h"
#include "fl_buff_decode.h"
#include "debug_print.h"
#include "sys_log.h"

#define DAT_DISP_BASE   (7)
//---------------------------------------------
// 日期显示
#define DISP_YEAR_ER    (DAT_DISP_BASE)    
#define DISP_YEAR_BAI   (DISP_YEAR_ER+1)
#define DISP_YEAR_SHI   (DISP_YEAR_BAI+1)
#define DISP_YEAR_GE    (DISP_YEAR_SHI+1)
#define DISP_YEAR_GANG  (DISP_YEAR_GE+1)
#define DISP_MONTH_SHI  (DISP_YEAR_GANG+1)
#define DISP_MONTH_GE   (DISP_MONTH_SHI+1)
#define DISP_MONTH_GANG (DISP_MONTH_GE+1)
#define DISP_DAY_SHI    (DISP_MONTH_GANG+1)
#define DISP_DAY_GE     (DISP_DAY_SHI+1)

#define DISP_DATA_LEN   (DISP_DAY_GE+1)  
//-------------------------------------------
// 时间显示
#define DISP_HOUR_SHI   (DAT_DISP_BASE)
#define DISP_HOUR_GE    (DISP_HOUR_SHI+2)
#define DISP_HOUR_MAO   (DISP_HOUR_GE+2)
#define DISP_MINUTE_SHI (DISP_HOUR_MAO+2)
#define DISP_MINUTE_GE  (DISP_MINUTE_SHI+2)
#define DISP_MINUTE_MAO (DISP_MINUTE_GE+2)
#define DISP_SECOND_SHI (DISP_MINUTE_MAO+2)
#define DISP_SECOND_GE  (DISP_SECOND_SHI+2)

#define DISP_TIME_LEN   (DISP_SECOND_GE+2)  
//--------------------------------------------
// IP显示
#define DISP_IP_BASE    (DAT_DISP_BASE)  
//-------------------------------------------
// 任务显示
#define DISP_TASK_TITLE (DAT_DISP_BASE) //8+1
#define DISP_TASK_INFO  (DISP_TASK_TITLE+9)
//-------------------------------------------
// 版本显示
//--------------------------------------------
#define DISP_VER_ID 14

#define DISP_IP_ID 1
#define DISP_GATE_ID 2
#define DISP_MASK_ID 4
#define DISP_MAC_ID  3

#define DISP_TIME_ID    8
#define DISP_DATA_ID    5
#define DISP_WEEK_ID    7

#define DISP_COULD_ID   13

//
#define DISP_TASKINFO1_ID 9
#define DISP_TASKINFO2_ID 10

#define DISP_TASKSTATE_ID  6

//
#define DISP_DHCPS_ID 11

#define DISP_IPCONFILCT_ID 12

uint8_t disp_buff[100];
volatile uint8_t disp_len;

void send_buff(uint8_t id){
    disp_buff[0] = 0xEE;
    disp_buff[1] = 0xB1;
    disp_buff[2] = 0x10;
    disp_buff[3] = 0x00;
    disp_buff[4] = 0x00;
    disp_buff[5] = 0x00;
    disp_buff[6] = id;
    disp_buff[disp_len] = 0x00;
    disp_buff[disp_len+1] = 0x00;
    disp_buff[disp_len+2] = 0xFF;
    disp_buff[disp_len+3] = 0xFC;
    disp_buff[disp_len+4] = 0xFF;
    disp_buff[disp_len+5] = 0xFF;
    disp_len += 6;
    /*
    for(uint8_t i=0;i<disp_len;i++){
        xtcp_debug_printf("%x ",disp_buff[i]);
    }
    xtcp_debug_printf("\n");
    */
    user_uart_tx(disp_buff,disp_len);
}

void user_disp_icon(uint8_t id,uint8_t dat){
    disp_buff[0] = 0xEE;
    disp_buff[1] = 0xB1;
    disp_buff[2] = 0x23;
    disp_buff[3] = 0x00;
    disp_buff[4] = 0x00;
    disp_buff[5] = 0x00;
    disp_buff[6] = id;
    disp_buff[7] = dat;
    disp_buff[8] = 0xFF;
    disp_buff[9] = 0xFC;
    disp_buff[10] = 0xFF;
    disp_buff[11] = 0xFF;
    user_uart_tx(disp_buff,12);
}

void disp_unti(uint8_t id,uint8_t *dat_base){
    memcpy(&disp_buff[DAT_DISP_BASE],dat_base,10);
    disp_len = 10+DAT_DISP_BASE;
    send_buff(id);
}

void disp_time_unti(time_info_t *time_info,uint8_t *buff,uint8_t adr_base){
    buff[adr_base] = 0x30+time_info->hour/10;
    //
    buff[adr_base+1] = 0x30+time_info->hour%10;
    // 冒号
    buff[adr_base+2] = 0x3A;
    // 分
    buff[adr_base+3] = 0x30+time_info->minute/10;
    buff[adr_base+4] = 0x30+time_info->minute%10;
    // 冒号
    buff[adr_base+5] = 0x3A;
    // 秒
    buff[adr_base+6] = 0x30+time_info->second/10;
    // 
    buff[adr_base+7] = 0x30+time_info->second%10;
    //
    buff[adr_base+8] = 0x00;
}

// 没有任务
uint8_t none_task_char[10] = {0x6C,0xA1,0x67,0x09,0x4E,0xFB,0x52,0xA1,0x00,0x00};

// 当前任务
uint8_t nowtask_char[10] = {0x5F,0x53,0x52,0x4D,0x4E,0xFB,0x52,0xA1,0x00,0x3A};
// 持续时间
uint8_t dura_char[10] = {0x63,0x01,0x7E,0xED,0x65,0xF6,0x95,0xF4,0x00,0x3A};
// 开始时间
uint8_t begtime_char[10] = {0x5F,0x00,0x59,0xCB,0x65,0xF6,0x95,0xF4,0x00,0x3A};
// 正在播放
uint8_t playing_char[10] = {0x6B,0x63,0x57,0x28,0x64,0xAD,0x65,0x3E,0x00,0x3A};
// 即将执行
uint8_t future_play_char[10] = {0x53,0x73,0x5C,0x06,0x62,0x67,0x88,0x4C,0x00,0x3A};

void user_dispunti_init(){
    dhcp_disp_none();
    ip_conflict_disp(0);
    disp_couldstate(0);
}

// 
uint8_t dispchange_futuref=0;
uint8_t dispchange_nowf=0;

void disp_taskname(uint8_t *name){
    // 任务名称
    uint8_t task_name_char[]={0x4E,0xFB,0x52,0xA1,0x54,0x0D,0x79,0xF0,0x00,0x3A};
    //------------------------------------------------------------------------------------
    //获取任务名
    memcpy(&g_sys_val.dispname_buff[g_sys_val.disp_num][0],task_name_char,10);
    uint8_t data_base=10;
    //
    uint8_t inc=0;
    for(inc=0;inc<DIV_NAME_NUM/2;inc++){
        g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+inc*2] = name[inc*2+1];
        g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+inc*2+1] = name[inc*2];
        if((g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+inc*2]==0)&&(g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+inc*2+1]==0))
            break;
    }
    // 显示省略号
    g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+20] = 00;
    g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+21] = 0x2E;
    g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+22] = 00;
    g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+23] = 0x2E;  
    g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+24] = 00;
    g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+25] = 0x2E;
    g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+26] =0x00;
    g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+27] =0x00;
    
    //g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+inc*2+1] = 0x00;
    //g_sys_val.dispname_buff[g_sys_val.disp_num][data_base+inc*2+2] = 0x00;
}

void disp_tasktime_forunicode(time_info_t *time_info,uint8_t *buff,uint8_t *adr_base){
    buff[*adr_base]  = 0;
    buff[*adr_base+1] = 0x30+time_info->hour/10;
    //
    *adr_base+=2;
    buff[*adr_base]  = 0;
    buff[*adr_base+1] = 0x30+time_info->hour%10;
    // 冒号
    *adr_base+=2;
    buff[*adr_base]  = 0;
    buff[*adr_base+1] = 0x3A;
    // 分
    *adr_base+=2;
    buff[*adr_base]  = 0;
    buff[*adr_base+1] = 0x30+time_info->minute/10;
    //
    *adr_base+=2;
    buff[*adr_base]  = 0;
    buff[*adr_base+1] = 0x30+time_info->minute%10;
    // 冒号
    *adr_base+=2;
    buff[*adr_base]  = 0;
    buff[*adr_base+1] = 0x3A;
    // 秒
    *adr_base+=2;
    buff[*adr_base]  = 0;
    buff[*adr_base+1] = 0x30+time_info->second/10;
    // 
    *adr_base+=2;
    buff[*adr_base]  = 0;
    buff[*adr_base+1] = 0x30+time_info->second%10;
    //
    *adr_base+=2;
}

void disp_task_time(){
    uint8_t data_base=0;
    //开始时间
    memcpy(g_sys_val.disinfo2buf[g_sys_val.disp_num],begtime_char,10); 
    data_base+=10;
    //
    disp_tasktime_forunicode(&g_tmp_union.task_allinfo_tmp.task_coninfo.time_info,g_sys_val.disinfo2buf[g_sys_val.disp_num],&data_base);
    // 持续时间
    /*
    memcpy(&g_sys_val.disinfo2buf[g_sys_val.disp_num][data_base],dura_char,10); 
    data_base+=10;
    time_info_t time_info;
    time_info.hour = tmp_union.task_allinfo_tmp.task_coninfo.dura_time/3600;
    time_info.minute = (tmp_union.task_allinfo_tmp.task_coninfo.dura_time%3600)/60;
    time_info.second = (tmp_union.task_allinfo_tmp.task_coninfo.dura_time%3600)%60;
    disp_tasktime_forunicode(&time_info,g_sys_val.disinfo2buf[g_sys_val.disp_num],&data_base);
    */
    g_sys_val.disinfo2buf[g_sys_val.disp_num][data_base] = 0;
    g_sys_val.disinfo2buf[g_sys_val.disp_num][data_base+1] = 0;
}

void user_disptask_refresh(){
    g_sys_val.disp_num = 0;
    g_sys_val.disp_furef=0;
    //---------------------------------------------------------------------------------------------
    // 获取现在任务名称
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if(timetask_now.ch_state[i]!=0xFF){
            //有正在运行的任务
            g_sys_val.disp_furef=1;
            g_sys_val.disp_ch[g_sys_val.disp_num] = i; 
            // 读取任务名称
            if(timetask_now.task_musicplay[i].rttask_f){
                fl_rttask_read(&g_tmp_union.rttask_dtinfo,timetask_now.task_musicplay[i].task_id);
                disp_taskname(g_tmp_union.rttask_dtinfo.name);
            }else{
                fl_timertask_read(&g_tmp_union.task_allinfo_tmp,timetask_now.task_musicplay[i].task_id);
                disp_taskname(g_tmp_union.task_allinfo_tmp.task_coninfo.task_name);
            }
            //user_rttask_musname_get(&g_sys_val.rttask_musinfo,i);
            g_sys_val.disp_task_id[g_sys_val.disp_num]=timetask_now.task_musicplay[i].task_id;
            //
            g_sys_val.disp_num++;
        }
        if(g_sys_val.disp_num>=MAX_DISP_TASK)
            break;
    }
    // 获取即将运行任务名称
    if(g_sys_val.disp_furef)
        return;
    // 没有运行中任务 显示即将运行任务
    timetask_t *today_t_p = timetask_list.today_timetask_head;
    for(uint8_t i=0;i<1;i++){ //只显示一个即时任务
        if(today_t_p==(timetask_t *)null)
            break;
        if(g_sys_val.disp_num>=MAX_DISP_TASK)
            break;
        //-----------------------------------------------------------------------------------
        // 显示任务名称
        fl_timertask_read(&g_tmp_union.task_allinfo_tmp,today_t_p->id);
        debug_printf("\n\nhave future task disp\n\n");
        disp_taskname(g_tmp_union.task_allinfo_tmp.task_coninfo.task_name);
        // 显示任务时间
        disp_task_time();
        // 无
        //------------------------------------------------------------------------------------
        g_sys_val.disp_num++;
        today_t_p = today_t_p->today_next_p;
    }
    //
    if (g_sys_val.disp_num==0){
        // 没有任务
        memcpy(g_sys_val.dispname_buff[0],none_task_char,10);
        memset(g_sys_val.disinfo2buf[0],0x00,10);
    } 
    g_sys_val.disp_delay_f = 1;
    g_sys_val.disp_delay_inc = 0;

}

// 延时显示任务
void disp_task_delay(){
    if(g_sys_val.disp_delay_f){
        g_sys_val.disp_delay_inc++;
        if(g_sys_val.disp_delay_inc>5){
            user_disp_task(0);
            g_sys_val.disp_delay_f=0;
        }
    }
}

//现在任务      rt_state 0=打铃任务 1=定时任务          now_state 
void user_disp_task(uint8_t state){
    user_disp_icon(DISP_TASKSTATE_ID,g_sys_val.disp_furef);
    //----------------------------------------------------------------------------------------
    // 任务名
    memcpy(&disp_buff[DAT_DISP_BASE],g_sys_val.dispname_buff[state],DIV_NAME_NUM+6);
    disp_len = DAT_DISP_BASE+DIV_NAME_NUM+6;
    send_buff(DISP_TASKINFO1_ID);
    // 信息2显示
    memcpy(&disp_buff[DAT_DISP_BASE],g_sys_val.disinfo2buf[state],MUSIC_NAME_NUM);
    disp_len = DAT_DISP_BASE+MUSIC_NAME_NUM;
    send_buff(DISP_TASKINFO2_ID);
}

void timer_task_disp(){
    static uint8_t tim_inc=0;
    static uint8_t disp_inc=0;
    tim_inc++;
    if(tim_inc>2){
        tim_inc=0;
        if(disp_inc+1 > g_sys_val.disp_num)
            disp_inc=0;
        user_disp_task(disp_inc);
        disp_inc++;
    }
}

void ip_disp_decode(uint8_t data,uint8_t *base_adr){
    uint8_t zero_f=0;
    //disp_buff[*base_adr] = 0x00;
    disp_buff[*base_adr] = data/100+0x30;
    if(disp_buff[*base_adr]!=0x30){
        zero_f = 1;
        *base_adr+=1;
    }
    //
    disp_buff[*base_adr] = (data/10)%10+0x30;
    //disp_buff[*base_adr] = 0x00; 
    if((zero_f)||(disp_buff[*base_adr]!=0x30))
        *base_adr+=1;    
    disp_buff[*base_adr] = data%10+0x30;
    //disp_buff[*base_adr] = 0x00; 
    *base_adr+=1;
    //    
}

void user_disp_ip(xtcp_ipconfig_t ipconfig){
    uint8_t base_adr = DISP_IP_BASE;
    ip_disp_decode(ipconfig.ipaddr[0],&base_adr);
    //disp_buff[base_adr] = 00;
    disp_buff[base_adr] = 0x2E;
    base_adr+=1;
    ip_disp_decode(ipconfig.ipaddr[1],&base_adr);
    //disp_buff[base_adr] = 00;
    disp_buff[base_adr] = 0x2E;
    base_adr+=1;
    ip_disp_decode(ipconfig.ipaddr[2],&base_adr);
    //disp_buff[base_adr] = 00;
    disp_buff[base_adr] = 0x2E;
    base_adr+=1;
    ip_disp_decode(ipconfig.ipaddr[3],&base_adr);
    disp_buff[base_adr] = 0x00;
    base_adr+=1;
    //----------------------------------------------------------------
    disp_len = base_adr;
    send_buff(DISP_IP_ID);
    //---------------------------------------------------------------
    // 显示网关  
    base_adr = DISP_IP_BASE;
    ip_disp_decode(ipconfig.gateway[0],&base_adr);
    //disp_buff[base_adr] = 00;
    disp_buff[base_adr] = 0x2E;
    base_adr+=1;
    ip_disp_decode(ipconfig.gateway[1],&base_adr);
    //disp_buff[base_adr] = 00;
    disp_buff[base_adr] = 0x2E;
    base_adr+=1;
    ip_disp_decode(ipconfig.gateway[2],&base_adr);
    //disp_buff[base_adr] = 00;
    disp_buff[base_adr] = 0x2E;
    base_adr+=1;
    ip_disp_decode(ipconfig.gateway[3],&base_adr);
    disp_buff[base_adr] = 0x00;
    base_adr+=1;
    //----------------------------------------------------------------
    disp_len = base_adr;
    send_buff(DISP_GATE_ID);
    //---------------------------------------------------------------
    // 显示掩码       4D 41 53 4B 
    base_adr = DISP_IP_BASE;
    ip_disp_decode(ipconfig.netmask[0],&base_adr);
    //disp_buff[base_adr] = 00;
    disp_buff[base_adr] = 0x2E;
    base_adr+=1;
    ip_disp_decode(ipconfig.netmask[1],&base_adr);
    //disp_buff[base_adr] = 00;
    disp_buff[base_adr] = 0x2E;
    base_adr+=1;
    ip_disp_decode(ipconfig.netmask[2],&base_adr);
    //disp_buff[base_adr] = 00;
    disp_buff[base_adr] = 0x2E;
    base_adr+=1;
    ip_disp_decode(ipconfig.netmask[3],&base_adr);
    disp_buff[base_adr] = 0x00;
    base_adr+=1;
    //----------------------------------------------------------------
    disp_len = base_adr;
    send_buff(DISP_MASK_ID);
    //---------------------------------------------------------------
    // 显示MAC    4D 41 43 3A 
    base_adr = DISP_IP_BASE;
    for(uint8_t i=0;i<6;i++){
        if((host_info.mac[i]>>4)<0x0A)
            disp_buff[base_adr] = (host_info.mac[i]>>4)+0x30;
        else
            disp_buff[base_adr] = (host_info.mac[i]>>4)-0x0A+0x41;
        //
        if((host_info.mac[i]&0x0F)<0x0A)
            disp_buff[base_adr+1] = (host_info.mac[i]&0x0F)+0x30;
        else
            disp_buff[base_adr+1] = (host_info.mac[i]&0x0F)-0x0A+0x41;
        //
        disp_buff[base_adr+2] = 0x2D;
        base_adr+=3; 
    }
    disp_buff[base_adr-1]=00;
    //----------------------------------------------------------------
    disp_len = base_adr-1;
    send_buff(DISP_MAC_ID);
}

void user_disp_time(){
    //
    disp_time_unti(&g_sys_val.time_info,disp_buff,DAT_DISP_BASE);
    disp_len = DAT_DISP_BASE+16;
    send_buff(DISP_TIME_ID);
}

void user_disp_data(){
    // year
    disp_buff[DISP_YEAR_ER] = 0x32;
    //
    disp_buff[DISP_YEAR_BAI] = 0x30 + g_sys_val.date_info.year/100;
    //
    disp_buff[DISP_YEAR_SHI] = 0x30 + (g_sys_val.date_info.year/10)%10;
    //
    disp_buff[DISP_YEAR_GE] = 0x30 + g_sys_val.date_info.year%10;
    // - 
    disp_buff[DISP_YEAR_GANG] = 0x2D;
    // month
    disp_buff[DISP_MONTH_SHI] = 0x30+g_sys_val.date_info.month/10;
    //
    disp_buff[DISP_MONTH_GE] = 0x30+g_sys_val.date_info.month%10;
    // -
    disp_buff[DISP_MONTH_GANG] = 0x2D;
    // day
    disp_buff[DISP_DAY_SHI] = 0x30+g_sys_val.date_info.date/10;
    //
    disp_buff[DISP_DAY_GE] = 0x30+g_sys_val.date_info.date%10;
    //
    disp_len = DISP_DATA_LEN;
    send_buff(DISP_DATA_ID);

    user_disp_icon(DISP_WEEK_ID,g_sys_val.date_info.week-1);
}

void user_disp_version(){
    uint8_t len=0;
    disp_buff[DAT_DISP_BASE] = 0x56;
    len++;
    disp_buff[DAT_DISP_BASE+len] = 0x30+(VERSION_H>>4);
    if((VERSION_H>>4)!=0)
        len++;
    disp_buff[DAT_DISP_BASE+len] = 0x30+(VERSION_H&0x0f);
    len++;
    disp_buff[DAT_DISP_BASE+len] = 0x2E;
    len++;
    disp_buff[DAT_DISP_BASE+len] = 0x30+(VERSION_L>>4);
    len++;
    disp_buff[DAT_DISP_BASE+len] = 0x30+(VERSION_L&0x0f);
    len++;
    //
    disp_len = DAT_DISP_BASE+len;
    send_buff(DISP_VER_ID);
}

// 显示DHCP打开
void dhcp_disp_en(){
    //“DHCP打开” 
    uint8_t dhcp_enchar[]={0x00,0x44,0x00,0x48,0x00,0x43,0x00,0x50,0x00,0x3A,0x62,0x53,0x5F,0x00}; 
    memcpy(&disp_buff[DAT_DISP_BASE],dhcp_enchar,14);
    disp_len = DAT_DISP_BASE+14;
    send_buff(DISP_DHCPS_ID);
}

void dhcp_disp_dis(){
    #if 0
    uint8_t dhcp_dischar[]={0x00,0x44,0x00,0x48,0x00,0x43,0x00,0x50,0x00,0x3A,0x51,0x73,0x95,0xED}; 
    memcpy(&disp_buff[DAT_DISP_BASE],dhcp_dischar,14);
    disp_len = DAT_DISP_BASE+14;
    send_buff(DISP_DHCPS_ID);
    #endif
}

// 清空DHCP显示
void dhcp_disp_none(){
    uint8_t dhcp_nonechar[]={0x00,0x00}; 
    memcpy(&disp_buff[DAT_DISP_BASE],dhcp_nonechar,2);
    disp_len = DAT_DISP_BASE+2;
    send_buff(DISP_DHCPS_ID);
    
}

// 显示IP冲突
void ip_conflict_disp(uint8_t state){
    if(state){
        uint8_t ipconflict_char[]={0x00,0x49,0x00,0x50,0x51,0xB2,0x7A,0x81,0x00,0x21}; 
        memcpy(&disp_buff[DAT_DISP_BASE],ipconflict_char,10);
        disp_len = DAT_DISP_BASE+10;
        send_buff(DISP_IPCONFILCT_ID);
    }
    else{
        memset(&disp_buff[DAT_DISP_BASE],0,2);
        disp_len = DAT_DISP_BASE+2;
        send_buff(DISP_IPCONFILCT_ID);
    }
}

// 显示云连接图标状态
// state=0 关闭
// state=1 打开
void disp_couldstate(uint8_t state){
    user_disp_icon(DISP_COULD_ID,state);
}

// 显示IP获取中
void dhcp_getin_disp(){
    // "IP获取中"
    uint8_t disp_char[]={0x00,0x49,0x00,0x50,0x83,0xB7,0x53,0xD6,0x4E,0x2D}; 
    memcpy(&disp_buff[DAT_DISP_BASE],disp_char,10);
    disp_len = DAT_DISP_BASE+10;
    send_buff(DISP_IPCONFILCT_ID);
}

// state=0 显示IP获取失败
// state=1 显示IP获取成功
void dhcp_getin_over_disp(uint8_t state){
    // “IP获取成功”
    uint8_t disp_char_succse[]={0x00,0x49,0x00,0x50,0x83,0xB7,0x53,0xD6,0x62,0x10,0x52,0x9F}; 
    // “IP获取失败”
    uint8_t disp_char_fail[]={0x00,0x49,0x00,0x50,0x83,0xB7,0x53,0xD6,0x59,0x31,0x8D,0x25};
    if(state){
        memcpy(&disp_buff[DAT_DISP_BASE],disp_char_succse,12);

    }else{
        memcpy(&disp_buff[DAT_DISP_BASE],disp_char_fail,12);
    }
    disp_len = DAT_DISP_BASE+12;
    send_buff(DISP_IPCONFILCT_ID);
}

void dhcp_getin_clear(){
    ip_conflict_disp(0);
}

void wifi_open_disp(){
    //wifi 已打开
    char wifiopen[]={0x00,0x57,0x00,0x49,0x00,0x46,0x00,0x49,0x5D,0xF2,0x62,0x53,0x5F,0x00};//14
                     
    memcpy(&disp_buff[DAT_DISP_BASE],wifiopen,14);
    //itoa_forutf16(host_info.mac[4],&disp_buff[DAT_DISP_BASE+30],16,2);
    disp_len = DAT_DISP_BASE+14;
    send_buff(DISP_DHCPS_ID);
    
    char hilinkname[]={0x00,0x48,0x00,0x49,0x00,0x2D,0x00,0x4C,0x00,0x49,0x00,0x4E,0x00,0x4B,0x00,0x5F};//16
    memcpy(&disp_buff[DAT_DISP_BASE],hilinkname,16);
    itoa_forutf16(host_info.mac[4],(char *)&disp_buff[DAT_DISP_BASE+16],16,2);   
    itoa_forutf16(host_info.mac[5],(char *)&disp_buff[DAT_DISP_BASE+20],16,2);  
    log_reverse_array((char *)&disp_buff[DAT_DISP_BASE+16], 2);
    log_reverse_array((char *)&disp_buff[DAT_DISP_BASE+18], 2);
    log_reverse_array((char *)&disp_buff[DAT_DISP_BASE+20], 2);
    log_reverse_array((char *)&disp_buff[DAT_DISP_BASE+22], 2);
    disp_len = DAT_DISP_BASE+24;
    send_buff(DISP_IPCONFILCT_ID);
}

void reset_data_disp(uint8_t second){
    char resetchar[]={0x4E,0x3B,0x67,0x3A,0x6B,0x63,0x57,0x28,0x60,0x62,0x59,0x0D,0x65,0x70,0x63,0x6E,0x00,0x20}; // 18 len ,0x00,0x25}; //正在复位主机数据 
    memcpy(&disp_buff[DAT_DISP_BASE],resetchar,18);
    disp_len = DAT_DISP_BASE+18;
    disp_buff[disp_len]=0;
    disp_buff[disp_len+1]=0x30+(second/10);
    disp_buff[disp_len+2]=0;
    disp_buff[disp_len+3]=0x30+(second%10);
    disp_buff[disp_len+4]=0;
    disp_buff[disp_len+5]=0x25;
    disp_len+=6;
    send_buff(DISP_TASKINFO1_ID);
    disp_buff[DAT_DISP_BASE]=0x00;
    disp_buff[DAT_DISP_BASE+1]=0x00;
    disp_len = DAT_DISP_BASE+2;
    send_buff(DISP_TASKINFO2_ID);
}


