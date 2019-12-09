#include "flash_user.h"
#include "fl_code.h"
//#include <stdio.h>
#include <string.h>
#include "file_list.h"
#include "debug_print.h"
#include "image_upgrade.h"
#include "reboot.h"
#include "file.h"
on tile[0]: out port p_power_led = XS1_PORT_4C; 

#define FL_TX_CLK (100000000/115200)
on tile[0]: out port p_uart0_tx = XS1_PORT_1G; 
on tile[0]: in port  p_uart0_rx = XS1_PORT_1H; 

//==========================================================================================
//Flash Config => 080 => 1M Byte    Boot Partition For 3072 Page
//                                  Data Partition For 1024 Page
//                                  All  Page 4096
//                                       1Page = 256 Byte
//==========================================================================================
fl_QuadDeviceSpec flash_spec_h[2]={FLASH080,FLASHGD64};

fl_QSPIPorts q_flport = {
                PORT_SQI_CS,
                PORT_SQI_SCLK,
                PORT_SQI_SIO,
                on tile[0]: XS1_CLKBLK_1
};

extern void c_read_flash_bytes(uint32_t flash_addr, uint32_t buff_addr, int br);
extern void c_read_flash_secoter(uint32_t secoter, uint32_t buff_addr, int br);
extern void c_write_flash_secoter(uint32_t secoter, uint32_t buff_addr, int bw);

void fl_manage_uart_tx(uint8_t data) {
    int t;
    p_uart0_tx <: 0 @ t; //send start bit and timestamp (grab port timer value)
    t += FL_TX_CLK;
#pragma loop unroll(8)
    for(int i = 0; i < 8; i++) {
        p_uart0_tx @ t <: >> data; //timed output with post right shift
        t += FL_TX_CLK;
    }
    p_uart0_tx @ t <: 1; //send stop bit
    t += FL_TX_CLK;
    p_uart0_tx @ t <: 1; //wait until end of stop bit
}

static uint8_t wifi_datlen=0;
static uint8_t uart_dattmp[64];

void send_uart_wifibuf(){
    for(uint8_t i=0;i<wifi_datlen;i++){
        fl_manage_uart_tx(uart_dattmp[i]);
    }
}

