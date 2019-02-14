#include "user_lcd.h"
#include "user_xccode.h"
#include "list_instance.h"
#include "user_unti.h"
#include "list_contorl.h"
#include "task_decode.h"
#include "fl_buff_decode.h"
#include "debug_print.h"

#define DAT_DISP_BASE   (7)
//---------------------------------------------
// 日期显示
#define DISP_YEAR_ER    (DAT_DISP_BASE)    
#define DISP_YEAR_BAI   (DISP_YEAR_ER+2)
#define DISP_YEAR_SHI   (DISP_YEAR_BAI+2)
#define DISP_YEAR_GE    (DISP_YEAR_SHI+2)
#define DISP_YEAR_GANG  (DISP_YEAR_GE+2)
#define DISP_MONTH_SHI  (DISP_YEAR_GANG+2)
#define DISP_MONTH_GE   (DISP_MONTH_SHI+2)
#define DISP_MONTH_GANG (DISP_MONTH_GE+2)
#define DISP_DAY_SHI    (DISP_MONTH_GANG+2)
#define DISP_DAY_GE     (DISP_DAY_SHI+2)
#define DISP_DAY_SPEC   (DISP_DAY_GE+2)
#define DISP_WEEK_XING  (DISP_DAY_SPEC+2) //2
#define DISP_WEEK_QI    (DISP_WEEK_XING+2) //2
#define DISP_WEEK_DAY   (DISP_WEEK_QI+2)   //2

#define DISP_DATA_LEN   (DISP_WEEK_DAY+2)  
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
#define DISP_TIME_ID    2
#define DISP_DATA_ID    1
#define DISP_TASKNAME_ID 11
#define DISP_TASKTIME_ID 13
#define DISP_TASKDURA_ID 14
#define DISP_TASKPLAY_ID 15

#define DISP_IP_ID 7
#define DISP_GATE_ID 8
#define DISP_MASK_ID 6
#define DISP_MAC_ID  10

#define DISP_A_ID  4
#define DISP_B_ID  3
#define DISP_C_ID  5
#define DISP_D_ID  12

#define DISP_VER_ID 9

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
        debug_printf("%x ",disp_buff[i]);
    }
    debug_printf("\n");
    */
    user_uart_tx(disp_buff,disp_len);
}

void disp_unti(uint8_t id,uint8_t *dat_base){
    memcpy(&disp_buff[DAT_DISP_BASE],dat_base,10);
    disp_len = 10+DAT_DISP_BASE;
    send_buff(id);
}

void disp_time_unti(time_info_t *time_info,uint8_t *buff,uint8_t adr_base){
    buff[adr_base] = 0x00;
    buff[adr_base+1] = 0x30+time_info->hour/10;
    //
    buff[adr_base+2] = 0x00;
    buff[adr_base+3] = 0x30+time_info->hour%10;
    // 冒号
    buff[adr_base+4] = 0x00;
    buff[adr_base+5] = 0x3A;
    // 分
    buff[adr_base+6] = 0x00;
    buff[adr_base+7] = 0x30+time_info->minute/10;
    buff[adr_base+8] = 0x00;
    buff[adr_base+9] = 0x30+time_info->minute%10;
    // 冒号
    buff[adr_base+10] = 0x00;
    buff[adr_base+11] = 0x3A;
    // 秒
    buff[adr_base+12] = 0x00;
    buff[adr_base+13] = 0x30+time_info->second/10;
    // 
    buff[adr_base+14] = 0x00;
    buff[adr_base+15] = 0x30+time_info->second%10;
    //
    buff[adr_base+16] = 0x00;
    buff[adr_base+17] = 0x00;
}

uint8_t disp_task_char[12]={0x62,0x53,0x94,0xC3,0x4E,0xFB,0x52,0xA1,0x00,0x3A,0x00,0x00};
uint8_t disp_rttask_char[12]={0x5B,0x9A,0x65,0xF6,0x4E,0xFB,0x52,0xA1,0x00,0x3A,0x00,0x00};
//(定时)
uint8_t timetask_char[8] = {0x00,0x28,0x5B,0x9A,0x65,0xF6,0x00,0x29};
//(打铃)
uint8_t ringtask_char[8] = {0x00,0x28,0x62,0x53,0x94,0xC3,0x00,0x29};
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
    disp_unti(DISP_A_ID,nowtask_char);
    disp_unti(DISP_B_ID,begtime_char);
    disp_unti(DISP_C_ID,dura_char);
    disp_unti(DISP_D_ID,playing_char);
}

// 
uint8_t dispchange_futuref=0;
uint8_t dispchange_nowf=0;

