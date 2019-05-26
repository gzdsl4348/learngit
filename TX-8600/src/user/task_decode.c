#include "task_decode.h"
#include "list_instance.h"
#include "list_contorl.h"
#include "user_unti.h"
#include "fl_buff_decode.h"
#include "user_xccode.h"
#include "ack_build.h"
#include "protocol_adrbase.h"
#include "music_play.h"
#include "debug_print.h"
#include "user_messend.h"
#include "user_lcd.h"
#include "conn_process.h"

extern uint8_t f_name[];

#ifndef null
#define null -1
#endif

//----------------------------------------------------------
// 定时任务列表初始化
void task_fl_init(){
    tmp_union.task_allinfo_tmp.task_coninfo.task_id=0xFFFF;
    tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum=0x00;
    tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum = 0x00;
    for(uint16_t i=0; i<MAX_HOST_TASK; i++){
        timer_task_write(&tmp_union.task_allinfo_tmp,i);
    }
}
//---------------------------------------------------------
// 获取定时任务列表基本信息
void timer_tasklist_read(){
    timetask_list.all_timetask_head=null;
    timetask_list.all_timetask_end=null;
    timetask_list.today_timetask_head=null;
    timetask_list.today_timetask_end=null;
    timetask_list.task_total=0;
    //
    for(uint16_t i=1; i<MAX_HOST_TASK; i++){
        timetask_list.timetask[i].all_next_p=null;
        timetask_list.timetask[i].today_next_p=null;
    }
    //
    for(uint16_t i=1;i<MAX_HOST_TASK;i++){
        timer_task_read(&tmp_union.task_allinfo_tmp,i);
        timetask_list.timetask[i].id = tmp_union.task_allinfo_tmp.task_coninfo.task_id;
        //判断错误任务 复位任务
        if((timetask_list.timetask[i].id!=0xFFFF)&&(timetask_list.timetask[i].id != i)){
            //xtcp_debug_printf("write task %d\n",timetask_list.timetask[i].id);
            tmp_union.task_allinfo_tmp.task_coninfo.task_id=0xFFFF;
            tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum=0x00;
            tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum = 0x00;
            //
            timetask_list.timetask[i].id = 0xFFFF;
            timer_task_write(&tmp_union.task_allinfo_tmp,i);
        }
        timetask_list.timetask[i].task_en = tmp_union.task_allinfo_tmp.task_coninfo.task_state;
        timetask_list.timetask[i].solu_id = tmp_union.task_allinfo_tmp.task_coninfo.solution_sn;
        timetask_list.timetask[i].repe_mode = tmp_union.task_allinfo_tmp.task_coninfo.task_repe_mode;
        timetask_list.timetask[i].time_info = tmp_union.task_allinfo_tmp.task_coninfo.time_info;
        timetask_list.timetask[i].week_repebit = tmp_union.task_allinfo_tmp.task_coninfo.week_repebit;
        //
        for(uint8_t j=0; j<MAX_TASK_DATE_NUM; j++)
            timetask_list.timetask[i].date_info[j] = tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[j];       
    }
    // 现在任务初始化
    for(uint8_t i=0;i<MAX_MUSIC_NUM;i++){
        timetask_now.ch_state[i] = 0xFF;
    }
    //       
}
//---------------------------------------------

//--------------------------------------------------------------------------------------------
// 所有任务列表生成
void create_alltask_list(){
    for(uint16_t i=1; i<MAX_HOST_TASK; i++){
        if(timetask_list.timetask[i].id!=0xFFFF){
            // 增加方案判断
            if((timetask_list.timetask[i].solu_id!=0xFF)&&(solution_list.solu_info[timetask_list.timetask[i].solu_id].state==0xFF)){
                timetask_list.timetask[i].id = 0xFFFF; //复位任务
                continue;
            }
            create_task_node_forid(timetask_list.timetask[i].id); //建立一个任务节点 
        }
    }
    #if 0
    timetask_t *tmp_p;
    tmp_p = timetask_list.all_timetask_head;
    while(tmp_p!=null){
        xtcp_debug_printf("creat task id %d\n",tmp_p->id);
        tmp_p = tmp_p->all_next_p;
    }
    #endif
}

//---------------------------------------------------------------
// 关闭音频模块播放
void task_music_config_stop(uint8_t ch){
    timetask_now.ch_state[ch] = 0xFF;
    task_music_stop(ch);
    user_disptask_refresh();
}

void task_music_stop_all(){
    for(uint8_t i=0; i<MAX_MUSIC_CH;i++){
        if(timetask_now.ch_state[i]!=0xFF){
            task_music_config_stop(i);
        }
    }
}

//-------------------------------------------------------------------------------------------
// 曲目事件切换
uint8_t random_music_tab[MAX_MUSIC_NUM] = {17,8,3,0,16,12,19,15,4,18,7,13,2,6,1,10,5,9,11,14};
uint8_t random_inc[MAX_MUSIC_CH] = {0};

uint8_t random_play_inc(uint8_t ch){
    for(uint8_t i=0;i<MAX_MUSIC_NUM;i++){
        random_inc[ch]++;
        if(random_inc[ch]>=MAX_MUSIC_NUM )
            random_inc[ch] =0;
        if(random_music_tab[random_inc[ch]] < timetask_now.task_musicplay[ch].music_tol){
            return random_music_tab[random_inc[ch]];
        } 
    }
    return 0;
}

void task_musicevent_change(uint8_t ch,char event,char data){
    static uint8_t music_inc[MAX_MUSIC_CH] = {0};
    
    if(timetask_now.ch_state[ch]==0xFF)
        return;
    if(g_sys_val.play_error_inc[ch]>=timetask_now.task_musicplay[ch].music_tol){
        timetask_now.ch_state[ch]=0xFF;
        user_audio_send_dis(ch);
        user_disptask_refresh();
        return;
    }    
    timetask_now.task_musicplay[ch].music_inc++;
    //timetask_now.task_musicplay[ch].play_mode = LOOP_PLAY_M;
    //xtcp_debug_printf("play mode %d\n", timetask_now.task_musicplay[ch].play_mode);
    switch(timetask_now.task_musicplay[ch].play_mode){
        case ORDER_PLAY_M:
            if(timetask_now.task_musicplay[ch].music_inc == timetask_now.task_musicplay[ch].music_tol){
                timetask_now.ch_state[ch] = 0xFF;
                user_audio_send_dis(ch);
                user_disptask_refresh();
                //task_music_config_stop(ch);
                return;
            }
            music_inc[ch] = timetask_now.task_musicplay[ch].music_inc;
            //xtcp_debug_printf("play mode %d  music num %d\n",timetask_now.task_musicplay[ch].play_mode,
            //                                            music_inc[ch]);
            break;
        case LOOP_PLAY_M:
            //xtcp_debug_printf("loop mic tol %d  mic inc %d ch%d\n",timetask_now.task_musicplay[ch].music_tol,timetask_now.task_musicplay[ch].music_inc,ch);
            if(timetask_now.task_musicplay[ch].music_inc == timetask_now.task_musicplay[ch].music_tol){
                timetask_now.task_musicplay[ch].music_inc=0;
            }
            music_inc[ch] = timetask_now.task_musicplay[ch].music_inc;
            break;
        case RANDOM_PLAY_M:
            music_inc[ch] = random_play_inc(ch);
            break;
    }
    //切换音乐
    timer_task_read(&tmp_union.task_allinfo_tmp,timetask_now.task_musicplay[ch].task_id);
    if(tmp_union.task_allinfo_tmp.task_coninfo.task_id!=0xFFFF)
        task_music_play(ch,music_inc[ch]);
    else{
        timetask_now.ch_state[ch] = 0xFF;
        user_audio_send_dis(ch);
        user_disptask_refresh();
    }
}

//-------------------------------------------------------------------------------------------
// 1Shz 播放计时轮询
void timer_taskmusic_check(){
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if(timetask_now.ch_state[i]!=0xFF){
            timetask_now.task_musicplay[i].time_inc++;
            if(timetask_now.task_musicplay[i].time_inc>=timetask_now.task_musicplay[i].dura_time){
                task_music_config_stop(i);
                user_disptask_refresh();
                                    
                // 任务信息更新
            	g_sys_val.task_config_s = 2; //任务编辑
            	g_sys_val.task_con_id = timetask_now.task_musicplay[i].task_id;
                mes_send_taskinfo(&tmp_union.task_allinfo_tmp);
            }
        }
    }
}
//---------------------------------------------------------------------------------------------------------------
// 配置发送目标与任务参数
void task_music_config_play(uint8_t ch,uint16_t id){
    xtcp_debug_printf("play task id %d,%d\n",id,ch);
    g_sys_val.play_error_inc[ch] = 0;
    timer_task_read(&tmp_union.task_allinfo_tmp,id);
    // 置通道播放状态
    timetask_now.ch_state[ch]=ch;
    // 初始化现在任务状态
    timetask_now.task_musicplay[ch].task_id = id;
    timetask_now.task_musicplay[ch].dura_time = tmp_union.task_allinfo_tmp.task_coninfo.dura_time; //获得持续时间
    timetask_now.task_musicplay[ch].play_mode = tmp_union.task_allinfo_tmp.task_coninfo.play_mode; //获得播放模式
    timetask_now.task_musicplay[ch].music_tol = tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum;   //音乐总数 
    timetask_now.task_musicplay[ch].time_inc = 0;
    timetask_now.task_musicplay[ch].music_inc = 0;
    memcpy(timetask_now.task_musicplay[ch].name,tmp_union.task_allinfo_tmp.task_coninfo.task_name,DIV_NAME_NUM);
    timetask_now.task_musicplay[ch].time_info = tmp_union.task_allinfo_tmp.task_coninfo.time_info;
    // 播放任务
    task_music_send(ch);
    // 随机模式处理
    uint8_t tmp_inc;
    tmp_inc = timetask_now.task_musicplay[ch].music_inc;
    if(timetask_now.task_musicplay[ch].play_mode == RANDOM_PLAY_M){
        random_inc[ch] = g_sys_val.time_info.second%20;
        tmp_inc = random_play_inc(ch);
    }
    user_disptask_refresh();
    task_music_play(ch,tmp_inc);
}

