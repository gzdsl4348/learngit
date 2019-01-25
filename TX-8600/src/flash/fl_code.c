#include "fl_code.h"
#include <quadflashlib.h>
#include "hwlock.h"
#include "swlock.h"
#include "debug_print.h"
#include "flash_user.h"
#include "string.h"

static uint8_t fl_sector_write_flag[SDRAM_FLASH_SECTOR_MAX_NUM]={0};
static uint8_t flash_init_flag = 0;
static swlock_t g_flash_swlock;

static uint8_t start_write_backup2flash_flag = 0;

void fl_code_init()
{
    swlock_init(&g_flash_swlock);
    memset(fl_sector_write_flag, 0, sizeof(fl_sector_write_flag));
}

void set_flash_sector_write_flag(int sector)
{
    swlock_acquire(&g_flash_swlock);
    if(start_write_backup2flash_flag == 0)
    {
        fl_sector_write_flag[sector] = 100;//500ms—” ±
    }
    swlock_release(&g_flash_swlock);
}

uint8_t can_write_flash_sector(int sector)
{
    uint8_t res = 0;
    
    swlock_acquire(&g_flash_swlock);
    
    if(fl_sector_write_flag[sector])
    {
        if(--fl_sector_write_flag[sector] == 0)
            res = 1;
    }

    swlock_release(&g_flash_swlock);

    return res;
}

void set_flash_init_flag()
{
    flash_init_flag = 1;
}

uint8_t is_flash_init()
{
    return flash_init_flag;
}

uint8_t is_flash_write_complete()
{
    uint8_t res = 1;
    
    swlock_acquire(&g_flash_swlock);
    
    for(int i=0; i<SDRAM_FLASH_SECTOR_MAX_NUM; i++)
    {
        if(fl_sector_write_flag[i])
        {
            res = 0;
            break;
        }
    }
    swlock_release(&g_flash_swlock);

    return res;
}

uint8_t start_write_backup2flash()
{
    start_write_backup2flash_flag = 1;
    
    swlock_acquire(&g_flash_swlock);
    
    for(int i=BACKUP_DATA_START_SECTOR; i<SDRAM_FLASH_SECTOR_MAX_NUM; i++)
        fl_sector_write_flag[i] = 1;
    
    swlock_release(&g_flash_swlock);

    return 1;
}

uint8_t is_write_backup2flash()
{
    return start_write_backup2flash_flag;
}

void write_backup2flash_complete()
{
    start_write_backup2flash_flag = 0;
}

void get_write_backup2flash_progress(uint8_t *complete, uint32_t *total, uint32_t *writed)
{
    if(complete)
    {
        *complete = (start_write_backup2flash_flag==0);
    }
    if(total)
    {
        *total = SDRAM_FLASH_SECTOR_MAX_NUM;
    }
    if(writed)
    {
        *writed = 0;
        for(int i=0; i<SDRAM_FLASH_SECTOR_MAX_NUM; i++)
        {
            if(fl_sector_write_flag[i]==0)
            {
                (*writed)++;
            }
        }
    }
    //debug_printf("get_write_backup2flash_progress %d %d %d\n", *complete, *total, *writed);
}

