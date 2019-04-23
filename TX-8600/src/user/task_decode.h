#ifndef __TASK_DECODE_H_
#define __TASK_DECODE_H_

#include <stdint.h>
#include "list_instance.h"

// 定时任务列表初始化
void task_fl_init();

// 获取定时任务列表基本信息
void timer_tasklist_read();
// 所有任务列表生成
void create_alltask_list();
// 今日任务列表生成
void create_todaytask_list(time_info_t time_info);

// 方案查询
void solution_check_recive();
// 方案日期判断
void solution_data_chk(uint8_t id);

// 任务列表查询
void task_check_recive();
// 任务列表连发
void tasklist_sending_decode(uint8_t list_num);

// 任务详细信息查询
void task_dtinfo_check_recive();

// 任务播放
void task_music_config_play(uint8_t ch,uint16_t id);

// 方案配置
void solution_config_recive();

// 任务配置
void task_config_recive();

// 任务详细信息配置
void task_dtinfo_config_recive();

// 任务批量编辑
void task_bat_config_recive();

// 批量编辑终端配置                     B30F
void bat_task_divset_recive();

// 任务启动禁止编辑
void task_en_recive();

// 任务信息连发
void task_dtinfo_decode(uint8_t list_num);
// 超时接收失败
void task_dtinfo_overtime_recive_close();

// 定时任务播放测试
void task_playtext_recive();

// 今日任务查询
void today_week_check_recive();

// 今日任务配置
void today_week_config_recive();

// 即时任务查询
void rttask_list_check_recive();

// 即时任务详细信息查询
void rttask_dtinfo_check_recive();

// 即时任务配置
void rttask_config_recive();

// 即时任务APP控制
void rttask_contorl_recive();
//
void rttask_contorl_connect_decode(uint8_t con_num);
//-------------------------------------------
// 即时任务 设备建立 
void rttask_build_recive();

// 即时任务 重发
void task_rttask_rebuild();

// 即时任务列表 flash初始化
void fl_rttask_dat_init();

// 即时任务列表 读取
void rt_task_list_read();

// 即时任务列表连续传送
void rttask_list_sending_decode(uint8_t list_num);

// 即时任务  ip列表更新            BF0C
void rttask_playlist_updata_init(uint8_t ip[],div_node_t *div_info_p);

void rttask_playlist_updata();


//读flash 阻塞时 10hz 协议处理线程
void task_pol_decode_process();

// 1hz计时任务运行线程
void timer_rttask_run_process();

// 音乐事件处理
void task_musicevent_change(uint8_t ch,char event,char data);

// 1S 定时任务计时
void timer_taskmusic_check();

// 任务检测
void task_check_and_play();
// 连续任务播放
void task_10hz_mutich_play();

// 任务停止
void task_music_config_stop(uint8_t ch);
// 停止所以播放任务
void task_music_stop_all();

// 即时任务开关状态复位
void rttask_build_overtime10hz();

// 关闭运行中即时任务
void close_running_rttask(uint8_t *mac);

void task_pageshow_recive();

void solulist_chk_forapp_recive();

void tasklist_forsolu_chk_recive();

#endif  //__TASK_DECODE_H_