//-------------------------------------------------------------------------------------------
// 现在任务查询与播放
void task_check_and_play(){
    for(uint8_t i=0;i<MAX_HOST_TASK;i++){
        if(timetask_list.today_timetask_head==null)
            break;
        if((timetask_list.today_timetask_head->time_info.hour == g_sys_val.time_info.hour)&&
           (timetask_list.today_timetask_head->time_info.minute == g_sys_val.time_info.minute)&&
           (timetask_list.today_timetask_head->time_info.second == g_sys_val.time_info.second)
        ){
            //--------------------------------------------------------------
            // 置为现在播放
            uint8_t play_num;
            play_num = i%MAX_MUSIC_CH;
            //
            if(g_sys_val.file_bat_contorl_s==0){
                g_sys_val.music_task_id[play_num] = timetask_list.today_timetask_head->id;
                g_sys_val.task_wait_state[play_num] = 1;
                g_sys_val.play_ok = 1;
            }
            //--------------------------------------------------------------
            // 切换   下一个今日任务
            timetask_list.today_timetask_head = timetask_list.today_timetask_head->today_next_p;
            if(g_sys_val.file_bat_contorl_s){
                user_disptask_refresh();
            }
            continue;
        }
        // 没有任务执行
        break;
    }//for
}

void task_10hz_mutich_play(){
    if(!g_sys_val.play_ok)
        return;
    g_sys_val.play_ok = 0;
    for(uint8_t j=0;j<MAX_MUSIC_CH;j++){
        if(g_sys_val.task_wait_state[j]){
            g_sys_val.task_wait_state[j] = 0;
            for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
                if(timetask_now.ch_state[i]==0xFF){
                    xtcp_debug_printf("play task:%d\n",g_sys_val.music_task_id[j]);
                    //task_music_config_play(i,g_sys_val.music_task_id[j]);
                    task_music_config_play(i,g_sys_val.music_task_id[j]);
                    // 任务信息更新
                	g_sys_val.task_config_s = 2; //任务编辑
                	g_sys_val.task_con_id = g_sys_val.music_task_id[j];
                    mes_send_taskinfo(&tmp_union.task_allinfo_tmp);
                    //--------------------------------------------------------------------------------
                    break;
                }
            }
            break;
        }
    }
}


//-------------------------------------------------------------------------------------------
// 当天任务列表生成
void create_todaytask_list(time_info_t time_info){
    timetask_t *task_p;
    timetask_t *today_t_p;
    timetask_t *last_today_p;
    unsigned beg_time;
    unsigned task_time;
    unsigned time_tmp;
    uint8_t tmp_f;
    //
    beg_time = (time_info.hour<<16)|(time_info.minute<<8)|time_info.second;
    task_p = timetask_list.all_timetask_head;
    timetask_list.today_timetask_head = null;
    //
    while(task_p!=null){
        tmp_f=0;
        if(task_p->repe_mode){//判断日期
            for(uint8_t i=0;i<MAX_TASK_DATE_NUM;i++){
                #if 0
                xtcp_debug_printf("year %d %d month %d %d day %d %d \n", task_p->date_info[i].year,g_sys_val.date_info.year,
                                                                    task_p->date_info[i].month,g_sys_val.date_info.month,
                                                                    task_p->date_info[i].date,g_sys_val.date_info.date);
                #endif
                if((task_p->date_info[i].date==g_sys_val.today_date.date)&&
                   (task_p->date_info[i].month==g_sys_val.today_date.month)&&
                   (task_p->date_info[i].year==g_sys_val.today_date.year)){
                        tmp_f=1;
                        break;
                   }
            }
        }
        else if((task_p->week_repebit>>(g_sys_val.today_date.week-1))&0x01){ //判断星期
            tmp_f=1;
        }
        // 判断方案与任务使能 
        if((!task_p->task_en)||((task_p->solu_id!=0xFF)&&(!solution_list.solu_info[task_p->solu_id].data_en))){
            tmp_f=0;
        }
        task_time = (task_p->time_info.hour<<16)|(task_p->time_info.minute<<8)|task_p->time_info.second; //新任务
        //
        // 所有任务里找出今日任务 将今日任务加入今日任务链表
        //if(tmp_f)
        //    xtcp_debug_printf("id %d,taskt %x,begt %x tasken %d  soluid %d soluen %d day mode %d today %d\n",task_p->id,task_time,beg_time,task_p->task_en,task_p->solu_id,solution_list.solu_info[task_p->solu_id].en,task_p->week_repebit,tmp_f);
        //for(uint8_t i=0;i<10;i++){
        //    xtcp_debug_printf("data %d,%d,%d\n",task_p->date_info[i].date,task_p->date_info[i].month,task_p->date_info[i].year);
        //}
        if((tmp_f)&&(task_time >= beg_time)){//判断时间
            //xtcp_debug_printf("today time right\n");
            task_p->today_next_p=null;
            today_t_p = timetask_list.today_timetask_head;
            //获取第一个任务
            if(today_t_p==null){    
                timetask_list.today_timetask_head = task_p; 
                today_t_p = task_p;
            }
            else{
                while(today_t_p!=null){
                    time_tmp = (today_t_p->time_info.hour<<16)|(today_t_p->time_info.minute<<8)|today_t_p->time_info.second;
                    if(task_time<time_tmp){  //插入任务 时间比较
                        if(today_t_p==timetask_list.today_timetask_head){ //插到第一个任务
                            timetask_list.today_timetask_head = task_p; //更新头任务
                            task_p->today_next_p = today_t_p;
                        }
                        else{ //插到中间
                            last_today_p->today_next_p = task_p;
                            task_p->today_next_p = today_t_p;
                        }
                        break;//退出while查找
                    }
                    last_today_p = today_t_p;
                    if(today_t_p->today_next_p == null){ // 尾插任务
                        today_t_p->today_next_p = task_p;
                        break;
                    }
                    today_t_p = today_t_p->today_next_p;
                }
            }
        }
        task_p = task_p->all_next_p;    
    }
    #if 1
    timetask_t *tmp_p = timetask_list.today_timetask_head;
    while(tmp_p!=null){
        xtcp_debug_printf("today task id %d\n",tmp_p->id);
        tmp_p = tmp_p->today_next_p;
    }
    #endif
    //-------------------------------------------------------------------------------------
    // 初始化现在任务
    task_check_and_play();
    user_disptask_refresh();
}
//====================================================================================================
// 方案查询
//====================================================================================================
void solution_check_recive(){
    user_sending_len = solution_list_ack_build(SOLUTION_CHECK_CMD,0);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    //xtcp_debug_printf("solution check\n");
}

//====================================================================================================
// 任务列表查询
//====================================================================================================
void task_check_recive(){
    uint8_t list_num = list_sending_init(TASK_CHECK_CMD,TASK_LIST_SENDING,&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE]);
    if(list_num == LIST_SEND_INIT)
        return;
    // 特殊处理
    t_list_connsend[list_num].list_info.tasklist.solu_en=0; //查找所有任务
    t_list_connsend[list_num].list_info.tasklist.solu_id=0;
    //
    t_list_connsend[list_num].list_info.tasklist.task_tol = timetask_list.task_total/MAX_TASK_ONCESEND;
    if(timetask_list.task_total%MAX_TASK_ONCESEND || t_list_connsend[list_num].list_info.tasklist.task_tol==0)
        t_list_connsend[list_num].list_info.tasklist.task_tol++;
    //
	if(g_sys_val.list_sending_f==0){
		g_sys_val.list_sending_f = 1;
	    user_sending_len = task_list_ack_build(TASK_CHECK_CMD,0,0,list_num);
	    user_xtcp_send(conn,t_list_connsend[list_num].could_s);
	}
}
//
//--------------------------------------------
// 任务列表连发处理
void tasklist_sending_decode(uint8_t list_num){
    user_sending_len = task_list_ack_build(t_list_connsend[list_num].list_info.tasklist.cmd,
                                           t_list_connsend[list_num].list_info.tasklist.solu_en,
                                           t_list_connsend[list_num].list_info.tasklist.solu_id,
                                           list_num);
    user_xtcp_send(t_list_connsend[list_num].conn,t_list_connsend[list_num].could_s);
    //xtcp_debug_printf("tasklist_sending\n");
}

//====================================================================================================
// 获取定时任务详细信息协议 B304
//====================================================================================================
void task_dtinfo_check_recive(){
    uint8_t list_num = list_sending_init(TASK_DTINFO_CK_CMD,TASK_DTINFO_SENDING,&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE]);
    if(list_num == LIST_SEND_INIT)
        return;
    //
    uint16_t id = xtcp_rx_buf[TASK_DTG_TASK_ID+1]<<8 |xtcp_rx_buf[TASK_DTG_TASK_ID];
    if(id>MAX_HOST_TASK)
        return;
    //
	if(g_sys_val.list_sending_f==0){
		g_sys_val.list_sending_f = 1;
	    timer_task_read(&tmp_union.task_allinfo_tmp,id);
	    //
	    user_sending_len = task_dtinfo_chk_build(list_num);
	    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
	}
    xtcp_debug_printf("dtchk task %d\n",id);
}

//----------------------------------------------
// 详细信息连发处理
void task_dtinfo_decode(uint8_t list_num){
    timer_task_read(&tmp_union.task_allinfo_tmp,t_list_connsend[list_num].list_info.task_dtinfo.task_id);
    user_sending_len = task_dtinfo_chk_build(list_num);
    user_xtcp_send(t_list_connsend[list_num].conn,t_list_connsend[list_num].could_s);
    //xtcp_debug_printf("task dtinfo send\n");
}

//====================================================================================================
// 方案日期判断
// @solu_mode :0  判断是否开启方案 
//            :1  判断开启或关闭方案
//====================================================================================================
void solution_data_chk(uint8_t id){
    //
    unsigned solu_begdate = (solution_list.solu_info[id].begin_date.year<<16)|
                            (solution_list.solu_info[id].begin_date.month<<8)|
                            (solution_list.solu_info[id].begin_date.date);
    //
    unsigned solu_enddate = (solution_list.solu_info[id].end_date.year<<16)|
                            (solution_list.solu_info[id].end_date.month<<8)|
                            (solution_list.solu_info[id].end_date.date);
    //
    unsigned sys_date = (g_sys_val.date_info.year<<16)|
                        (g_sys_val.date_info.month<<8)|
                        (g_sys_val.date_info.date);
    
    solution_list.solu_info[id].data_en = 0;
    if((solution_list.solu_info[id].state!=0xFF)&&(solution_list.solu_info[id].en)){
        if((solu_begdate<=sys_date)&&(solu_enddate>=sys_date)){
            solution_list.solu_info[id].data_en = 1;
        }
    }
}