void disp_task_info(uint32_t dura_time,uint8_t *name,time_info_t *time_info,uint8_t solu_id,uint8_t future_f){
    uint8_t inc=0;
    uint8_t adr_base;
    //------------------------------------------------------------------------------------
    //获取任务名
    for(inc=0;inc<DIV_NAME_NUM/2;inc++){
        g_sys_val.dispname_buff[g_sys_val.disp_num][inc*2] = name[inc*2+1];
        g_sys_val.dispname_buff[g_sys_val.disp_num][inc*2+1] = name[inc*2];
        if((g_sys_val.dispname_buff[g_sys_val.disp_num][inc*2]==0)&&(g_sys_val.dispname_buff[g_sys_val.disp_num][inc*2+1]==0))
            break;
    }
    // 判断打铃or定时任务
    if(solu_id==0xFF){
        memcpy(&g_sys_val.dispname_buff[g_sys_val.disp_num][inc*2],timetask_char,8);
    }
    else{
        memcpy(&g_sys_val.dispname_buff[g_sys_val.disp_num][inc*2],ringtask_char,8);
    }
    g_sys_val.dispname_buff[g_sys_val.disp_num][inc*2+8] = 0x00;
    g_sys_val.dispname_buff[g_sys_val.disp_num][inc*2+9] = 0x00;
    //--------------------------------------------------------------------------
    //获取开始时间
    disp_time_unti(time_info,g_sys_val.disptime_buff[g_sys_val.disp_num],0); 
    //获取持续时间
    uint8_t hour,minute,second;
    adr_base = 0;
    hour = dura_time/3600;
    minute = (dura_time%3600)/60;
    second = (dura_time%3600)%60;
    //小时            
    if(hour!=0){
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x00;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = hour/10+0x30;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x00;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = hour%10+0x30;
        adr_base++;
        //5C 0F 65 F6
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x5C;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x0F;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x65;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0xF6;
        adr_base++;
    }
    //分钟 
    if(minute!=0){
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x00;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = minute/10+0x30;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x00;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = minute%10+0x30;
        adr_base++;
        //52 06 94 9F 
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x52;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x06;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x94;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x9F;
        adr_base++;
    }
    //秒
    if(second!=0){
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x00;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = second/10+0x30;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x00;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = second%10+0x30;
        adr_base++;
        //79 D2
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x79;
        adr_base++;
        g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0xD2;
        adr_base++;
    }
    g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base] = 0x00;
    g_sys_val.dispdura_buff[g_sys_val.disp_num][adr_base+1] =0x00;
    //
    g_sys_val.disp_furef[g_sys_val.disp_num] = future_f;
    //音乐
    g_sys_val.dispmusic_buff[g_sys_val.disp_num][0] = 0x65;
    g_sys_val.dispmusic_buff[g_sys_val.disp_num][1] = 0xE0;
    g_sys_val.dispmusic_buff[g_sys_val.disp_num][2] = 0x00;
    g_sys_val.dispmusic_buff[g_sys_val.disp_num][3] = 0x00;
    
}

