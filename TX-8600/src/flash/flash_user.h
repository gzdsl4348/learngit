#ifndef  __FLASH_H_
#define  __FLASH_H_

#include <xs1.h>
#include <platform.h>
#include <stdint.h>

#include "flash_adrbase.h"

#include "flash_spec.h"
#include <quadflashlib.h>

#include "sdram.h"
#include "sdram_def.h"

#define UPGRADE_END_REPLY   1
#define UPGRADE_BUFF_SIZE   2

#define BACKUP_DATA_START_SECTOR SOLUSION_DAT_SECTOR

enum WIFI_TYPE_E{ 
    D_WIFI_DHCP_EN=0x00,
    D_WIFI_DHCP_DIS,
    D_WIFI_SAVE,
    D_WIFI_APPLY,
};
    
#ifdef __XC__

typedef interface image_upgrade_if
{
    int begin_image_upgrade(unsigned int _image_size);
    
    int put_image_data(unsigned char data[], unsigned int num, char last_page_flag);

    void stop_image_upgrade(int error);
    
    [[clears_notification]] void get_image_upgrade_reply(int &event, int &data);
    
    [[notification]] slave void image_upgrade_ready(void);

} image_upgrade_if;

//-------------------------------------------------------------------------
// flash manage
//-------------------------------------------------------------------------
typedef interface fl_manage_if{
    //write backup
     void start_write_backup();
     void write_backup(uint32_t size, uint8_t buff[]);
     void start_write_backup2flash();
     void get_write_backup2flash_progress(uint8_t &complete, uint32_t &total, uint32_t &writed);
    
     //read backup
     void read_backup(uint32_t address, uint32_t size, uint8_t buff[]);
    
     //flash 
    uint8_t is_flash_init_complete();

    uint8_t is_flash_write_complete();
    
    void flash_sector_write(unsigned sector_num,uint8_t buff[]);
    
    void flash_sector_read(unsigned sector_num,uint8_t buff[]);

    //user
    void read_dirtbl(uint8_t buff[], int btr, int &br);

    void read_musictbl(uint8_t music_index, uint8_t buff[], int btr, int &br);

    //uart0
    void uart0_tx(uint8_t data[],uint8_t len,uint8_t mode);

    void if_fl_music_tmpbuf_read(unsigned num,uint8_t buff[]);

    void if_fl_music_tmpbuf_write(unsigned num,uint8_t buff[]);

    void if_fl_divinfo_tmpbuf_write(unsigned num,uint8_t buff[]);

    void if_fl_divinfo_tmpbuf_read(unsigned num,uint8_t buff[]);

    void xtcp_buff_fifo_get(uint8_t num,uint8_t buff[],uint8_t tx_rx_f);

    void xtcp_buff_fifo_put(uint8_t num,uint8_t buff[],uint8_t tx_rx_f);

	void if_messend_buff_put(uint8_t wptr,uint8_t buff[]);

	void if_messend_buff_get(uint8_t rptr,uint8_t buff[]);

    void rttask_nameinfo_get(task_music_info_t &music_info,uint8_t ch);
    
    void rttask_nameinfo_put(task_music_info_t music_info,uint8_t ch);

    void wifi_uartsend(uint8_t mode);

    void wifi_uart_setip(uint8_t ip[]);
}fl_manage_if;

//
//-----------------------------------------------------------------------------------
// Flash Read and Write Process
// Interface : i_flash
//      fun1 : void write(uint8_t write_buf[], uint8_t num, uint8_t base_adr);
//      fun2 : void read(uint8_t read_buf[], uint8_t num, uint8_t base_adr);
//
// user demo:	
//		i_flash.write(read_buf,6,FLASH_ADR_MAC);
//		i_flash.read(read_buf,6,FLASH_ADR_MAC);
//
//-----------------------------------------------------------------------------------
void flash_process(server image_upgrade_if i_image,streaming chanend c_sdram);

//----------------------------------------------------------------------------------
//
[[combinable]]
void user_flash_manage(server fl_manage_if if_fl_manage[n_fl_manage],static const unsigned n_fl_manage,streaming chanend c_sdram);
//==================================================================================================
#endif

#endif	//__FLASH_H_