//====================================================================================================
// 配置定时方案信息协议   B305
//====================================================================================================
void solution_config_recive(){
    uint8_t state=0;
    //获取id
    uint8_t id = xtcp_rx_buf[SOLU_CFG_SOLU_ID];
    //xtcp_debug_printf("solution config %d\n",id);
    //创建方案
    if((xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL]==0)||(xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL]==3)){
        id = 0xFF;
        for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
            if(solution_list.solu_info[i].state==0xFF){
                solution_list.solu_info[i].state=i; 
                solution_list.solu_info[i].id=i; 
                xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT] = 0xFF;
                id = i;
                break;
            }
        }
    }
    if((id>MAX_TASK_SOULTION)){     
        state = 1; //方案已满
        goto solution_config_end;
    }
    //删除方案
    if(xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL]==1){
        // 删除方案
        solution_list.solu_info[id].state=0xFF; 
        xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT] = 0xFF;
        // 删除方案里的所有任务。
        timetask_t *task_p;
        uint16_t tmp_id;
        task_p = timetask_list.all_timetask_head;
        // 扫描所有任务
        while(task_p!=null){
            if(task_p->solu_id == id){
                tmp_id = task_p->id;
                task_p = task_p->all_next_p;
                timer_task_read(&tmp_union.task_allinfo_tmp,tmp_id);
                delete_task_node(tmp_id);
                timetask_list.timetask[tmp_id].id=0xFFFF;
                tmp_union.task_allinfo_tmp.task_coninfo.task_id =0xFFFF;
                timer_task_write(&tmp_union.task_allinfo_tmp,tmp_id);
                continue;
            }
            task_p = task_p->all_next_p;
        }
    }
    timetask_t *task_p;
    timetask_t *task_end_p;
    //克隆方案
    if(xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL]==3){
        //所有任务里找出当前方案任务
        task_p = timetask_list.all_timetask_head;
        task_end_p = timetask_list.all_timetask_end;
        while((task_p!=null)){
            // 判断克隆任务
            if(task_p->solu_id == xtcp_rx_buf[SOLU_CFG_SOLU_ID]){
                // 获取需克隆的任务
                timer_task_read(&tmp_union.task_allinfo_tmp,task_p->id);
                // 创建任务并克隆
                if(create_task_node()){ //任务添加成功   
                    tmp_union.task_allinfo_tmp.task_coninfo.task_id = timetask_list.all_timetask_end->id;
                    tmp_union.task_allinfo_tmp.task_coninfo.solution_sn = id; 
                    timetask_list.timetask[timetask_list.all_timetask_end->id].solu_id = id;
                    timetask_list.timetask[timetask_list.all_timetask_end->id].task_en = tmp_union.task_allinfo_tmp.task_coninfo.task_state;
                    timetask_list.timetask[timetask_list.all_timetask_end->id].week_repebit = tmp_union.task_allinfo_tmp.task_coninfo.week_repebit;
                    timetask_list.timetask[timetask_list.all_timetask_end->id].repe_mode = tmp_union.task_allinfo_tmp.task_coninfo.task_repe_mode;
                    memcpy(timetask_list.timetask[timetask_list.all_timetask_end->id].date_info,tmp_union.task_allinfo_tmp.task_coninfo.dateinfo,3*MAX_TASK_DATE_NUM);
                    timetask_list.timetask[timetask_list.all_timetask_end->id].time_info = tmp_union.task_allinfo_tmp.task_coninfo.time_info;
                    //
                    timer_task_write(&tmp_union.task_allinfo_tmp,timetask_list.all_timetask_end->id);
                }
                else{ // 任务已满 失败
                    state = 2;
                    break;
                }
            }
            if((task_p==task_end_p)){
                break;
            }
            task_p = task_p->all_next_p;
        }
    }
    // 方案配置
    //xtcp_debug_printf("con solu %x\n",xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT]);
    //
    //配置方案名称
    if((xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT]>>1)&1)
        memcpy(&solution_list.solu_info[id].name,&xtcp_rx_buf[SOLU_CFG_SOLU_NAME],DIV_NAME_NUM);
    //配置方案日期
    if((xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT]>>2)&1){
        memcpy(&solution_list.solu_info[id].begin_date,&xtcp_rx_buf[SOLU_CFG_SOLU_BEGDATE],3);
        memcpy(&solution_list.solu_info[id].end_date,&xtcp_rx_buf[SOLU_CFG_SOLU_ENDDATE],3);
    } 
    // 配置方案优先级
    solution_list.solu_info[id].prio = xtcp_rx_buf[SOLU_CFG_SOLU_PRIO]; 
    // 改变任务优先级
    
    task_p = timetask_list.all_timetask_head;
    while(task_p!=null){
        if(task_p->solu_id == id){
            timer_task_read(&tmp_union.task_allinfo_tmp,task_p->id);
            tmp_union.task_allinfo_tmp.task_coninfo.task_prio = solution_list.solu_info[id].prio;
            timer_task_write(&tmp_union.task_allinfo_tmp,task_p->id);
        }
        task_p = task_p->all_next_p;
    }

    // 配置方案使能
    if(xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT]&1){ 
        if(xtcp_rx_buf[SOLU_CFG_SOLU_STATE]){
            uint8_t max_open_solu=0;
            for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
                if((solution_list.solu_info[i].state!=0xFF)&&(solution_list.solu_info[i].en))
                    max_open_solu++;
            }
            if(max_open_solu<2)
                solution_list.solu_info[id].en = 1;
        }
        else{
            solution_list.solu_info[id].en = 0;
        }
    }
    //
    if(xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL]==3)
        solution_list.solu_info[id].en = 0;
    //
    solution_data_chk(id);
    g_sys_val.need_flash |= NEED_FL_SOLUTION;
solution_config_end:
    user_sending_len = solution_config_build(id,state,xtcp_rx_buf[SOLU_CFGACK_CONFIG]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    if(state==0){
        mes_send_suloinfo(id);
        taskview_page_messend();
    }
    create_todaytask_list(g_sys_val.time_info);
}

//====================================================================================================
// 配置定时任务信息协议 B306
//====================================================================================================
void task_config_recive(){
    uint16_t id;
    // 获得任务ID
    id = (xtcp_rx_buf[TASK_CFG_TASK_ID+1]<<8)|xtcp_rx_buf[TASK_CFG_TASK_ID];
    //xtcp_debug_printf("B306 id %d\n",id);
    //
    if(id>MAX_HOST_TASK-1)
        return;
    // 禁止任务提前处理
    if((xtcp_rx_buf[TASK_CFG_TASK_STATE]==0)&&((xtcp_rx_buf[TASK_CFG_CONTORL]==1)||(xtcp_rx_buf[TASK_CFG_CONTORL]==2))){
        for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
            if((timetask_now.ch_state[i]!=0xFF)&&(timetask_now.task_musicplay[i].task_id==id)){
                task_music_config_stop(i);
                // 重读数据
                //timer_task_read(&tmp_union.task_allinfo_tmp,id);
            }
        }
    }
    //
    uint8_t config_bit = xtcp_rx_buf[TASK_CFG_CFGBIT];
    // 取得任务信息
    //timer_task_read(&tmp_union.task_allinfo_tmp,id);
    //
    //xtcp_debug_printf("task config state %d\n",xtcp_rx_buf[TASK_CFG_CONTORL]);
    if(xtcp_rx_buf[TASK_CFG_CONTORL]==1){ //删除任务
        delete_task_node(id);
        timetask_list.timetask[id].id=0xFFFF;
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_id = 0xFFFF;
        xtcp_rx_buf[TASK_CFG_CFGBIT] = 0xFF;
        config_bit =0xFF;
        g_sys_val.task_delete_s = 1;
        g_sys_val.task_config_s = 1;
    }
    else if(xtcp_rx_buf[TASK_CFG_CONTORL]==0){ //添加任务
        g_sys_val.task_creat_s = 1;
        g_sys_val.task_config_s = 0;
        config_bit=0xFF;
        xtcp_rx_buf[TASK_CFG_CFGBIT] = 0xFF;
    }
    else if(xtcp_rx_buf[TASK_CFG_CONTORL]==3){ //克隆任务
        g_sys_val.task_creat_s = 1;
        g_sys_val.task_config_s = 2;
        config_bit=0x0;
        xtcp_rx_buf[TASK_CFG_CFGBIT] = 0xFF;
    }
    // 设置根据标志位设置信息
    //xtcp_debug_printf("task id %d\n",id);
    if(config_bit&1){    //任务名称
        //xtcp_debug_printf("con name\n");
        //for(uint8_t i=0;i<DIV_NAME_NUM;i++){
        //    xtcp_debug_printf("%x,",xtcp_rx_buf[TASK_CFG_TASK_NAME+i]);
        //}
        xtcp_debug_printf("\n");
        memcpy(g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_name,&xtcp_rx_buf[TASK_CFG_TASK_NAME],DIV_NAME_NUM);
    }
    config_bit>>=1;     //任务状态
    if(config_bit&1){
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_state = xtcp_rx_buf[TASK_CFG_TASK_STATE];
        //xtcp_debug_printf("task state %d\n",g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_state);
    }
    config_bit>>=1;
    if(config_bit&1){   //任务音量
        //xtcp_debug_printf("vol %d\n",xtcp_rx_buf[TASK_CFG_TASK_VOL]);
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_vol = xtcp_rx_buf[TASK_CFG_TASK_VOL];
    }
    config_bit>>=1;
    if(config_bit&1){   //任务重复模式
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_repe_mode = xtcp_rx_buf[TASK_CFG_REPE_MODE];
        //按日期重复
        //xtcp_debug_printf("config day:\n");
        for(uint8_t i=0;i<MAX_TASK_DATE_NUM;i++){
            memcpy(&g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[i],&xtcp_rx_buf[TASK_CFG_REPE_DATE+i*3],3);
        }
        //按周重复
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.week_repebit = xtcp_rx_buf[TASK_CFG_REPE_WEEK];
    }
    config_bit>>=1; //任务开始时间
    if(config_bit&1){
        memcpy(&g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.time_info,&xtcp_rx_buf[TASK_CFG_BEG_TIME],3);
        
    }
    config_bit>>=1; //任务持续时间
    if(config_bit&1){
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.dura_time = xtcp_rx_buf[TASK_CFG_DURA_TIME]*3600+xtcp_rx_buf[TASK_CFG_DURA_TIME+1]*60+xtcp_rx_buf[TASK_CFG_DURA_TIME+2];
    }
    config_bit>>=1; //播放设置
    if(config_bit&1){
        //xtcp_debug_printf("con play\n");
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.play_mode = xtcp_rx_buf[TASK_CFG_PLAY_MODE];
        //tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum = xtcp_rx_buf[TASK_CFG_PLAY_TOL];
    }
    config_bit>>=1; //方案设置
    if(config_bit&1){
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn = xtcp_rx_buf[TASK_CFG_SOLU_ID];
    }
    // 任务优先级
    if(g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn==0xFF){
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_prio = xtcp_rx_buf[TASK_CFG_TASK_PRIO];
    }
    else{ //打铃任务按方案优先级选择
        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_prio = AUX_I1;
        for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
            if((solution_list.solu_info[i].state!=0xFF)&&(solution_list.solu_info[i].id==xtcp_rx_buf[TASK_CFG_SOLU_ID])){
                g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_prio = solution_list.solu_info[i].prio;
            }
        }
    }
    //xtcp_debug_printf("task prio %d,%d\n", g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_prio,xtcp_rx_buf[TASK_CFG_SOLU_ID]);    
    
    // 配置失败
    if(xtcp_rx_buf[TASK_CFG_CFGBIT] == 0)
        g_sys_val.task_con_state |= 2;
    // 
}