//
//==========================================================================================
// Flash Read and Write Process
// Interface : i_flash
//      fun1 : void write(uint8_t write_buf[], uint8_t num, uint8_t base_adr);
//      fun2 : void read(uint8_t read_buf[], uint8_t num, uint8_t base_adr);
//==========================================================================================
void flash_process(server image_upgrade_if i_image,streaming chanend c_sdram)
{
	//=============================================================================
	// flash preprocess 
	//=============================================================================
    //Flash User Data Read Tmp Buff
    timer tmr;
    unsigned timeout;
    s_sdram_state sdram_state;
    uint8_t  tmp_buff[256];
    uint32_t j, i, t1, t2;
    
    unsigned *pw_buff = (unsigned*)tmp_buff;
    
	//--------------------------------------------------------------
  	//Process Reset Get 256Byte Flash Tmp Buff  To FlashTmpBuff
    //-------------------------------------------------------------
    if(fl_connectToDevice(q_flport,flash_spec_h,2)){
        //printf("ConnectFail\n");    
    }      
    _image_upgrade_init();

    sdram_init_state(c_sdram, sdram_state);
    
    tmr :> t1;
    
    for(i=0; i<SDRAM_FLASH_SECTOR_MAX_NUM; i++)
    {
        
        for(j=0; j<16; j++)
        {
            fl_readDataPage(16*i+j, (uint8_t*)pw_buff);
            sdram_write(c_sdram, sdram_state, (4096/4)*i+(256/4)*j, (256/4), pw_buff);   
            sdram_complete(c_sdram, sdram_state);
        }
    }
    
    tmr :> t2;

    //debug_printf("flash to sdram size:%d byte time:%d us\n",4096*SDRAM_FLASH_SECTOR_MAX_NUM, (t2-t1)/100);
    set_flash_init_flag();
    
    tmr :> timeout;
    timeout += 5*100000;

	//
	//=====================================================================
 	// Process Main Loop
 	//=====================================================================
	while(1)
    {
        select{
            case i_image.get_image_upgrade_reply(int &event, int &data):
            {
                _get_image_upgrade_reply(event, data);
                break;
            }
            case i_image.begin_image_upgrade(unsigned int image_size) -> int res:
            {
                res = _begin_image_upgrade(image_size);
                break;
            }
            case i_image.stop_image_upgrade(int error):
            {
                _stop_image_upgrade(error);
                break;
            }            
            case i_image.put_image_data(unsigned char data[], unsigned int num, char last_page_flag) -> int idle_size:
            {
                KFIFO_PUT(g_upgrade_manager.kf, data, num);
                idle_size = IMAGE_UPGRADE_FIFO_SIZE-KFIFO_SIZE(g_upgrade_manager.kf);
                
                g_upgrade_manager.last_page_flag = last_page_flag;
                break;
            }
            
            case tmr  when timerafter(timeout) :> timeout://5ms
            {
                timeout += 5*100000;
                
                _image_upgrade_poll(i_image, 5);

                static uint32_t index = 0;
#if 0
                static int old_value = 0;
                if(!old_value && is_write_backup2flash()) index = 759;
                old_value = is_write_backup2flash();
#endif
                for(i=index; i<(index+SDRAM_FLASH_SECTOR_MAX_NUM); i++)
                {
                    j = i%SDRAM_FLASH_SECTOR_MAX_NUM;
                    if(can_write_flash_sector(j))
                    {  
                        // 写flash操作从FLASH_DAT_SECTOR_BASE开始, 写备份操作从BACKUP_DATA_START_SECTOR开始
                        if(j>=(is_write_backup2flash()?BACKUP_DATA_START_SECTOR:FLASH_DAT_SECTOR_BASE))
                        {
                            tmr :> t1;
                            //erase and write one sector
                            fl_eraseDataSector(j);                    
                            for(uint16_t n=0; n<16; n++)
                            {
                                sdram_read(c_sdram, sdram_state, (is_write_backup2flash()?SDRAM_DATA_BACKUP_START:0)+(4096/4)*j+(256/4)*n, (256/4), pw_buff);
                                sdram_complete(c_sdram, sdram_state);
                                fl_writeDataPage(16*j+n, (uint8_t*)pw_buff);
                            }
                            tmr :> t2;
                            uint8_t c;
                            uint32_t t;
                            uint32_t w;
                            get_write_backup2flash_progress(c, t, w);                            
                            //debug_printf("fw[%d] [w:%d][%d %d] %dms\n", j, w,i, (index+SDRAM_FLASH_SECTOR_MAX_NUM), (t2-t1)/100000);
                        }
             
                        
                        if(is_write_backup2flash()) 
                        {
                            // 当is_write_backup2flash()成立时, 会出现i一定小于(index+SDRAM_FLASH_SECTOR_MAX_NUM)的情况
                            // 所以要执行i = 0;
                            if(index++ >= SDRAM_FLASH_SECTOR_MAX_NUM) index = 0;
                            i = index;
                            continue;
                        }
                        else 
                        { 
                            if(index++ >= SDRAM_FLASH_SECTOR_MAX_NUM) index = 0;                               
                            break;
                        }
                    }
                }
                if(is_write_backup2flash()) debug_printf("backup2flash break\n");
                if(is_write_backup2flash() && is_flash_write_complete())  
                {
                    tmr :> t1;
                    for(i=0; i<SDRAM_FLASH_SECTOR_MAX_NUM; i++)
                    {
                        if(i<BACKUP_DATA_START_SECTOR) continue;
                        
                        for(j=0; j<16; j++)
                        {
                            sdram_read(c_sdram, sdram_state, SDRAM_DATA_BACKUP_START+(4096/4)*i+(256/4)*j, (256/4), pw_buff);                              
                            sdram_complete(c_sdram, sdram_state);
                            sdram_write(c_sdram, sdram_state, (4096/4)*i+(256/4)*j, (256/4), pw_buff);
                            sdram_complete(c_sdram, sdram_state);
                        }
                    }
                    tmr :> t2;
                    debug_printf("write_backup2flash_complete %dms\n", (t2-t1)/100000);
                    write_backup2flash_complete();
                }

                static uint8_t led_tim=0;
                static uint8_t flash_f=0;
                led_tim ++;
                if(led_tim>100){
                    led_tim;
                    flash_f^=0xFF;
                    p_power_led <: flash_f;
                }
                break;
            }
        }//select
    }	
	//===========================================================================================
}

