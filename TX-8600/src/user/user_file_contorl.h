#ifndef __USER_FILE_CONTORL_H_
#define __USER_FILE_CONTORL_H_

#include "stdint.h"

void music_patch_list_chk_recive();

void music_patch_list_send_decode();

void music_music_list_chk_recive();

void music_music_list_send_decode();

void music_patchname_config_recive();

void music_busy_chk_recive();

void music_file_config_recive();

void file_contorl_ack_decode(uint8_t error_code);

void musicfile_bar_chk_recive();

void music_bat_contorl_recive();

void music_bat_info_recive();

void file_bat_contorl_event(uint8_t error_code);

void bat_filecontorl_resend_tim();

#endif