//====================================================================================================
// 配置定时任务详细信息协议                     B307
//====================================================================================================
void task_dtinfo_config_recive(){
    //机器同一时间只能处理一种多包发送类型
    //if(conn_sending_s.id!=null)
    //    goto dtinfo_send_end;
    //xtcp_debug_printf("B407 ID : %d,pack tol %d inc %d\n",((xtcp_rx_buf[TASK_DTCFG_ID+1]<<8)|xtcp_rx_buf[TASK_DTCFG_ID]),xtcp_rx_buf[TASK_DTCFG_TOLPACK],xtcp_rx_buf[TASK_DTCFG_PACKNUM]);
    // 单次配一个任务
    xtcp_debug_printf("rec pack inc %d\n",xtcp_rx_buf[TASK_DTCFG_PACKNUM]);
    
    if((g_sys_val.task_recid==0xFFFF)&&(xtcp_rx_buf[TASK_DTCFG_PACKNUM]==0)){
        g_sys_val.task_recid = (xtcp_rx_buf[TASK_DTCFG_ID+1]<<8)|xtcp_rx_buf[TASK_DTCFG_ID];
        g_sys_val.task_con_id = g_sys_val.task_recid;
        timer_task_read(&g_sys_val.tmp_union.task_allinfo_tmp,g_sys_val.task_recid);
        g_sys_val.task_music_inc = 0;
        g_sys_val.task_packrec_inc = 0;
        g_sys_val.task_con_state = 0;
        g_sys_val.task_delete_s = 0;
        g_sys_val.task_creat_s = 0;
        g_sys_val.task_dtinfo_setmusic_f = 0;
        g_sys_val.task_dtinfo_setdiv_f = 0;
        g_sys_val.task_creat_s = 0;
        g_sys_val.task_config_s = 2;
       // xtcp_debug_printf("rec id:%d %d,%d\n",g_sys_val.task_recid,g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum,g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum);
    }
    // 接收包+1
    g_sys_val.task_packrec_inc ++;
    //处理分包
    //xtcp_debug_printf("rec config byte %d\n",xtcp_rx_buf[TASK_DTCFG_PACKTYPE]);
    if(g_sys_val.task_recid == ((xtcp_rx_buf[TASK_DTCFG_ID+1]<<8)|xtcp_rx_buf[TASK_DTCFG_ID]) ){
        uint16_t data_base;
        //基础信息配置
        if(xtcp_rx_buf[TASK_DTCFG_PACKTYPE]==2){
            task_config_recive();
        }
        else if(xtcp_rx_buf[TASK_DTCFG_PACKTYPE]==0){ //获取发送列表
            #if 0
            for(uint8_t i=0;i<80;i++){
                xtcp_debug_printf("%2x ",xtcp_rx_buf[i]);
                if(i!=0&&i%20==0)
                    xtcp_debug_printf("\n");
            }
            xtcp_debug_printf("\n");
            #endif
            g_sys_val.task_dtinfo_setdiv_f = 1;
            data_base = TASK_DTCFG_MAC_BASE;
            //获得设备总数
            g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum = xtcp_rx_buf[TASK_DTCFG_MACTOL];
            xtcp_debug_printf("div tol %d\n",xtcp_rx_buf[TASK_DTCFG_MACTOL]);
            for(uint8_t i=0;i<xtcp_rx_buf[TASK_DTCFG_MACTOL] ;i++){
                // 获得分区控制位
                g_sys_val.tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].zone_control = 
                                                            (xtcp_rx_buf[data_base+TASK_DTCFG_AREACFG+1]<<8)|xtcp_rx_buf[data_base+TASK_DTCFG_AREACFG];
                xtcp_debug_printf("task area %x\n",g_sys_val.tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].zone_control);
                // 获得MAC
                memcpy(g_sys_val.tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac,
                       &xtcp_rx_buf[data_base+TASK_DTCFG_MAC],6);
                //
                data_base += TASK_DTCFG_MAC_LEN;
            }
        }
        else if(xtcp_rx_buf[TASK_DTCFG_PACKTYPE]==1){ //获取音乐列表
            g_sys_val.task_dtinfo_setmusic_f = 1;
            data_base = TASK_DTCFG_MUSIC_BASE;
            for(uint8_t i=0; i<xtcp_rx_buf[TASK_DTCFG_MUSICTOL]; i++){
                // 获得音乐路径名
                memcpy(g_sys_val.tmp_union.task_allinfo_tmp.task_musiclist.music_info[g_sys_val.task_music_inc].music_path,
                       &xtcp_rx_buf[data_base+TASK_DTCFG_MUSICPATCH],PATCH_NAME_NUM);
                #if 0
                for(uint8_t i=0;i<PATCH_NAME_NUM;i++){
                    if(xtcp_rx_buf[data_base+TASK_DTCFG_MUSICNAME+i]==0){
                        xtcp_debug_printf("\n");  
                        break;
                    }
                    xtcp_debug_printf("%c",xtcp_rx_buf[data_base+TASK_DTCFG_MUSICPATCH+i]);        
                }
                #endif
                // 获得音乐名
                memcpy(g_sys_val.tmp_union.task_allinfo_tmp.task_musiclist.music_info[g_sys_val.task_music_inc].music_name,
                          &xtcp_rx_buf[data_base+TASK_DTCFG_MUSICNAME],MUSIC_NAME_NUM);
                //
                #if 0
                for(uint8_t i=0;i<MUSIC_NAME_NUM;i++){
                    if(g_sys_val.tmp_union.task_allinfo_tmp.task_musiclist.music_info[g_sys_val.task_music_inc].music_name[i]==0){
                        xtcp_debug_printf("\n");  
                        break;
                    }
                    xtcp_debug_printf("%c",g_sys_val.tmp_union.task_allinfo_tmp.task_musiclist.music_info[g_sys_val.task_music_inc].music_name[i]);        
                }
                #endif
                g_sys_val.task_music_inc++;  
                g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum = g_sys_val.task_music_inc;
                //xtcp_debug_printf("music tol %d\n",g_sys_val.task_music_inc);
                data_base += TASK_DTCFG_MUSICLEN;
            }
        }
        // 多包处理完成
        if(((xtcp_rx_buf[TASK_DTCFG_PACKNUM]+1)==xtcp_rx_buf[TASK_DTCFG_TOLPACK])&&(g_sys_val.task_packrec_inc == xtcp_rx_buf[TASK_DTCFG_TOLPACK])){
            g_sys_val.task_packrec_inc=0;
            //是否单条添加任务
            if((xtcp_rx_buf[TASK_DTCFG_TOLPACK]==1)&&(xtcp_rx_buf[TASK_DTCFG_PACKTYPE]==2)){
                goto dtinfo_config_ok;
            }
            //判断写入是否 0首歌 或者 0个设备
            if((((g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum==0)&&(g_sys_val.task_dtinfo_setmusic_f))||
                ((g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum==0)&&(g_sys_val.task_dtinfo_setdiv_f)))&&
                (g_sys_val.task_delete_s!=1)){
                //
                g_sys_val.task_con_state |= 0xC0;
            }
            //----------------------------------------------------------------------------------------------
            dtinfo_config_ok:
            if(g_sys_val.task_con_state) // 配置失败
                goto dtinfo_send_end;
            //----------------------------------------------------------------------------------------------
            // 配置成功
            if(g_sys_val.task_creat_s){
                //------------------------------------------------------------------------------------------------------
                //任务数量判断
                uint16_t tasknum_cnt=0;
                timetask_t *task_p = timetask_list.all_timetask_head;
                //
                while(task_p!=null){
                    if(task_p->solu_id==g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn)
                        tasknum_cnt++;
                    task_p = task_p->all_next_p;
                }
                if((tasknum_cnt>=MAX_TIMED_TASK_NUM)&&(g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn==0xFF)){
                    g_sys_val.task_con_state |= 1;
                    goto dtinfo_send_end;
                }
                if(tasknum_cnt>=MAX_SOUL_HAVETASK){
                    g_sys_val.task_con_state |= 1;
                    goto dtinfo_send_end;
                }
                //-------------------------------------------------------------------------------------------------
                uint16_t id;
                if(create_task_node()){ //任务添加成功   
                    id = timetask_list.all_timetask_end->id;
                    g_sys_val.task_con_id = id;
                    g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_id =id;
                    //                        
                    xtcp_debug_printf("task creat id:%d\n",id);
                }         
                else{
                    g_sys_val.task_con_state |= 1;
                    goto dtinfo_send_end;
                }
            }   
            xtcp_debug_printf("task save %d\n",g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.dura_time);
            timetask_list.timetask[g_sys_val.task_con_id ].time_info = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.time_info;
            timetask_list.timetask[g_sys_val.task_con_id ].task_en = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_state;
            timetask_list.timetask[g_sys_val.task_con_id ].solu_id = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn;
            timetask_list.timetask[g_sys_val.task_con_id ].repe_mode = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_repe_mode;
            timetask_list.timetask[g_sys_val.task_con_id ].week_repebit = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.week_repebit;
            for(uint8_t i=0;i<MAX_TASK_DATE_NUM;i++){
                timetask_list.timetask[g_sys_val.task_con_id ].date_info[i] = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.dateinfo[i];
            }
            //-------------------------------------------------------------
            // 判断时间 自动禁止
            timetask_t *task_p;
            uint8_t over_time_inc=0;
            unsigned next_tasktime,beg_time,end_time;
            uint8_t tasksolu_id;
            //
            task_p = timetask_list.all_timetask_head;;
            beg_time = (g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour*3600)+
                       (g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute*60)+
                        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.time_info.second;
            end_time = beg_time+g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.dura_time;
            //
            tasksolu_id = g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.solution_sn;
            //
            while(task_p!=null){
                timer_task_read(&tmp_union.task_allinfo_tmp,task_p->id);
                next_tasktime = (tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour*3600)+
                                (tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute*60)+
                                 tmp_union.task_allinfo_tmp.task_coninfo.time_info.second;
                // 时间不正确
                //xtcp_debug_printf("nt %d bt %d | ntt %d et %d | tid %d,ctid %d | sid %d tsid %d\n",next_tasktime,beg_time,next_tasktime,end_time,task_p->id,g_sys_val.task_con_id,tasksolu_id,task_p->solu_id);
                if((next_tasktime >= beg_time)&&(next_tasktime<=end_time)&&(task_p->id!=g_sys_val.task_con_id)&&(tasksolu_id==task_p->solu_id)){
                    over_time_inc++;
                    xtcp_debug_printf("task time inc\n");
                    if(over_time_inc>SOLU_MAX_PLAYCH){
                        g_sys_val.task_con_state |= 16;
                        g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_state = 0;
                        timetask_list.timetask[g_sys_val.task_con_id].task_en=0;
                        xtcp_debug_printf("task time error\n");
                        break;
                    }
                }
                task_p = task_p->all_next_p;
            }
            //-------------------------------------------------------------
            xtcp_debug_printf("rttask write id:%d buf id:%d\n",g_sys_val.task_con_id,g_sys_val.tmp_union.task_allinfo_tmp.task_coninfo.task_id);
            
            timer_task_write(&g_sys_val.tmp_union.task_allinfo_tmp,g_sys_val.task_con_id);
            //-----------------------------------------------------------------------------------------------------
            // 完成处理
            dtinfo_send_end:
            g_sys_val.task_recid=0xFFFF;
            user_sending_len = threebyte_ack_build(g_sys_val.task_con_id,g_sys_val.task_con_id>>8,g_sys_val.task_con_state,TASK_DIINFO_CONFIG_CMD);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
            xtcp_debug_printf("send id: %d\n state %x\n",g_sys_val.task_con_id,g_sys_val.task_con_state);
            create_todaytask_list(g_sys_val.time_info);
            //---------------------------------------------------------------------------------
            // 信息更新
            if(g_sys_val.task_con_state == 0){
                mes_send_taskinfo(&g_sys_val.tmp_union.task_allinfo_tmp);
            }
            //--------------------------------------------------------------------------------
            xtcp_debug_printf("task dtinfo config over\n");
        }    
    }
    //    
}
// 超时接收失败
void task_dtinfo_overtime_recive_close(){
    if(g_sys_val.task_recid!=0xFFFF){
        g_sys_val.task_rec_count++;
        if(g_sys_val.task_rec_count>8){
            g_sys_val.task_recid=0xFFFF;
            g_sys_val.task_rec_count = 0;
        }
    }
}

