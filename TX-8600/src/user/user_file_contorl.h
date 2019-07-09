/*
*  File Name:remotel_business_base.h
*  Created on: 2019年5月20日
*  Author: caiws
*  description : 文件操作处理部分	
*  Modify date: 2019年5月20日
* 	Modifier Author: 无
*  description : 无
*/

#ifndef __USER_FILE_CONTORL_H_
#define __USER_FILE_CONTORL_H_

#include "stdint.h"

void music_patch_list_chk_recive();

void music_patch_list_send_decode(uint8_t list_num);

void music_music_list_chk_recive();

void music_music_list_send_decode(uint8_t list_num);

void music_patchname_config_recive();

void music_busy_chk_recive();

void music_file_config_recive();

void file_contorl_ack_decode(uint8_t error_code);

void musicfile_bar_chk_recive();

void music_bat_contorl_recive();

void music_bat_info_recive();

void file_bat_contorl_event(uint8_t error_code);

void bat_filecontorl_resend_tim();

void sdcard_sizechk_recive();

/*
*@description: 批量处理音乐断线查询
*@Author: ljh
*@param[out]: 无
*@param[in]: 无
*@return: 无
*@Modify date:
*@Modifier Author:ljh
*@others 无
*/
void music_batrechk_recive();

void wav_modeset_recive();

#endif