void user_disptask_refresh(){
    g_sys_val.disp_num = 0;
    //---------------------------------------------------------------------------------------------
    // 4个现在任务
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if(timetask_now.ch_state[i]!=0xFF){
            g_sys_val.disp_ch[g_sys_val.disp_num] = i; 
            disp_task_info(timetask_now.task_musicplay[i].dura_time,
                           timetask_now.task_musicplay[i].name,
                           &timetask_now.task_musicplay[i].time_info,
                           timetask_now.task_musicplay[i].sulo_id,
                           0);
            g_sys_val.disp_num++;
        }
    }
    // 未来任务
    timetask_t *today_t_p = timetask_list.today_timetask_head;
    for(uint8_t i=0;i<2;i++){
        if(today_t_p==null)
            break;
        timer_task_read(&tmp_union.task_allinfo_tmp,today_t_p->id);  
        disp_task_info(tmp_union.task_allinfo_tmp.task_coninfo.dura_time,
                       tmp_union.task_allinfo_tmp.task_coninfo.task_name,
                       &tmp_union.task_allinfo_tmp.task_coninfo.time_info,
                       tmp_union.task_allinfo_tmp.task_coninfo.solution_sn,
                       1);
        g_sys_val.disp_num++;
        today_t_p = today_t_p->today_next_p;
    }
    if (g_sys_val.disp_num==0){
        // 没有任务
        memcpy(g_sys_val.dispname_buff[0],none_task_char,10);
        g_sys_val.disptime_buff[0][0] =0x65;
        g_sys_val.disptime_buff[0][1] =0xE0;
        g_sys_val.disptime_buff[0][2] =0x00;
        g_sys_val.disptime_buff[0][3] =0x00;
        //
        g_sys_val.dispdura_buff[0][0] =0x65;
        g_sys_val.dispdura_buff[0][1] =0xE0;
        g_sys_val.dispdura_buff[0][2] =0x00;
        g_sys_val.dispdura_buff[0][3] =0x00;
        //
        g_sys_val.dispmusic_buff[0][0] =0x65;
        g_sys_val.dispmusic_buff[0][1] =0xE0;
        g_sys_val.dispmusic_buff[0][2] =0x00;
        g_sys_val.dispmusic_buff[0][3] =0x00;
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
    uint16_t adr_base;
    if(g_sys_val.disp_furef[state])        
        disp_unti(DISP_A_ID,future_play_char);
    else
        disp_unti(DISP_A_ID,nowtask_char);
    //----------------------------------------------------------------------------------------
    // 任务名
    memcpy(&disp_buff[DAT_DISP_BASE],g_sys_val.dispname_buff[state],DIV_NAME_NUM+8);
    disp_len = DAT_DISP_BASE+DIV_NAME_NUM+8;
    send_buff(DISP_TASKNAME_ID);
    // 开始时间
    memcpy(&disp_buff[DAT_DISP_BASE],g_sys_val.disptime_buff[state],DIV_NAME_NUM);
    disp_len = DAT_DISP_BASE+DIV_NAME_NUM;
    send_buff(DISP_TASKTIME_ID);
    // 持续时间
    memcpy(&disp_buff[DAT_DISP_BASE],g_sys_val.dispdura_buff[state],DIV_NAME_NUM);
    disp_len = DAT_DISP_BASE+DIV_NAME_NUM;
    send_buff(DISP_TASKDURA_ID);
    // 播放音乐
    memcpy(&disp_buff[DAT_DISP_BASE],g_sys_val.dispmusic_buff[state],MUSIC_NAME_NUM);
    disp_len = DAT_DISP_BASE+DIV_NAME_NUM;
    send_buff(DISP_TASKPLAY_ID);

    /*
    //----------------------------------------------------------------------------------------
    if((state==0)||(dispchange_futuref==1)){
        // 未来任务1
        memcpy(&disp_buff[DAT_DISP_BASE],g_sys_val.dispfuture_buff[state*2],DIV_NAME_NUM*2);
        adr_base = DAT_DISP_BASE+DIV_NAME_NUM*2;
        //    
        //debug_printf("task 3:");
        //for(uint8_t i=0;i<50;i++)
        //    debug_printf("%x ",disp_buff[i]);
        //debug_printf("\n");
        //
        send_buff(DISP_FUTURE_TASK1_ID);
        //----------------------------------------------------------------------------------------
        // 未来任务2
        memcpy(&disp_buff[DAT_DISP_BASE],g_sys_val.dispfuture_buff[state*2+1],DIV_NAME_NUM*2);
        adr_base = DAT_DISP_BASE+DIV_NAME_NUM*2;
        //
        //debug_printf("task 4:");
        //for(uint8_t i=0;i<50;i++)
        //    debug_printf("%x ",disp_buff[i]);
        //debug_printf("\n");
        //
        send_buff(DISP_FUTURE_TASK2_ID);
        //----------------------------------------------------------------------------------------
    }
    */
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
    if((zero_f)||(g_sys_val.tmp_union.buff[*base_adr+1]!=0x30))
        *base_adr+=1;    
    disp_buff[*base_adr] = data%10+0x30;
    //disp_buff[*base_adr] = 0x00; 
    *base_adr+=1;
    //    
}

void user_disp_ip(xtcp_ipconfig_t ipconfig){
    uint8_t base_adr = DISP_IP_BASE;
    disp_buff[base_adr] = 0x49;
    disp_buff[base_adr+1] = 0x50;
    disp_buff[base_adr+2] = 0x3A;
    base_adr+=3;    
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
    disp_buff[base_adr] = 0x47;
    disp_buff[base_adr+1] = 0x41;
    disp_buff[base_adr+2] = 0x54;
    disp_buff[base_adr+3] = 0x45;
    disp_buff[base_adr+4] = 0x3A;   
    base_adr+=5;    
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
    //----------------------------------------------------------------\
    disp_len = base_adr;
    send_buff(DISP_GATE_ID);
    //---------------------------------------------------------------
    // 显示掩码       4D 41 53 4B 
    base_adr = DISP_IP_BASE;
    disp_buff[base_adr] = 0x4D;
    disp_buff[base_adr+1] = 0x41;
    disp_buff[base_adr+2] = 0x53;
    disp_buff[base_adr+3] = 0x4B;
    disp_buff[base_adr+4] = 0x3A;   
    base_adr+=5;    
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
    //----------------------------------------------------------------\
    disp_len = base_adr;
    send_buff(DISP_MASK_ID);
    //---------------------------------------------------------------
    // 显示MAC    4D 41 43 3A 
    base_adr = DISP_IP_BASE;
    disp_buff[base_adr] = 0x4D;
    disp_buff[base_adr+1] = 0x41;
    disp_buff[base_adr+2] = 0x43;
    disp_buff[base_adr+3] = 0x3A;  
    base_adr+=4;    
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
    //----------------------------------------------------------------\
    disp_len = base_adr-1;
    send_buff(DISP_MAC_ID);
}