//====================================================================================================
// 定时任务批量编辑协议                  B30D
//====================================================================================================
#define DAY_TIME_SECOND (24*60*60)
void task_bat_config_recive(){
    uint16_t dat_base = TASK_BAT_TASKID;
    uint16_t id;
    uint8_t beg_hour,beg_minute,beg_second;
    unsigned rectime,tasktime;

    rectime = (xtcp_rx_buf[TASK_BAT_BEGTIME]*3600)+(xtcp_rx_buf[TASK_BAT_BEGTIME+1]*60)+xtcp_rx_buf[TASK_BAT_BEGTIME+2];
    rectime = rectime%DAY_TIME_SECOND;
    
    beg_hour = xtcp_rx_buf[TASK_BAT_BEGTIME] %24;
    beg_minute = xtcp_rx_buf[TASK_BAT_BEGTIME+1] %60;
    beg_second = xtcp_rx_buf[TASK_BAT_BEGTIME+2] %60;
    //
    uint8_t state=01;
    for(uint8_t i=0;i<xtcp_rx_buf[TASK_BAT_TASKTOL];i++){
        id = xtcp_rx_buf[dat_base]|(xtcp_rx_buf[dat_base+1]<<8);
        if(id>MAX_HOST_TASK){
            state = 0;
            break;
        }
        timer_task_read(&tmp_union.task_allinfo_tmp,id);
        tasktime = (tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour*3600)+
                   (tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute*60)+tmp_union.task_allinfo_tmp.task_coninfo.time_info.second;
        // 删除任务
        if(xtcp_rx_buf[TASK_BAT_CONFIG_S]==01){
            delete_task_node(id);
            timetask_list.timetask[id].id=0xFFFF;
            tmp_union.task_allinfo_tmp.task_coninfo.task_id = 0xFFFF;
            timer_task_write(&tmp_union.task_allinfo_tmp,id);
            dat_base+=2;
            continue;    
        }
        // 提前时间
        if(xtcp_rx_buf[TASK_BAT_CONFIG_S]==02){
            if(rectime>tasktime){
                tasktime = DAY_TIME_SECOND - (rectime-tasktime);
            }
            else{
                tasktime -= rectime;
            }
            tmp_union.task_allinfo_tmp.task_coninfo.time_info.second = tasktime % 60;
            tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute = (tasktime % 3600)/60;
            tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour = tasktime/3600;
            timetask_list.timetask[id].time_info = tmp_union.task_allinfo_tmp.task_coninfo.time_info;
        }
        // 延后时间
        if(xtcp_rx_buf[TASK_BAT_CONFIG_S]==03){
            tasktime += rectime;
            tasktime = tasktime%DAY_TIME_SECOND;
            tmp_union.task_allinfo_tmp.task_coninfo.time_info.second = tasktime % 60;
            tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute = (tasktime % 3600)/60;
            tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour = tasktime/3600;
            timetask_list.timetask[id].time_info = tmp_union.task_allinfo_tmp.task_coninfo.time_info;
        }
        // 持续时间
        if(xtcp_rx_buf[TASK_BAT_DURATIME_S]==01){
            tmp_union.task_allinfo_tmp.task_coninfo.dura_time = xtcp_rx_buf[TASK_BAT_DURATIME]*3600+xtcp_rx_buf[TASK_BAT_DURATIME+1]*60+xtcp_rx_buf[TASK_BAT_DURATIME+2];
        }
        timer_task_write(&tmp_union.task_allinfo_tmp,id);
        // 下一个id
        dat_base+=2;
    }
    //-------------------------------------------------------------
    
    // 判断时间 自动禁止
    timetask_t *task_p;
    dat_base = TASK_BAT_TASKID;
    unsigned next_tasktime,beg_time,end_time;
    uint8_t tasksolu_id;
    for(uint8_t i=0;i<xtcp_rx_buf[TASK_BAT_TASKTOL];i++){
        uint8_t over_time_inc=0;
        task_p = timetask_list.all_timetask_head;
        id = xtcp_rx_buf[dat_base]|(xtcp_rx_buf[dat_base+1]<<8);
        timer_task_read(&tmp_union.task_allinfo_tmp,id);
        //
        beg_time = (tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour*3600)+
                   (tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute*60)+
                   tmp_union.task_allinfo_tmp.task_coninfo.time_info.second;
        end_time = beg_time+tmp_union.task_allinfo_tmp.task_coninfo.dura_time;
        //
        tasksolu_id = tmp_union.task_allinfo_tmp.task_coninfo.solution_sn;
        //
        while(task_p!=null){
            timer_task_read(&tmp_union.task_allinfo_tmp,task_p->id);
            next_tasktime = (tmp_union.task_allinfo_tmp.task_coninfo.time_info.hour*3600)+
                            (tmp_union.task_allinfo_tmp.task_coninfo.time_info.minute*60)+
                             tmp_union.task_allinfo_tmp.task_coninfo.time_info.second;
            // 时间不正确
            if((next_tasktime > beg_time)&&(next_tasktime<end_time)&&(task_p->id!=id)&&(tasksolu_id==task_p->solu_id)){
                over_time_inc++;
                if(over_time_inc>SOLU_MAX_PLAYCH){
                    timer_task_read(&tmp_union.task_allinfo_tmp,id);
                    tmp_union.task_allinfo_tmp.task_coninfo.task_state = 0;
                    timetask_list.timetask[id].task_en=0;
                    timer_task_write(&tmp_union.task_allinfo_tmp,id);
                }
                break;
            }
            task_p = task_p->all_next_p;
        }
        // 下一个id
        dat_base+=2;
    }
    
    //-------------------------------------------------------------
    create_todaytask_list(g_sys_val.time_info);
    user_sending_len = onebyte_ack_build(state,TASK_BAT_CONFIG_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]); 
    mes_send_listinfo(TIMETASKERROR_INFO_REFRESH,0);
    taskview_page_messend();
}

