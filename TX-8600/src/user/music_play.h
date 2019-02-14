#ifndef _MUSIC_PLAY_H_
#define _MUSIC_PLAY_H_

#include <stdint.h>

void task_music_send(uint8_t ch);

void task_music_stop(uint8_t ch);

void task_music_play(uint8_t ch,uint8_t num);

#endif

