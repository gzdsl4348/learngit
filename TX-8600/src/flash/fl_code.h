#ifndef __FLCODE_H_
#define __FLCODE_H_
#include <stdint.h>
#include <xccompat.h>


void fl_code_init();

void set_flash_sector_write_flag(int sector);

uint8_t can_write_flash_sector(int sector);

void set_flash_init_flag();

uint8_t is_flash_init();

uint8_t is_flash_write_complete();

uint8_t start_write_backup2flash();

uint8_t is_write_backup2flash();

void write_backup2flash_complete();

void get_write_backup2flash_progress(REFERENCE_PARAM(uint8_t,complete), REFERENCE_PARAM(uint32_t,total), REFERENCE_PARAM(uint32_t,writed));

#endif  //__FLCODE_H_