//====================================================================================================
// 定时任务播放禁止/启动协议                  B30E
//====================================================================================================
void task_en_recive(){ 
    uint16_t id;
    uint8_t state=0;
    id = (xtcp_rx_buf[POL_DAT_BASE+1]<<8)|xtcp_rx_buf[POL_DAT_BASE];
    //
    if(id>MAX_HOST_TASK-1)
        goto task_en_end;
    // 禁止任务提前处理
    if(xtcp_rx_buf[POL_DAT_BASE+2]==0){
        for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
            if((timetask_now.ch_state[i]!=0xFF)&&(timetask_now.task_musicplay[i].task_id==id)){
                task_music_config_stop(i);
                // 重读数据
                //timer_task_read(&tmp_union.task_allinfo_tmp,id);
            }
        }
    }
    timer_task_read(&tmp_union.task_allinfo_tmp,id);
    tmp_union.task_allinfo_tmp.task_coninfo.task_state = xtcp_rx_buf[POL_DAT_BASE+2];
    xtcp_debug_printf("B30E task en %d\n",xtcp_rx_buf[POL_DAT_BASE+2]);
    timetask_list.timetask[id].task_en = xtcp_rx_buf[POL_DAT_BASE+2];
    timer_task_write(&tmp_union.task_allinfo_tmp,id);
    create_todaytask_list(g_sys_val.time_info);
    state = 1;
    task_en_end:
    user_sending_len = onebyte_ack_build(state,TASK_EN_CONFIG_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);    
    //---------------------------------------------------------------------------------
    // 任务信息更新
    if(state=1){
        xtcp_debug_printf("\n\ntask updata\n\n");
		g_sys_val.task_config_s = 2; //任务编辑
		g_sys_val.task_con_id = id;
        mes_send_taskinfo(&tmp_union.task_allinfo_tmp);
    }
    //--------------------------------------------------------------------------------
}

//====================================================================================================
// 定时任务播放测试协议                  B308 
//====================================================================================================
void task_playtext_recive(){    
    uint16_t id = (xtcp_rx_buf[TASK_PLAY_ID+1]<<8)|xtcp_rx_buf[TASK_PLAY_ID];
    if(xtcp_rx_buf[TASK_PLAY_CONTORL]){// 1 停止
        for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
            //xtcp_debug_printf("text chk %d %d\n",timetask_now.ch_state[i],timetask_now.task_musicplay[i].task_id);
            if((timetask_now.ch_state[i]!=0xFF)&&(timetask_now.task_musicplay[i].task_id==id)){
                xtcp_debug_printf("text stop %d\n",id);
                task_music_config_stop(i);
                // 任务信息更新
            	g_sys_val.task_config_s = 2; //任务编辑
            	g_sys_val.task_con_id = id;
                mes_send_taskinfo(&tmp_union.task_allinfo_tmp);
                goto play_text_sucess;
            }
        }
        goto play_text_fail;
    }
    xtcp_debug_printf("play id %d\n",id);
    // 读取任务
    timer_task_read(&tmp_union.task_allinfo_tmp,id);
    // 判断是否空任务
    if(tmp_union.task_allinfo_tmp.task_coninfo.task_id==0xFFFF){
        goto play_text_fail;
    }
    //--------------------------------------------------------------
    // 播放任务 
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if(g_sys_val.task_wait_state[i]==0){
            g_sys_val.music_task_id[i] = id;
            g_sys_val.task_wait_state[i] = 1;
            g_sys_val.play_ok = 1;
            user_disptask_refresh();
            break;
        }
    }
    //--------------------------------------------------------------
    //
    play_text_sucess:
    user_sending_len =  id_ack_build(id,1,TASK_PLAYTEXT_CMD);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);

    //---------------------------------------------------------------------------------
	
    return;
    play_text_fail:
    user_sending_len =  id_ack_build(id,0,TASK_PLAYTEXT_CMD);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);    
    //
}

//====================================================================================================
// 今日任务查询                     B309
//====================================================================================================
void today_week_check_recive(){
    user_sending_len = todaytask_ack_build();
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    xtcp_debug_printf("today week check\n");
}

//====================================================================================================
// 今日任务配置                     B30B
//====================================================================================================
void today_week_config_recive(){
    g_sys_val.today_date.week = xtcp_rx_buf[POL_DAT_BASE];
    g_sys_val.today_date.year = xtcp_rx_buf[POL_DAT_BASE+1];  
    g_sys_val.today_date.month = xtcp_rx_buf[POL_DAT_BASE+2];
    g_sys_val.today_date.date = xtcp_rx_buf[POL_DAT_BASE+3];
    //    
    user_sending_len = onebyte_ack_build(1,TASK_CONFIG_WEEK_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    create_todaytask_list(g_sys_val.time_info);
    //
    mes_send_listinfo(TODAYTASK_INFO_REFRESH,0);
    //
    xtcp_debug_printf("today week config\n");
}

//==================================================================================================
// 批量编辑终端配置                     B30F
//==================================================================================================
void bat_task_divset_recive(){
    uint16_t task_id,task_num;
    uint16_t dat_len=POL_DAT_BASE+2;
    uint16_t div_base,div_tol,adr_base;
    //
    #if 0
    xtcp_debug_printf("\n\nbat set \n");
    for(uint8_t i=0;i<200;i++){
        xtcp_debug_printf("%x ",xtcp_rx_buf[POL_DAT_BASE+i]);
    }
    xtcp_debug_printf("\n\n");
    #endif
    task_num = xtcp_rx_buf[POL_DAT_BASE]|(xtcp_rx_buf[POL_DAT_BASE+1]<<8);
    
    div_base = task_num*2+dat_len;
    div_tol = xtcp_rx_buf[div_base];
    div_base++;
    xtcp_debug_printf("div tol %d task tol %d\n",div_tol,task_num);
    for(uint16_t i=0;i<task_num;i++){
        adr_base = div_base;
        task_id = xtcp_rx_buf[dat_len]|(xtcp_rx_buf[dat_len+1]<<8);
        timer_task_read(&tmp_union.task_allinfo_tmp,task_id);
        // 编辑播放终端列表
        for(uint16_t div_num=0; div_num<div_tol; div_num++){
            tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum = div_tol;
            tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[div_num].zone_control = xtcp_rx_buf[adr_base]|(xtcp_rx_buf[adr_base+1]<<8);
            memcpy(tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[div_num].mac,&xtcp_rx_buf[adr_base+2],6);
            adr_base+=8;
        }    
        timer_task_write(&tmp_union.task_allinfo_tmp,task_id);
        dat_len +=2;
    }
    user_sending_len = onebyte_ack_build(1,TASK_BAT_DIVSET_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);   
}


//====================================================================================================
// 即时任务列表初始化            
//====================================================================================================
void fl_rttask_dat_init(){
    rttask_lsit.all_head_p = null;
    rttask_lsit.run_head_p = null;
    rttask_lsit.all_end_p = null;
    rttask_lsit.rttask_tol = 0;
    tmp_union.rttask_dtinfo.rttask_id=0xFFFF;
    for(uint8_t i=0;i<MAX_RT_TASK_NUM;i++){
        rttask_lsit.rttask_info[i].rttask_id=0xFFFF;
        rttask_lsit.rttask_info[i].all_next_p=null;
        rttask_lsit.rttask_info[i].run_next_p=null;
        rt_task_write(&tmp_union.rttask_dtinfo,i);
    }
}

//====================================================================================================
// 即时任务列表 读取          
//====================================================================================================
void rt_task_list_read(){
    rttask_lsit.all_head_p = null;
    rttask_lsit.all_end_p = null;
    rttask_lsit.run_head_p = null;
    rttask_lsit.run_end_p = null;
    for(uint8_t i=1;i<MAX_RT_TASK_NUM;i++){
        rttask_lsit.rttask_info[i].all_next_p = null;
        rttask_lsit.rttask_info[i].run_next_p = null;
        rttask_lsit.rttask_info[i].rttask_id = 0xFFFF;
        rt_task_read(&tmp_union.rttask_dtinfo,i);
        // 判断错误任务 复位任务
        if((tmp_union.rttask_dtinfo.rttask_id!=0xFFFF)&&(tmp_union.rttask_dtinfo.rttask_id!=i)){
            tmp_union.rttask_dtinfo.rttask_id=0xFFFF;   
            rt_task_write(&tmp_union.rttask_dtinfo,i);
        }
        if(tmp_union.rttask_dtinfo.rttask_id!=0xFFFF){
            create_rttask_node_forid(i);
            xtcp_debug_printf("rt list %d\n",i);
        }
    }
}

//====================================================================================================
// 即时任务列表查询                     B400
//====================================================================================================
void rttask_list_check_recive(){
    uint8_t list_num = list_sending_init(RTTASK_CHECK_CMD,RTTASK_LIST_SENDING,&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE]);
    if(list_num==LIST_SEND_INIT)
        return;
    //
	if(g_sys_val.list_sending_f==0){
		g_sys_val.list_sending_f = 1;
	    user_sending_len = rttask_list_chk_build(list_num);
	    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
	}
    //xtcp_debug_printf("rttask list check\n");
}

void rttask_list_sending_decode(uint8_t list_num){
    user_sending_len = rttask_list_chk_build(list_num);
    user_xtcp_send(t_list_connsend[list_num].conn,t_list_connsend[list_num].could_s);
    //xtcp_debug_printf("rttask list sending\n");
}