//===============================================================================================
// 非阻塞线程
//===============================================================================================
extern f_opr_mgr_t g_fopr_mgr;
[[combinable]]
void user_flash_manage(server fl_manage_if if_fl_manage[n_fl_manage],static const unsigned n_fl_manage,streaming chanend c_sdram){
    timer tmr;
    uint32_t i, t1, t2;
    
    uint8_t tmp_buff[1572];
    uint32_t write_backup_offset = 0;
    
    s_sdram_state sdram_state;
    unsigned *pw_buff = (unsigned*)tmp_buff;
    
    fl_code_init();
    
    sdram_init_state(c_sdram, sdram_state);
    
	while(1){
	    select{
            case if_fl_manage[unsigned a].start_write_backup():
            {
                write_backup_offset = 0;
                break;
            }
            case if_fl_manage[unsigned a].write_backup(uint32_t size, uint8_t buff[]):
            {
                for(i=0; i<(size/256); i++)
                {
                    memcpy(tmp_buff, buff+256*i, 256);
                    sdram_write(c_sdram, sdram_state, SDRAM_DATA_BACKUP_START+(write_backup_offset/4)+(256/4)*i, (256/4), pw_buff);
                    sdram_complete(c_sdram, sdram_state);
                }

                write_backup_offset += size;
                break;
            }
            case if_fl_manage[unsigned a].start_write_backup2flash():
            {
                start_write_backup2flash();
                break;
            }
            case if_fl_manage[unsigned a].get_write_backup2flash_progress(uint8_t &complete, uint32_t &total, uint32_t &writed):
            {
                uint8_t c;
                uint32_t t;
                uint32_t w;
                get_write_backup2flash_progress(c, t, w);
                complete = c;
                // BACKUP_DATA_START_SECTOR后的数据块执行备份
                total = t-BACKUP_DATA_START_SECTOR+1;
                writed = (w>BACKUP_DATA_START_SECTOR)?(w-BACKUP_DATA_START_SECTOR):0;
                break;
            }
            case if_fl_manage[unsigned a].read_backup(uint32_t address, uint32_t size, uint8_t buff[]):
            {
                for(i=0; i<(size/256); i++)
                {
                    sdram_read(c_sdram, sdram_state, (address/4)+(256/4)*i, (256/4), pw_buff);
                    sdram_complete(c_sdram, sdram_state);
                    memcpy(buff+256*i, tmp_buff, 256);
                }
                break;
            }
            case if_fl_manage[unsigned a].is_flash_init_complete()-> uint8_t res:
            {
                res = !is_flash_init();
                break;
            }
            case if_fl_manage[unsigned a].is_flash_write_complete() -> uint8_t res:
            {
                res = is_flash_write_complete();
                break;
            }
            case if_fl_manage[unsigned a].flash_sector_write(unsigned sector_num,uint8_t buff[]):
            {
                tmr :> t1;
                for(i=0; i<16; i++)
                {
                    memcpy(tmp_buff, buff+256*i, 256);
                    sdram_write(c_sdram, sdram_state, (4096/4)*sector_num+(256/4)*i, (256/4), pw_buff);  
                    sdram_complete(c_sdram, sdram_state);
                }
                //fl_sector_write_flag[sector_num]= 1;
                set_flash_sector_write_flag(sector_num);
                tmr :> t2;
                debug_printf("\nflash_sector_write %dus %d\n\n", (t2-t1)/100, sector_num);
                break;
            }
            case if_fl_manage[unsigned a].flash_sector_read(unsigned sector_num, uint8_t buff[]):
            {
                for(i=0; i<16; i++)
                {
                    sdram_read(c_sdram, sdram_state, (4096/4)*sector_num+(256/4)*i, (256/4), pw_buff);
                    sdram_complete(c_sdram, sdram_state);
                    memcpy(buff+256*i, tmp_buff, 256);
                }
                break;
            }
            // user 读音乐库文件夹列表 [int n + n*dir_info_t]
            case if_fl_manage[unsigned a].read_dirtbl(uint8_t buff[], int btr, int &br):
            {
                //fl_readDataPage(SDRAM_FILE_LIST_START*16,tmp_buff);
                if(g_fopr_mgr.sdcard_status == SD_CARD_NO_FOUND)
                    break;
                sdram_read(c_sdram, sdram_state, SDRAM_FILE_LIST_START, (256/4), pw_buff);
                sdram_complete(c_sdram, sdram_state);
                memcpy(buff, tmp_buff, 256);
                br = 4 + (tmp_buff, int[])[0]*sizeof(dir_info_t);
                for(i=1; i<=(br/256); i++)
                {
                    //fl_readDataPage(SDRAM_FILE_LIST_START*16+i,tmp_buff);
                    
                    text_debug2("in \n");
                    sdram_read(c_sdram, sdram_state, SDRAM_FILE_LIST_START+(256/4)*i, (256/4), pw_buff);                 
                    sdram_complete(c_sdram, sdram_state);
                    memcpy(buff+i*256, tmp_buff, 256);
                }
                break;
            }
            // user 读音乐文件列表 [int n + n*music_info_t]
        case if_fl_manage[unsigned a].read_musictbl(uint8_t music_index, uint8_t buff[], int btr, int &br):
            {
                sdram_read(c_sdram, sdram_state, (SDRAM_FILE_LIST_START+music_index*SDRAM_FILE_LIST_SECTOR_SIZE), (256/4), pw_buff);
                sdram_complete(c_sdram, sdram_state);
                memcpy(buff, tmp_buff, 256);
            
                br = 4 + (tmp_buff, int[])[0]*sizeof(music_info_t);
            
                for(i=1; i<=(br/256); i++)
                {
                    sdram_read(c_sdram, sdram_state, (SDRAM_FILE_LIST_START+music_index*SDRAM_FILE_LIST_SECTOR_SIZE)+(256/4)*i, (256/4), pw_buff); 
                    sdram_complete(c_sdram, sdram_state);
                    memcpy(buff+i*256, tmp_buff, 256);
                }
                break;
            }           
        // 批处理临时音乐文件名读取
        case if_fl_manage[unsigned a].if_fl_music_tmpbuf_read(unsigned num,uint8_t buff[]):
            sdram_read(c_sdram, sdram_state, USER_MUSICNAME_TMP_BASE+num*64/4, (64/4), pw_buff);
            sdram_complete(c_sdram, sdram_state);
            memcpy(buff, tmp_buff, 64);
            break;
        // 批处理临时音乐文件名存放
        case if_fl_manage[unsigned a].if_fl_music_tmpbuf_write(unsigned num,uint8_t buff[]):
            memcpy(tmp_buff, buff, 64);
            sdram_write(c_sdram, sdram_state, USER_MUSICNAME_TMP_BASE+num*64/4, (64/4), pw_buff);
            sdram_complete(c_sdram, sdram_state);
            break;
        // 搜索设备临时存放
        case if_fl_manage[unsigned a].if_fl_divinfo_tmpbuf_write(unsigned num,uint8_t buff[]):
            memcpy(tmp_buff, buff, 200);
            sdram_write(c_sdram, sdram_state, USER_DIV_SEARCH_BASE+num*200/4, (200/4), pw_buff);
            sdram_complete(c_sdram, sdram_state);
            break;
        // 搜索设备读取
        case if_fl_manage[unsigned a].if_fl_divinfo_tmpbuf_read(unsigned num,uint8_t buff[]):
            sdram_read(c_sdram, sdram_state, USER_DIV_SEARCH_BASE+num*200/4, (200/4), pw_buff);
            sdram_complete(c_sdram, sdram_state);
            memcpy(buff, tmp_buff, 200);
            break;
        // xtcp fifo 写入
        case if_fl_manage[unsigned a].xtcp_buff_fifo_put(uint8_t num,uint8_t buff[],uint8_t tx_rx_f):
            unsigned data_base;
            if(tx_rx_f){
                data_base = USER_XTCP_TXFIFO_BASE;
            }else{
                data_base = USER_XTCP_RXFIFO_BASE;
            }
            memcpy(tmp_buff, buff, 1472);
            sdram_write(c_sdram, sdram_state, data_base+num*1472/4, (1472/4), pw_buff);
            sdram_complete(c_sdram, sdram_state);
            break;
        // xtcp fifo 读取
        case if_fl_manage[unsigned a].xtcp_buff_fifo_get(uint8_t num,uint8_t buff[],uint8_t tx_rx_f):
            unsigned data_base;
            if(tx_rx_f){
                data_base = USER_XTCP_TXFIFO_BASE;
            }else{
                data_base = USER_XTCP_RXFIFO_BASE;
            }
            sdram_read(c_sdram, sdram_state, data_base+num*1472/4, (1472/4), pw_buff);
            sdram_complete(c_sdram, sdram_state);
            memcpy(buff,pw_buff,1472);
            break;
		// 消息更新 写入
	    case if_fl_manage[unsigned a].if_messend_buff_put(uint8_t wptr,uint8_t buff[]):
            memcpy(tmp_buff, buff, 1472);
            sdram_write(c_sdram, sdram_state, USER_MESSEND_BUFF_BASE+wptr*1472/4, (1472/4), pw_buff);
            sdram_complete(c_sdram, sdram_state);
			break;
		// 消息读取 写入
		case if_fl_manage[unsigned a].if_messend_buff_get(uint8_t rptr,uint8_t buff[]):
			sdram_read(c_sdram, sdram_state, USER_MESSEND_BUFF_BASE+rptr*1472/4, (1472/4), pw_buff);
			sdram_complete(c_sdram, sdram_state);
			memcpy(buff,pw_buff,1472);
			break;
        // 
        case if_fl_manage[unsigned a].rttask_nameinfo_get(task_music_info_t &music_info,uint8_t ch):
			sdram_read(c_sdram, sdram_state, USER_RTTASKINFO_BASE+sizeof(task_music_info_t)*ch, sizeof(task_music_info_t), pw_buff);
            sdram_complete(c_sdram, sdram_state);
			memcpy(&music_info,pw_buff,sizeof(task_music_info_t));
            break;
        case if_fl_manage[unsigned a].rttask_nameinfo_put(task_music_info_t music_info,uint8_t ch):
            memcpy(tmp_buff,&music_info,sizeof(task_music_info_t));
            sdram_write(c_sdram, sdram_state, USER_RTTASKINFO_BASE+sizeof(task_music_info_t)*ch, sizeof(task_music_info_t), pw_buff);
            sdram_complete(c_sdram, sdram_state);
            break;
		// wifi串口
        case if_fl_manage[unsigned a].uart0_tx(uint8_t data[],uint8_t len,uint8_t mode):
            switch(mode){
                case 0:
                    for(uint8_t i=0;i<len;i++){
                        fl_manage_uart_tx(data[i]);
                    }
                    break;
                case 1:
                    p_uart0_tx <: 1;
                    break;
                case 2:
                    p_uart0_tx <: 0;
                    break;
            } //switch
            break;
        case if_fl_manage[unsigned a].wifi_uartsend(uint8_t mode):
            switch(mode){
                case D_WIFI_DHCP_EN:        
                    static char disdhcp[]={0x61,0x74,0x2B,0x44,0x68,0x63,0x70,0x64,0x3D,0x31,0x0D,0x0A};
                    wifi_datlen = 12;
                    memcpy(uart_dattmp,disdhcp,wifi_datlen);
                    break;
                case D_WIFI_DHCP_DIS:
                    static char endhcp[]={0x61,0x74,0x2B,0x44,0x68,0x63,0x70,0x64,0x3D,0x30,0x0D,0x0A}; 
                    wifi_datlen = 12;                
                    memcpy(uart_dattmp,endhcp,wifi_datlen);
                    break;
                case D_WIFI_SAVE:
                    static char wifisave[]={0x61,0x74,0x2B,0x53,0x61,0x76,0x65,0x3D,0x31,0x0D,0x0A}; 
                    wifi_datlen = 11;                     
                    memcpy(uart_dattmp,wifisave,wifi_datlen);
                    break;
                case D_WIFI_APPLY:
                    static char wifiapply[]={0x61,0x74,0x2B,0x41,0x70,0x70,0x6C,0x79,0x3D,0x31,0x0D,0x0A}; //12
                    wifi_datlen = 12;                     
                    memcpy(uart_dattmp,wifiapply,wifi_datlen);
                    break;
            }            
            send_uart_wifibuf();
            break;
            
        case if_fl_manage[unsigned a].wifi_uart_setip(uint8_t ip[]):
                #define IPSET_LEN 9
                char wifi_ipset[]={0x61,0x74,0x2B,0x4C,0x41,0x4E,0x49,0x70,0x3D}; 
                //#define MASKSET_LEN 13
                //char wifi_maskset[]={0x61,0x74,0x2B,0x4C,0x41,0x4E,0x49,0x70,0x4D,0x61,0x73,0x6B,0x3D};
                //
                uint8_t ip_adrbase;
 
                memcpy(&uart_dattmp[0],wifi_ipset,IPSET_LEN);
                ip_adrbase = IPSET_LEN;
                
                uint8_t tmp_f;
                for(uint8_t i=0;i<4;i++){
                    tmp_f=0;
                    if(ip[i]/100 != 0){
                        uart_dattmp[ip_adrbase] = ip[i]/100+0x30;
                        ip_adrbase++;
                        tmp_f = 1;
                    }
                    if(((ip[i]%100)/10!=0)||tmp_f){
                        uart_dattmp[ip_adrbase] = (ip[i]%100)/10+0x30;
                        ip_adrbase++;
                    } 
                    uart_dattmp[ip_adrbase] = ip[i]%10 + 0x30;
                    ip_adrbase++;
                    if(i!=3){
                        uart_dattmp[ip_adrbase] = 0x2E;
                        ip_adrbase++;
                    }
                }
                //增加结束符
                uart_dattmp[ip_adrbase] = 0x0D;
                uart_dattmp[ip_adrbase+1] = 0x0A;
                ip_adrbase+=2;
                
                wifi_datlen = ip_adrbase;
                send_uart_wifibuf();
             break;
        }//select       
	}
}





