#include "timer_process.h"
#include "list_instance.h"
#include "list_contorl.h"
#include "user_xccode.h"
#include "user_unti.h"
#include "divlist_decode.h"
#include "bsp_ds1302.h"
#include "conn_process.h"
#include "account.h"
#include "flash_adrbase.h"
#include "fl_buff_decode.h"
#include "task_decode.h"
#include "user_lcd.h"
#include "user_messend.h"
#include "user_file_contorl.h"
#include "could_serve.h"

#include "debug_print.h"

//------------------------------------------------------------------------------
// 日期计时线程
//------------------------------------------------------------------------------
void timer_process(){
	uint8_t tmp;
    static uint8_t get_f=3;
    g_sys_val.time_info.second++;
    if(g_sys_val.time_info.second>(60-1)){
        g_sys_val.time_info.second=0;
        g_sys_val.time_info.minute++;	
        if(g_sys_val.time_info.minute>(60-1))
        {
            g_sys_val.time_info.minute=0;
            g_sys_val.time_info.hour++;
            cld_timesysnc_request();
            //每天23点同步时间 更新注册
            if(g_sys_val.time_info.hour==23){
                get_f = 0; 
            }
            if(g_sys_val.time_info.hour>(24-1)){
                get_f=0;
                ds1302_get_date();
                g_sys_val.time_info.hour=0;
                //
                // 判断方案日期
                for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
                    if((solution_list.solu_info[i].en==1)&&(solution_list.solu_info[i].state!=0xFF))
                        solution_data_chk(i);
                        g_sys_val.need_flash |= NEED_FL_SOLUTION;
                }
                //
                g_sys_val.today_date = g_sys_val.date_info;
                //
                create_todaytask_list(g_sys_val.time_info);
                // 注册日期判断
                if(host_info.regiser_days!=0){
                    host_info.regiser_days--;
                    // 机器到期
                    if((host_info.regiser_days==0)&&(host_info.regiser_state==1)){
                        host_info.regiser_state = 0;
                    }
                    hostinfo_fl_write();
                }
                g_sys_val.today_date = g_sys_val.date_info;
            }
        }    
    }
    if(get_f<3){
        get_f++;
        ds1302_get_date();
        if(get_f==3){
            mes_send_listinfo(TODAYTASK_INFO_REFRESH,1);
            user_disp_data();
            // 更新注册信息
            register_could_chk();
        }
    }
}

//===========================================================================

//---------------------------------------------------------------------------
//10hz 检测线程
//---------------------------------------------------------------------------
void timee10hz_process(){
    mes_send_overtime();
    task_10hz_mutich_play();    // 任务连续播放处理
    timer_flash_dat_process();  // flash 延时烧录处理线程
    task_dtinfo_overtime_recive_close(); //关闭详细信息 连接
    disp_task_delay();          // 任务刷新显示
    bat_filecontorl_resend_tim(); //音乐回复重发
    rttask_build_overtime10hz(); //即时任务建立状态超时
    backup_sendmes_10hz();
    divfound_over_timeinc();
    rttask_playlist_updata();
}

//--------------------------------------------------------------------------
// 1hz 检测线程
//--------------------------------------------------------------------------
void second_process(){
    timer_process();            //系统时钟
    if(g_sys_val.eth_link_state){
        conn_overtime_close();
        div_heart_overtime_close();
        account_login_overtime();
        timer_rttask_run_process();
        #if COULD_TCP_EN
        could_heart_send_timer();
        #endif
    }
    mic_time1hz_close();
    timer_taskmusic_check();    //定时任务几时
    task_check_and_play();      //定时任务定时播放
    user_disp_time();           // lcd 时钟显示
    timer_task_disp();          // 显示切换
	//debug_printf("y:%d-m:%d-d:%d-w:%d t %d:%d:%d\n",g_sys_val.date_info.year,g_sys_val.date_info.month,g_sys_val.date_info.date,g_sys_val.date_info.week,
	//										g_sys_val.time_info.hour,g_sys_val.time_info.minute,g_sys_val.time_info.second);
    //ds1302_get_date();
}

