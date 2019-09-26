#ifndef _MUSIC_PLAY_H_
#define _MUSIC_PLAY_H_

#include <stdint.h>
#include "sys_config_dat.h"

void task_music_send(uint8_t ch,taskmac_info_t *p_taskmac_info,uint8_t div_tol,uint8_t task_prio,uint8_t task_vol);

void task_music_stop(uint8_t ch);

void task_music_play(uint8_t ch,uint8_t num,task_music_info_t *p_music_info);

// 即时任务控制

// 音乐播放
void rttask_music_play(uint16_t id);
// 音乐停止
void rttask_music_stop(uint16_t id);
// 上一曲
void rttask_music_last(uint16_t id);
// 下一曲
void rttask_music_next(uint16_t id);
// 音乐选择
void rttask_music_select(uint16_t id,uint8_t mus_inc);
// 选择音乐时间
void rttask_music_totimer(uint16_t id,uint16_t music_sec);
// 选择音乐模式
void rttask_music_setmode(uint16_t id,uint8_t mode);
// 设置音乐音量
void rttask_music_setvol(uint16_t id,uint8_t vol);

uint16_t get_music_tolsec(task_music_info_t *p_music_info);


#endif