uint8_t week_day[16]={0X00,0X00,0x4E,0x00,0x4E,0x8C,0x4E,0x09,0x56,0xDB,0x4E,0x94,0x51,0x6D,0x65,0xE5};

void user_disp_time(){
    //
    disp_time_unti(&g_sys_val.time_info,disp_buff,DAT_DISP_BASE);
    disp_len = DAT_DISP_BASE+16;
    send_buff(DISP_TIME_ID);
}

void user_disp_data(){
    // year
    disp_buff[DISP_YEAR_ER] = 0x00;
    disp_buff[DISP_YEAR_ER+1] = 0x32;
    //
    disp_buff[DISP_YEAR_BAI] = 0x00; 
    disp_buff[DISP_YEAR_BAI+1] = 0x30 + g_sys_val.date_info.year/100;
    //
    disp_buff[DISP_YEAR_SHI] = 0x00;
    disp_buff[DISP_YEAR_SHI+1] = 0x30 + (g_sys_val.date_info.year/10)%10;
    //
    disp_buff[DISP_YEAR_GE] = 0x00;
    disp_buff[DISP_YEAR_GE+1] = 0x30 + g_sys_val.date_info.year%10;
    // - 
    disp_buff[DISP_YEAR_GANG] = 0x00;
    disp_buff[DISP_YEAR_GANG+1] = 0x2D;
    // month
    disp_buff[DISP_MONTH_SHI] = 0x00;
    disp_buff[DISP_MONTH_SHI+1] = 0x30+g_sys_val.date_info.month/10;
    //
    disp_buff[DISP_MONTH_GE] = 0x00;
    disp_buff[DISP_MONTH_GE+1] = 0x30+g_sys_val.date_info.month%10;
    // -
    disp_buff[DISP_MONTH_GANG] = 0x00;
    disp_buff[DISP_MONTH_GANG+1] = 0x2D;
    // day
    disp_buff[DISP_DAY_SHI] = 0x00;
    disp_buff[DISP_DAY_SHI+1] = 0x30+g_sys_val.date_info.date/10;
    //
    disp_buff[DISP_DAY_GE] = 0x00;
    disp_buff[DISP_DAY_GE+1] = 0x30+g_sys_val.date_info.date%10;
    // 空格
    disp_buff[DISP_DAY_SPEC] = 0x00;
    disp_buff[DISP_DAY_SPEC+1] = 0x20;
    // 星  
    disp_buff[DISP_WEEK_XING] = 0x66;
    disp_buff[DISP_WEEK_XING+1] = 0x1F;
    // 期
    disp_buff[DISP_WEEK_QI] = 0x67;
    disp_buff[DISP_WEEK_QI+1] = 0X1F;
    // x
    disp_buff[DISP_WEEK_DAY] = week_day[g_sys_val.date_info.week*2];
    disp_buff[DISP_WEEK_DAY+1] = week_day[g_sys_val.date_info.week*2+1];
    
    //
    disp_len = DISP_DATA_LEN;
    send_buff(DISP_DATA_ID);
}

void user_disp_version(){
    
    disp_buff[DAT_DISP_BASE] = 0x56;
    disp_buff[DAT_DISP_BASE+1] = 0x30+(VERSION_H>>4);
    disp_buff[DAT_DISP_BASE+2] = 0x30+(VERSION_H&0x0f);
    
    disp_buff[DAT_DISP_BASE+3] = 0x2E;
    disp_buff[DAT_DISP_BASE+4] = 0x30+(VERSION_L>>4);
    disp_buff[DAT_DISP_BASE+5] = 0x30+(VERSION_L&0x0f);
    //
    disp_len = DAT_DISP_BASE+6;
    send_buff(DISP_VER_ID);
}