//====================================================================================================
// 即时任务详细信息查询                   B402
//====================================================================================================
void rttask_dtinfo_check_recive(){
    uint16_t id;
    id = (xtcp_rx_buf[RTTASK_DTCK_ID+1]<<8)|xtcp_rx_buf[RTTASK_DTCK_ID]; 
    user_sending_len = rttask_dtinfo_chk_build(id);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

void rttask_runningtask_stop_start(uint16_t id,uint8_t state,uint8_t del_en){
    div_node_t *div_tmp_p;
    conn_list_t *div_conn_p;
    // 查找该任务id是否已经在运行
    rttask_info_t *runtmp_p = rttask_lsit.run_head_p;
    while(runtmp_p!=null){
        //比较任务id
        if(runtmp_p->rttask_id == id){ 
            if(del_en){
                // 任务运行中 删除
                delete_rttask_run_node(id);
                //------------------------------------------------------------------------------------------------------------
                // 关闭设备
                rt_task_read(&tmp_union.rttask_dtinfo,runtmp_p->rttask_id);
            }
            // 找到源设备
            div_tmp_p = get_div_info_p(tmp_union.rttask_dtinfo.src_mas);
            if(div_tmp_p == null){
                break;
            }
            //------------------------------------------------------------------------------------------------------------
            // 找到源连接
            div_conn_p = get_conn_for_ip(div_tmp_p->div_info.ip);
            if(div_conn_p == null){
                break;
            }
            runtmp_p->dura_time = tmp_union.rttask_dtinfo.dura_time;
            runtmp_p->over_time = 0;
            
            user_sending_len = rttask_connect_build(state, // 0 启动任务 // 1 关闭任务
                                                    tmp_union.rttask_dtinfo.account_id,
                                                    tmp_union.rttask_dtinfo.rttask_id,
                                                    RTTASK_BUILD_CMD,00);
            user_xtcp_send(div_conn_p->conn,0);
            //有运行中任务 
            xtcp_debug_printf("old rttask rebuild\n");
            break;
        }
        runtmp_p = runtmp_p->run_next_p;
    }


}

//====================================================================================================
// 即时任务配置                    B403
//====================================================================================================
void rttask_config_recive(){
    //
    uint16_t id;
    uint8_t state=1;

    id = (xtcp_rx_buf[RTTASK_CFG_TASKID+1]<<8)|xtcp_rx_buf[RTTASK_CFG_TASKID];
    // 添加任务
    if(xtcp_rx_buf[RTTASK_CFG_CONTORL]==0){
        if(!create_rttask_node()){
            state=0;
        }
        else{
            id = rttask_lsit.all_end_p->rttask_id;
            // 获得新建任务ID
            tmp_union.rttask_dtinfo.rttask_id = id;
            // 获得创建账户ID
            tmp_union.rttask_dtinfo.account_id = xtcp_rx_buf[RTTASK_CFG_ACID];
        }
        //
    }
    // 删除任务
    else if(xtcp_rx_buf[RTTASK_CFG_CONTORL]==1){
        //删除运行中任务，停止任务
        rttask_runningtask_stop_start(id,1,1);
        // 删除整个任务节点
        if(delete_rttask_node(id)){
            tmp_union.rttask_dtinfo.rttask_id = 0xFFFF;
            state=1;
        }
    }
    // 编辑任务
    else{
        rt_task_read(&tmp_union.rttask_dtinfo,id);
        //编辑中改变音源停止任务
        if(!charncmp(tmp_union.rttask_dtinfo.src_mas,&xtcp_rx_buf[RTTASK_CFG_SRCMAC],6)){
            rttask_runningtask_stop_start(id,1,0);
        }
    }
    //-----------------------------------------------------------------------------------------
    #if 0
    rttask_info_t *tmp_p;
    tmp_p = rttask_lsit.all_head_p;
    while(tmp_p!=null){
        xtcp_debug_printf("rt task id %d\n",tmp_p->rttask_id);
        tmp_p = tmp_p->all_next_p;
    }
    #endif
    //-----------------------------------------------------------------------------------------
    // 配置名称
    memcpy(tmp_union.rttask_dtinfo.name,&xtcp_rx_buf[RTTASK_CFG_TASKNAME],DIV_NAME_NUM);
    // 配置播放源机器
    memcpy(tmp_union.rttask_dtinfo.src_mas,&xtcp_rx_buf[RTTASK_CFG_SRCMAC],6);
    // 配置音量
    tmp_union.rttask_dtinfo.task_vol = xtcp_rx_buf[RTTASK_CFG_TASKVOL];
    // 任务持续时间
    if(xtcp_rx_buf[RTTASK_CFG_DURATIME]==0xFF && xtcp_rx_buf[RTTASK_CFG_DURATIME+1]==0xFF && xtcp_rx_buf[RTTASK_CFG_DURATIME+2]==0xFF){
        tmp_union.rttask_dtinfo.dura_time=0xFFFFFFFF;
    }
    else{
        tmp_union.rttask_dtinfo.dura_time = xtcp_rx_buf[RTTASK_CFG_DURATIME]*3600+xtcp_rx_buf[RTTASK_CFG_DURATIME+1]*60+xtcp_rx_buf[RTTASK_CFG_DURATIME+2];
    }
    // 遥控按键配置
    tmp_union.rttask_dtinfo.task_key = xtcp_rx_buf[RTTASK_CFG_KETINFO];
    // 设置优先级
    tmp_union.rttask_dtinfo.prio = xtcp_rx_buf[RTTASK_CFG_TASKPRIO];
    // 配置发送终端
    if(xtcp_rx_buf[RTTASK_CFG_DIVTOL] < MAX_DIV_LIST){
        tmp_union.rttask_dtinfo.div_tol = xtcp_rx_buf[RTTASK_CFG_DIVTOL];
        uint16_t data_base = RTTASK_CFG_DIV_BASE;
        for(uint8_t i=0; i<xtcp_rx_buf[RTTASK_CFG_DIVTOL]; i++){
            tmp_union.rttask_dtinfo.des_info[i].zone_control = (xtcp_rx_buf[data_base+RTTASK_CFG_AREACONTORL+1]<<8)|xtcp_rx_buf[data_base+RTTASK_CFG_AREACONTORL];
            //xtcp_debug_printf("rt t area %x\n",tmp_union.rttask_dtinfo.des_info[i].zone_control);
            memcpy(tmp_union.rttask_dtinfo.des_info[i].mac,&xtcp_rx_buf[data_base+RTTASK_CFG_MAC],6);
            data_base += RTTASK_CFG_LEN;
        }
    }    
    // 保存信息
    rt_task_write(&tmp_union.rttask_dtinfo,id);
    //----------------------------------------------------------------------------------------------------------------------------
    // 即时任务运行中，重发
    rttask_runningtask_stop_start(id,0,0);
    //----------------------------------------------------------------------------------------------------------------------------
    user_sending_len = rttask_config_ack_build(id,state);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    // 推送消息
    if(state){
        mes_send_rttaskinfo(id,xtcp_rx_buf[RTTASK_CFG_CONTORL]);
        // 任务页面更新通知
        taskview_page_messend();
    }
}
//====================================================================================================
// 关闭运行中的即时任务 
//====================================================================================================
void close_running_rttask(uint8_t *mac){
    // 查找同一音源是否有旧任务并关闭
    rttask_info_t *runtmp_p = rttask_lsit.run_head_p;
    while(runtmp_p!=null){
        rt_task_read(&tmp_union.rttask_dtinfo,runtmp_p->rttask_id);       
        //比较MAC
        if(charncmp(tmp_union.rttask_dtinfo.src_mas,mac,6)){
            delete_rttask_run_node(runtmp_p->rttask_id);
            //有运行中任务 
            xtcp_debug_printf("del old rttask\n");
            break;
        }
        runtmp_p = runtmp_p->run_next_p;
    }
}

//====================================================================================================
// 即时任务开关控制                    B404
//====================================================================================================
void rttask_contorl_recive(){
    xtcp_debug_printf("rttask contorl\n");
    uint16_t id;
    uint16_t user_id;
    div_node_t *div_tmp_p;
    conn_list_t *div_conn_p;
    // 获取任务ID
    id = (xtcp_rx_buf[RTTASK_PLAY_TASKID+1]<<8)|xtcp_rx_buf[RTTASK_PLAY_TASKID];
    user_id = xtcp_rx_buf[RTTASK_PLAY_USERID]|(xtcp_rx_buf[RTTASK_PLAY_USERID+1]<<8);
    //------------------------------------------------------------------------------------------
    // 查找任务是否已被建立
    //if((rttask_run_chk(id))&&(xtcp_rx_buf[RTTASK_PLAY_CONTORL]))
    //    goto rttaask_creat_fail;
    //-------------------------------------------------------------------------------------------
    // 即时任务读取
    rt_task_read(&tmp_union.rttask_dtinfo,id);
    //---------------------------------------------------------------------------------------------------------------
    // 从设备列表找源设备
    //xtcp_debug_printf("rt src mac %x,%x,%x,%x,%x,%x\n",tmp_union.rttask_dtinfo.src_mas[0],tmp_union.rttask_dtinfo.src_mas[1],tmp_union.rttask_dtinfo.src_mas[2],
    //                                             tmp_union.rttask_dtinfo.src_mas[3],tmp_union.rttask_dtinfo.src_mas[4],tmp_union.rttask_dtinfo.src_mas[5]);
    div_tmp_p = get_div_info_p(tmp_union.rttask_dtinfo.src_mas);
    if(div_tmp_p == null){
        goto rttaask_creat_fail;
    }
    // 判断设备是否离线
    if(div_tmp_p->div_info.div_state==0){
        goto rttaask_creat_fail;
    }
    //---------------------------------------------------------------------------------------------------------------
    // 找播放源设备连接节点
    xtcp_connection_t new_conn;
    div_conn_p = get_conn_for_ip(div_tmp_p->div_info.ip);
    //
    if(div_conn_p == null){
        if(user_xtcp_connect_udp(8805,div_tmp_p->div_info.ip,&new_conn))
            goto rttaask_creat_fail;
         create_conn_node(&new_conn);    //新建一个conn节点
    }
    else{
        new_conn = div_conn_p->conn;
    }
    xtcp_debug_printf("des_ip %d,%d,%d,%d\n",div_tmp_p->div_info.ip[0],div_tmp_p->div_info.ip[1],div_tmp_p->div_info.ip[2],div_tmp_p->div_info.ip[3]);
    //---------------------------------------------------------------------------------------------------------------
    for(uint8_t i=0;i<MAX_RTTASK_CONTORL_NUM;i++){
        if(rttask_build_state[i].des_conn_id==0){
            rttask_build_state[i].src_conn_id = conn.id;
            rttask_build_state[i].des_conn_id = new_conn.id;
            rttask_build_state[i].rttask_id = id;
            rttask_build_state[i].user_id = user_id;
            rttask_build_state[i].contorl = !xtcp_rx_buf[RTTASK_PLAY_CONTORL];
            rttask_build_state[i].over_time = 0;
            rttask_build_state[i].dura_time = tmp_union.rttask_dtinfo.dura_time;
            user_sending_len = rttask_connect_build(rttask_build_state[i].contorl,
                                                    rttask_build_state[i].user_id,
                                                    rttask_build_state[i].rttask_id,
                                                    RTTASK_BUILD_CMD,00);
            
            // 局域网发送
            user_xtcp_send(new_conn,0);
            return;
        }
    }
    //--------------------------------------------------------------------------------------
    // 任务建立失败
    rttaask_creat_fail:
    xtcp_debug_printf("rt creat fail\n");
    user_sending_len = rttask_creat_build(0x00,0x03,id);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}
//====================================================================================================
// 即时任务 设备建立                    B407
//====================================================================================================
void rttask_build_recive(){
    xtcp_debug_printf("rttask build \n");
    uint8_t state;
    conn_list_t *conn_tmp_p; 
    for(uint8_t i=0;i<MAX_RTTASK_CONTORL_NUM;i++){
        if(conn.id == rttask_build_state[i].des_conn_id){
            rttask_build_state[i].des_conn_id = 0;
            //---------------------------------------------------------------------------------------------------------------------
            //创建运行中任务
            xtcp_debug_printf("contorl %d\n",rttask_build_state[i].contorl);
            if(rttask_build_state[i].contorl==0){
                xtcp_debug_printf("creat run rt %d\n",rttask_build_state[i].rttask_id);
                //---------------------------------------------------------------------------------------------------------
                // 关闭运行中的即时任务 
                close_running_rttask(&xtcp_rx_buf[POL_MAC_BASE]);
                //---------------------------------------------------------------------------------------------------------
                // 创建新任务
                if(!(rttask_run_chk(rttask_build_state[i].rttask_id))){
                    if(create_rttask_run_node(rttask_build_state[i].rttask_id)==0){
                        state = 0; 
                        goto rttask_contorl_end;
                    }
                    state = 1;
                    rttask_lsit.run_end_p->user_id = rttask_build_state[i].user_id;
                    rttask_lsit.run_end_p->dura_time = rttask_build_state[i].dura_time;
                    rttask_lsit.run_end_p->over_time = 0;
                    rttask_lsit.run_end_p->run_state = 01;
                }
                //---------------------------------------------------------------------------------------------------------
            }
            //---------------------------------------------------------------------------------------------------------------------
            //关闭运行中任务
            else if(rttask_build_state[i].contorl==1){   
                state = 1;
                delete_rttask_run_node(rttask_build_state[i].rttask_id);
                xtcp_debug_printf("close run rt %d\n",rttask_build_state[i].rttask_id);
            }
            //---------------------------------------------------------------------------------------------------------------------
            rttask_contorl_end:
            xtcp_debug_printf("rt build over %d\n",state);
            user_sending_len = rttask_creat_build(rttask_build_state[i].user_id,state,rttask_build_state[i].rttask_id);
            //--------------------------------------------------------------------------------------------------------------------
            // 找控制源CONN连接
            if(rttask_build_state[i].src_conn_id == g_sys_val.could_conn.id){
                user_xtcp_send(g_sys_val.could_conn,1);
                return;
            }
            conn_tmp_p = get_conn_info_p(rttask_build_state[i].src_conn_id);
            if(conn_tmp_p!=null){
                user_xtcp_send(conn_tmp_p->conn,0);
            }
            //
            mes_send_rttaskinfo(rttask_build_state[i].rttask_id,02);
            break;
        }
    }
}
//==============================================================================
// 即时任务开关状态复位
void rttask_build_overtime10hz(){
    for(uint8_t i=0;i<MAX_RTTASK_CONTORL_NUM;i++){
        if(rttask_build_state[i].des_conn_id!=0){
            rttask_build_state[i].over_time++;
            if((rttask_build_state[i].over_time%4)==0){
                conn_list_t *conn_tmp_p; 
                conn_tmp_p = get_conn_info_p(rttask_build_state[i].des_conn_id);
                if(conn_tmp_p!=null){
                    user_sending_len = rttask_connect_build(rttask_build_state[i].contorl,
                                                            rttask_build_state[i].user_id,
                                                            rttask_build_state[i].rttask_id,
                                                            RTTASK_BUILD_CMD,00);
                    user_xtcp_send(conn_tmp_p->conn,xtcp_rx_buf[POL_COULD_S_BASE]);
                }
            }
            if(rttask_build_state[i].over_time>15){
                rttask_build_state[i].des_conn_id=0;
                xtcp_debug_printf("rttask build timeout\n");
            }
        }
    }
}

//====================================================================================================
// 即时任务 任务重发建立                    B40A
//====================================================================================================
void task_rttask_rebuild(){
    //查找运行中任务
    rttask_info_t *tmp_p = rttask_lsit.run_head_p;
    while(tmp_p!=null){
        //比较MAC
        rt_task_read(&tmp_union.rttask_dtinfo,tmp_p->rttask_id);
        if(charncmp(tmp_union.rttask_dtinfo.src_mas,&xtcp_rx_buf[POL_DAT_BASE],6)){
            //有运行中任务
            user_sending_len = rttask_connect_build(00,
                                                    tmp_p->user_id,
                                                    tmp_p->rttask_id,RTTASK_REBUILD_CMD,00);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
            return;
        }
        tmp_p = tmp_p->run_next_p;
    }
    // 没有运行中任务
    memset(&tmp_union.rttask_dtinfo,sizeof(rttask_dtinfo_t),0x00);
    user_sending_len = rttask_connect_build(01,
                                            00,
                                            00,RTTASK_REBUILD_CMD,01);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//====================================================================================================
// 即时任务  播放列表更新            BF0D
//====================================================================================================
void rttask_playlist_updata_init(uint8_t ip[],div_node_t *div_info_p){
    g_sys_val.rttask_updat_p = rttask_lsit.run_head_p;
    g_sys_val.rttask_updat_f = 1;
    g_sys_val.rttask_div_p = div_info_p;
    memcpy(g_sys_val.rttask_up_ip,ip,4);
    xtcp_debug_printf("div ip change %d %d %d %d\n",div_info_p->div_info.ip[0],div_info_p->div_info.ip[1],div_info_p->div_info.ip[2],div_info_p->div_info.ip[3]);
}

void rttask_playlist_updata(){
    uint8_t needsend;
    if(g_sys_val.rttask_updat_f==0)
        return;
    while(g_sys_val.rttask_updat_p!=null){
        user_sending_len = rttask_listupdat_build(&needsend,g_sys_val.rttask_updat_p->rttask_id,g_sys_val.rttask_div_p);
        if(needsend){
            //---------------------------------------------------------------------------------------------------------------
            // 找播放源设备连接节点
            div_node_t *div_tmp_p;
            conn_list_t *div_conn_p;
            div_tmp_p = get_div_info_p(tmp_union.rttask_dtinfo.src_mas);
            div_conn_p = get_conn_for_ip(div_tmp_p->div_info.ip);
            //
            if(div_conn_p != null)
                user_xtcp_send(div_conn_p->conn,0);
            xtcp_debug_printf("send rttask list updat\n");
            g_sys_val.rttask_updat_p = g_sys_val.rttask_updat_p->run_next_p;
            return;
        }
        g_sys_val.rttask_updat_p = g_sys_val.rttask_updat_p->run_next_p;
    }
    g_sys_val.rttask_updat_f = 0;
    xtcp_debug_printf("rttask list updat end\n");
}


//====================================================================================================
void timer_rttask_run_process(){
    rttask_info_t *tmp_p = rttask_lsit.run_head_p;
    div_node_t *div_tmp_p;
    conn_list_t *div_conn_p;

    while(tmp_p!=null){
        //xtcp_debug_printf("tim %d\n",tmp_p->dura_time);
        if(tmp_p->dura_time!=0xFFFFFFFF){
            // 任务异常 计时暂停
            if(tmp_p->run_state!=2)
                tmp_p->over_time++;
            //xtcp_debug_printf("task time id%d t%d,\n",tmp_p->rttask_id,tmp_p->over_time);
            if(tmp_p->over_time>=tmp_p->dura_time){ 
                rt_task_read(&tmp_union.rttask_dtinfo,tmp_p->rttask_id);
                //------------------------------------------------------------------------------------------------------------
                // 找到源设备
                div_tmp_p = get_div_info_p(tmp_union.rttask_dtinfo.src_mas);
                if(div_tmp_p == null){
                    goto close_rttask;
                }
                xtcp_debug_printf("close rt mac %x,%x,%x,%x,%x,%x\n",tmp_union.rttask_dtinfo.src_mas[0],tmp_union.rttask_dtinfo.src_mas[1],tmp_union.rttask_dtinfo.src_mas[2],
                                                                tmp_union.rttask_dtinfo.src_mas[3],tmp_union.rttask_dtinfo.src_mas[4],tmp_union.rttask_dtinfo.src_mas[5]);
                //------------------------------------------------------------------------------------------------------------
                // 找到源连接
                div_conn_p = get_conn_for_ip(div_tmp_p->div_info.ip);
                if(div_conn_p == null){
                    goto close_rttask;
                }
                user_sending_len = rttask_connect_build(1,
                                                        tmp_union.rttask_dtinfo.account_id,
                                                        tmp_union.rttask_dtinfo.rttask_id,
                                                        RTTASK_BUILD_CMD,00);
                user_xtcp_send(div_conn_p->conn,0);
                close_rttask:
                delete_rttask_run_node(tmp_p->rttask_id);
            }
        }
        tmp_p = tmp_p->run_next_p;
    }//while
}

//====================================================================================================
// B312 任务面板数量显示 B312
//====================================================================================================
void task_pageshow_recive(){
    user_sending_len = taskview_page_build(TASK_PAGESHOW_B312_CMD);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//====================================================================================================
// B310 带任务数量的方案列表查询 B310
//====================================================================================================
void solulist_chk_forapp_recive(){
    user_sending_len = solution_list_ack_build(TASK_SOLUFORAPP_B310_CMD,1);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//====================================================================================================
// B311 查看指定数量 B311
//====================================================================================================
void tasklist_forsolu_chk_recive(){
    uint8_t list_num = list_sending_init(TASK_TASK_FORSOLU_B311_CMD,TASK_LIST_SENDING,&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE]);
    if(list_num==LIST_SEND_INIT)
        return;
    //
    t_list_connsend[list_num].list_info.tasklist.solu_en=1;
    t_list_connsend[list_num].list_info.tasklist.solu_id=xtcp_rx_buf[POL_DAT_BASE];
    //
    uint16_t task_tol=0;
    timetask_t *task_p = timetask_list.all_timetask_head;
    xtcp_debug_printf("solu id chl%d\n",xtcp_rx_buf[POL_DAT_BASE]);
    while(task_p!=null){
        if(task_p->solu_id==xtcp_rx_buf[POL_DAT_BASE]){
            task_tol++;
        }
        task_p = task_p->all_next_p;
    }
    xtcp_debug_printf("task num %d\n",task_tol);
    t_list_connsend[list_num].list_info.tasklist.task_tol = task_tol/MAX_TASK_ONCESEND;
    if(task_tol%MAX_TASK_ONCESEND || t_list_connsend[list_num].list_info.tasklist.task_tol==0)
        t_list_connsend[list_num].list_info.tasklist.task_tol++;
    //
    user_sending_len = task_list_ack_build(t_list_connsend[list_num].list_info.tasklist.cmd,1,t_list_connsend[list_num].list_info.tasklist.solu_id,list_num);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

