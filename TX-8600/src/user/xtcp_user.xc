#include "xtcp_user.h"
#include "mac_factory_write.h"
#include "account.h"
#include "list_contorl.h"
#include "conn_process.h"
#include "list_instance.h"
#include "timer_process.h"
#include "user_unti.h"
#include "bsp_ds1302.h"
#include "fl_buff_decode.h"
#include "task_decode.h"
#include "tftp.h"
#include "user_xccode.h"
#include "flash_user.h"
#include "task_decode.h"
#include "uart.h"
#include "user_lcd.h"
#include "user_file_contorl.h"
#include "user_messend.h"
#include "reboot.h"
#include "ack_build.h"
#include "checksum.h"

#include "debug_print.h"
#include "string.h"
#include "stdio.h"

on tile[1]: out port p_wifi_on = XS1_PORT_4B; 

on tile[1]: in port p_wifi_chk = XS1_PORT_4A; 

on tile[1]: out port p_eth_reset = XS1_PORT_1B;

void gateway_event(client xtcp_if i_xtcp);
//=======================================================================================================
//Gobal_Val And Code Option Define Here 
//-------------------------------------------------------------------------------------------------------
//----------------------------------
// XTCP TX&RX Buffer 
char all_rx_buf[RX_BUFFER_SIZE];
char all_tx_buf[TX_BUFFER_SIZE];

char *unsafe xtcp_rx_buf;
char *unsafe xtcp_tx_buf;
//--------------------------------
// XTCP sending len
uint16_t user_sending_len=0; 
//---------------------------------
// XTCP conn event 
xtcp_connection_t conn;  // A temporary variable to hol
static xtcp_connection_t gateway_conn={0};

extern uint8_t gateway_mac[];
//---------------------------------
//
//audio eth info
audio_ethinfo_t t_audio_ethinfo;
//------------------------------------------------------
//client point
client interface xtcp_if   * unsafe i_user_xtcp = NULL;
client interface fl_manage_if  * unsafe i_user_flash = NULL;
client interface ethaud_cfg_if  * unsafe i_ethaud_cfg = NULL;
client interface file_server_if *unsafe i_fs_user = NULL;
client interface flash_if *unsafe i_core_flash = NULL;
client interface uart_rx_if *unsafe i_uart_rx=NULL;
client interface uart_tx_buffered_if *unsafe i_uart_tx=NULL;

//--------------------------------------------------------------
// extern val 
extern host_info_t host_info;

//=======================================================================================

//-------------------------------------------------------------------------------------------------------
void mac_writeflash(uint8_t macadr[6]){
    unsafe{
	// Get Host info
	i_user_flash->flash_sector_read(USER_DAT_SECTOR,tmp_union.buff);
	sys_dat_read((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);//主机信息读取
    //
    memcpy(host_info.mac,macadr,6);
	sys_dat_write((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);
    //
    while(i_user_flash->is_flash_write_complete());
    user_fl_sector_write(USER_DAT_SECTOR);
    //
    debug_printf("write: %x,%x,%x,%x,%x,%x\n",macadr[0],macadr[1],macadr[2],macadr[3],macadr[4],macadr[5]);
    }
}
//=========================================================================================================
//#define TFTP_TYPE_IMAGE     0
//#define TFTP_TYPE_FILE      1

static client file_server_if * unsafe pi_fs;
static client image_upgrade_if * unsafe pi_image;

static unsigned char tftp_ack_code = TFTP_ACK_IDLE;

#define TFTP_TYPE_IMAGE         0
#define TFTP_TYPE_FILE          1
#define TFTP_TYPE_WRITE_BACKUP  2
#define TFTP_TYPE_READ_BACKUP   3
static unsigned char tftp_type = 0;

static unsigned char tftp_block_wait = 0;
static int tftp_blksize = 512;

static uint8_t tftp_data_reverse = 0;
static uint8_t tftp_frist_block = 0;
static uint16_t tftp_upgrade_dev_type[] = {'T','X','-','8','6','0','0',0};

static void array_reverse(uint8_t a[], uint32_t size)
{
    for(int i=0; i<size; i++) a[i] = ~a[i];
}

static uint8_t tftp_upgrade_jude_dev_type(uint8_t block[])
{
#if 0   // 使能设备型号判断
    int i = 0;
    uint16_t len = block[32] + (block[33]>>8);
    
    if(len > 512) len = 512;
    
    for(i=0; i<len; i++)
    {
        if(block[34+i] == '|') block[34+i] = 0;
    }

    for(i=0; i<(len-1); i++)
    {
        if(memcmp(tftp_upgrade_dev_type, block+34+i, sizeof(tftp_upgrade_dev_type)) == 0)
            return 1;
    }
    return 0;
#else   //失能设备型号判断
    return 1;
#endif
}

static uint8_t tftp_upgrade_jude_header(uint8_t block[])
{
    return (block[0]=='B'&&block[1]=='L'&&block[2]=='E');
}

void tftp_upgrade_reply_deal(client xtcp_if i_xtcp, client image_upgrade_if i_image)
{
    int event, data;
    
    i_image.get_image_upgrade_reply(event, data);
    
    switch(event)
    {
        case UPGRADE_END_REPLY:
        {
            if(data == 0)
                tftp_send_ack(i_xtcp, TFTP_ACK_SUCCEED, 0);
            else
                tftp_send_ack(i_xtcp, TFTP_ACK_FAILED, data);                    
            break;
        }
        case UPGRADE_BUFF_SIZE:
        {
            if(tftp_ack_code != TFTP_ACK_IDLE)
            {
                tftp_send_ack(i_xtcp, tftp_ack_code, 0);
                tftp_ack_code = TFTP_ACK_IDLE;
            }
            break;
        }                
    }

}

void tftp_upload_reply_deal(client xtcp_if i_xtcp, char reply_type, char reply_data)
{
    switch(reply_type)
    {
        case FOU_REPLY_START:
        {
            if(reply_data == FOU_REPLY_SUCCEED){
                for(uint8_t i=0; i<MAX_MUSIC_CH;i++){
                    if(timetask_now.ch_state[i]!=0xFF){
                        timetask_now.ch_state[i]=0xFF;
                        task_music_config_stop(i);
                    }
                }
                tftp_send_ack(i_xtcp, TFTP_ACK_SUCCEED, 0);
            }
            else
                tftp_send_ack(i_xtcp, TFTP_ACK_FAILED, reply_data);                    
            break;
        }
        case FOU_REPLY_GET_DATA:
        {
            if(tftp_block_wait)
            {
                tftp_block_wait = 0;
                tftp_send_ack(i_xtcp, TFTP_ACK_SUCCEED, 0);
            }
            break;
        }
        case FOU_REPLY_END:
        {
            tftp_send_ack(i_xtcp, reply_data==FOU_REPLY_SUCCEED?TFTP_ACK_SUCCEED:TFTP_ACK_FAILED, 0);
            break;
        }
    }

}

int tftp_app_transfer_begin(unsigned char filename[], int tsize, int blksize, int write_mode)
{

    unsafe {
        debug_printf("tftp_app_transfer_begin %s %d\n", filename);
        
        g_sys_val.tftp_busy_f = 1;

        tftp_block_wait = 0;
        tftp_blksize = blksize;
        if(0==strcmp(filename, "upgrade_image.bin") || 0==strcmp(filename, "image.bin"))
        {
            //TFTP_SUPPORT_IMAGE
            tftp_type = TFTP_TYPE_IMAGE;
            tftp_frist_block = 1;
            tftp_data_reverse = 0;

            if(tftp_blksize==512 && pi_image->begin_image_upgrade(tsize) == 0)
                return SHORTLY_ACK_SUCCEED;
            else
                return SHORTLY_ACK_FAILED;            
        }
        else if(0==strcmp(filename, "backup.bin"))
        {
            if(write_mode)
            {
                tftp_type = TFTP_TYPE_WRITE_BACKUP;
                debug_printf("start_write_backup\n");
                //start_write_backup
                i_user_flash->start_write_backup();
            }
            else
            {
                tftp_type = TFTP_TYPE_READ_BACKUP;
                debug_printf("start_read_backup\n");
                //start_read_backup                
            }
            
            return SHORTLY_ACK_SUCCEED;
        }
        else
        {
            tftp_type = TFTP_TYPE_FILE;
            
            if(pi_fs->file_upload_start(filename, 100) == FOR_SUCCEED)
                return DELAYED_ACK;
            else
                return SHORTLY_ACK_FAILED;
        }
    }
}

void tftp_app_timer(int interval_ms)
{
    unsafe {
        if(tftp_type != TFTP_TYPE_FILE) return;
        if(tftp_block_wait && pi_fs->file_upload_get_fifo_size() >= tftp_blksize)
        {
            tftp_block_wait = 0;
            tftp_send_ack(*i_user_xtcp, TFTP_ACK_SUCCEED, 0);
        }
    }
}

int tftp_app_process_data_block(unsigned char data[], int num_bytes)
{
    unsafe {
        char is_last_block = (num_bytes!=tftp_blksize);
        int fifo_size = 0;
        //debug_printf("tftp_app_process_data_block num:%d\n", num_bytes);
        if(tftp_type == TFTP_TYPE_IMAGE)
        {
            if(tftp_frist_block)
            {
                tftp_frist_block = 0;
                
                //判断数据头与设备类型
                if(tftp_upgrade_jude_header(data))
                {
                    if(tftp_upgrade_jude_dev_type(data))
                    {
                        tftp_data_reverse = 1;
                        return SHORTLY_ACK_SUCCEED;
                    }
                    debug_printf("tftp_upgrade_jude_dev_type error\n");
                    return SHORTLY_ACK_FAILED;
                }
            }
            
            if(tftp_data_reverse) array_reverse(data, num_bytes);
            
            fifo_size = pi_image->put_image_data(data, num_bytes, is_last_block);
            if(is_last_block != 1)
            {
                if(fifo_size >= tftp_blksize)
                {
                    return SHORTLY_ACK_SUCCEED;
                }
                else
                {
                    tftp_ack_code = TFTP_ACK_SUCCEED;
                    return DELAYED_ACK;
                }
            }
            else
            {
                //wait end_reply
                return DELAYED_ACK;
            }

        }
        else if(tftp_type == TFTP_TYPE_FILE)
        {
            pi_fs->file_upload_put_data(data, num_bytes, is_last_block, fifo_size);

            if(is_last_block) //wait end_reply
            {
                return DELAYED_ACK;
            }
            
            if(fifo_size>=tftp_blksize)
            {
                return SHORTLY_ACK_SUCCEED;
            }
            else
            {
                tftp_block_wait = 1;
                return DELAYED_ACK;
            }
        }
        else if(tftp_type == TFTP_TYPE_WRITE_BACKUP)
        {
            //write_backup
            i_user_flash->write_backup(num_bytes, data);
            return SHORTLY_ACK_SUCCEED;
        }
    }
}
int last_block_num = 0;
int tftp_app_process_send_data_block(unsigned char tx_buf[], int block_num, int block_size, int &complete)
{
#define BACKUP_START_ADDRESS    (0)
#define BACKUP_SIZE             (SDRAM_FLASH_SECTOR_MAX_NUM*4096-BACKUP_START_ADDRESS)
    unsafe {

        //debug_printf("tftp_app_process_send_data_block #%d %d\n", block_num, block_size);
        i_user_flash->read_backup(BACKUP_START_ADDRESS+(block_num-1)*block_size, block_size, tx_buf);
        
        last_block_num = block_num;
        
        if(block_num*block_size > BACKUP_SIZE) 
        {
            debug_printf("tftp_app_process_send_data_block complete\n");
            complete = 1;
            return 0;
        }
        else if(block_num*block_size == BACKUP_SIZE)
        {
            complete = 0;
            return 0;
        }
        else
        {
            complete = 0;
            return block_size;
        }
    }
}
void tftp_app_transfer_complete(void)
{
    unsafe {
    g_sys_val.tftp_busy_f = 0;
    if(tftp_type==TFTP_TYPE_FILE)
    {
        mes_send_listinfo(MUSICLIS_INFO_REFRESH,0);
    }
    else if(tftp_type==TFTP_TYPE_IMAGE)
    {
        pi_image->stop_image_upgrade(0);
        //g_sys_val.reboot_f=1;
    }
    else if(tftp_type==TFTP_TYPE_WRITE_BACKUP)
    {
        //i_user_flash->start_write_backup2flash();
    }
    debug_printf("tftp_app_transfer_complete %d\n", tftp_type);
    }
}

void tftp_app_transfer_error(void)
{
    debug_printf("tftp_app_transfer_error [tftp_block_wait %d] [last_block_num %d]\n",tftp_block_wait,last_block_num);
    unsafe {
        if(tftp_type==TFTP_TYPE_FILE)
            pi_fs->file_upload_forced_stop();
        if(tftp_type==TFTP_TYPE_IMAGE)
            pi_image->stop_image_upgrade(0);
    }
    g_sys_val.tftp_busy_f = 0;
}


//=======================================================================================================
//------------------------------------------------------------------------------------
// flash 数据初始化函数
void user_fldat_init(){
    unsafe{
    unsigned init_string;
    //-------------------------------------------------------------
	//设备列表初始化
	//div_list_init();        
    //g_sys_val.fl_divlist_inc=0;
    //while(!timer_fl_divlist_decode()); //烧写
    
	//sys_dat_write((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);
    //user_fl_sector_write(USER_DAT_SECTOR);
    // sn
    //------------------------------------------------------------
    i_user_flash->flash_sector_read(USER_DAT_SECTOR,tmp_union.buff);
	sys_dat_read((char*)(&init_string),4,FLASH_ADR_INIT);   
    //init_string = 0;
	if(0x5AA57349==init_string){
		return;
	}
    //------------------------------------------------------------    
    // 账户列表初始化
    account_list_init();
    // 账户flash初始化
    for(uint8_t i=0;i<MAX_ACCOUNT_NUM;i++){
        tmp_union.account_all_info.account_info=account_info[i];
        account_fl_write(&tmp_union.account_all_info,i);
    }
    //-------------------------------------------------------------
    // 分区信息初始化
    area_list_init();
    while(!timer_fl_arealist_decode()); //烧写
    //设备列表初始化
	div_list_init();        
    g_sys_val.fl_divlist_inc=0;
    while(!timer_fl_divlist_decode()); //烧写
    // 任务列表初始化
    task_fl_init();
    // 即时任务链表初始化
    fl_rttask_dat_init();
    //
    //-------------------------------------------------------------
    // 用户信息初始化
    i_user_flash->flash_sector_read(USER_DAT_SECTOR,tmp_union.buff);
	init_string = 0x5AA57349;
	sys_dat_write((char*)(&init_string),4,FLASH_ADR_INIT);
    sys_dat_read((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);//主机信息读取
    //
    if((host_info.mac[0]==0x42)&&(host_info.mac[1]==0x4C)&&(host_info.mac[2]==0x45)){
        memcpy(host_info_tmp.mac,host_info.mac,6);
    }    
    memcpy(&host_info,&host_info_tmp,sizeof(host_info_t));
	sys_dat_write((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);
    user_fl_sector_write(USER_DAT_SECTOR);
    //-----------------------------------------------------------------------------------------------------
    // 方案信息初始化
    for(uint8_t i=0;i<MAX_TASK_SOULTION;i++)
        solution_list.solu_info[i].state=0xFF;
    //solution_list.solu_info[0].state=0x00;
    //solution_list.solu_info[0].id=0xFF;
    sys_dat_write((char*)(&solution_list),sizeof(solution_list_t),FLASH_SOLUSION_LIST);
    while(i_user_flash->is_flash_write_complete());
    user_fl_sector_write(SOLUSION_DAT_SECTOR);
    //--------------------------------------------------------------
    }//unsafe 
    debug_printf("reset flash\n");
}

void sys_gobalval_init(){
    // sn_key 密钥
    const uint8_t sn_key[DIV_NAME_NUM] = {0x5E,0x7F,0x5D,0xDE,0x5E,0x02,0x72,0x31,0x90,0x12,0x60,0x1D,0x75,0x35,0x5B,
                                          0x50,0x67,0x09,0x96,0x50,0x51,0x6C,0x53,0xF8,0x78,0x14,0x53,0xD1,0x90,0xE8,0x00,0x00};
	g_sys_val.eth_link_state=0;
    g_sys_val.need_flash=0;
    g_sys_val.fl_divlist_inc=0;
    g_sys_val.task_recid=0xFF;
    g_sys_val.task_rec_count = 0;
    g_sys_val.file_bat_conn.id = null;
    //
    memcpy(g_sys_val.sn_key,sn_key,DIV_NAME_NUM);
    //
	conn_sending_s.id =null;
	conn_sending_s.conn_state = CONN_INIT;
}

//--------------------------------------------------------------------------------------
// 系统信息获取与初始化
void sys_info_init(){
    unsafe{
    //-------------------------------------------------------------------------------------------------
	// Get Host info
	i_user_flash->flash_sector_read(USER_DAT_SECTOR,tmp_union.buff);
	sys_dat_read((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);//主机信息读取
	host_info.version[0] = VERSION_H;
    host_info.version[1] = VERSION_L;
	//----------------------------------------------------------------------------------------------------
	i_user_flash->flash_sector_read(SOLUSION_DAT_SECTOR,tmp_union.buff);
	sys_dat_read((char*)(&solution_list),sizeof(solution_list_t),FLASH_SOLUSION_LIST); //方案信息读取

    uint8_t mac[6] = {'B','L','E',0x00,0xDD,0x20};
    memcpy(host_info.mac, mac, 6);
    host_info.ipconfig.ipaddr[0] = 172;
    host_info.ipconfig.ipaddr[1] = 16;
    host_info.ipconfig.ipaddr[2] = 13;
    host_info.ipconfig.ipaddr[3] = 111;
    
    //
    area_fl_read();    //分区表读取
    divlist_fl_read(); //列表读取
    account_list_read();//账户列表读取
    timer_tasklist_read(); //定时任务列表读取
    rt_task_list_read();   //获取即时任务链表
    create_alltask_list(); //建立所有任务
    //建立今日任务
    //
    conn_list_init();   //链表初始化
    connlong_list_init(); //长连接初始化
    sys_gobalval_init(); //全局变量初始化
    }//unsafe
}

//=======================================================================================================
void read_link_up_event(){
    unsafe{
    g_sys_val.gateway_standy=1;
    //
    //if(g_sys_val.could_conn.id==0)
    //    i_user_xtcp->connect(7002,g_sys_val.could_ip,XTCP_PROTOCOL_TCP);
    }
}

//======================================================================================================
void dhcp_dis(){
    char disdhcp[]={0x61,0x74,0x2B,0x44,0x68,0x63,0x70,0x64,0x3D,0x31,0x0D}; 
    //user_lan_uart0_tx(disdhcp,11);
}

//=======================================================================================================
/*
//--------------------------------------------------------------------------------
		XTCP EVENT USER DECODE PROCESS  
//-------------------------------------------------------------------------------*/
static uint8_t g_mp3_test_start_flag = 0;

static void start_single_mp3_test(client ethaud_cfg_if if_ethaud_cfg, client file_server_if if_fs, int i)
{
    uint16_t fn[100] = {'1','.','m','p','3',0};
    
    
    audio_txlist_t audio_txlist = {{{{172,16,13,10}, {0x42,0x4C,0x45,0x00,0x71,0x10}}, 
                                    {{172,16,13,120}, {0x42,0x4C,0x45,0x00,0x70,0x10}},
                                    {{172,16,13,130}, {0x42,0x4C,0x45,0x00,0x60,0x10}},
                                    {{172,16,13,140}, {0x42,0x4C,0x45,0x00,0x70,0x10}},
                                    {{172,16,13,150}, {0x42,0x4C,0x45,0x00,0x60,0x10}},
                                    {{172,16,13,160}, {0x42,0x4C,0x45,0x00,0x70,0x10}},
                                    {{172,16,13,170}, {0x42,0x4C,0x45,0x00,0x60,0x10}},
                                    {{172,16,13,180}, {0x42,0x4C,0x45,0x00,0x70,0x10}},
                                    {{172,16,13,190}, {0x42,0x4C,0x45,0x00,0x60,0x10}},
                                    {{172,16,13,200}, {0x42,0x4C,0x45,0x00,0x70,0x10}}},3};
    
    audio_txlist.t_des_info[0].ip[3]+=i;
    audio_txlist.t_des_info[0].mac[5]+=i;
    
    if(i == 0)
    {
#if 0
        for(int j=0; j<MAX_SENDCHAN_NUM; j++)
        {
            memcpy(&audio_txlist.t_des_info[j], &audio_txlist.t_des_info[0], sizeof(des_info_t));
        }
        audio_txlist.num_info = MAX_SENDCHAN_NUM;
        audio_txlist.t_des_info[MAX_SENDCHAN_NUM-1].ip[3] = 101;
        audio_txlist.t_des_info[MAX_SENDCHAN_NUM-1].mac[5] = 0xAF;
#endif
        audio_txlist.t_des_info[0].ip[3] = 101;
        audio_txlist.t_des_info[0].mac[5] = 0xAF;
    }
    if_ethaud_cfg.set_audio_desip_infolist(&audio_txlist, i);
#if 0
    uint16_t fn1[100] = {'1','0','.','m','p','3',0};
    else if(i<9)
    {
        fn[0] = '1'+i;
        if_fs.music_start(i,(uint8_t*)fn,100,0);
    }
    else if(i<19)
    {
        fn1[1] = '0'+i-9;
        if_fs.music_start(i,(uint8_t*)fn1,100,0);
    }
    else if(i<29)
    {
        fn1[0] = '2';
        fn1[1] = '0'+i-19;
        if_fs.music_start(i,(uint8_t*)fn1,100,0);
    }    
    else if(i<39)
    {
        fn1[0] = '3';
        fn1[1] = '0'+i-39;
        if_fs.music_start(i,(uint8_t*)fn1,100,0);
    }    
#endif
    if_fs.music_start(i,(uint8_t*)fn,100,0);

    user_audio_senden(i);
    set_audio_vol(i, 40);   

}

static void mp3_test(client ethaud_cfg_if if_ethaud_cfg, client file_server_if if_fs)
{
#define START_CH    0
#define END_CH      64    

    static int i = START_CH, j=0;

    if(j) {
        j = 0;
        return;
    } else {
        j = 1;
    }
    
    if(g_mp3_test_start_flag==0)
    {
        i = START_CH;
        return;
    }
    if(i == END_CH)
    {
        i = START_CH;
        g_mp3_test_start_flag = 0;
        return;
    }
    debug_printf("start_single_mp3_test [%d]\n", i);
    start_single_mp3_test(if_ethaud_cfg, if_fs, i);

    i++;

}
void xtcp_uesr(client xtcp_if i_xtcp,client ethaud_cfg_if if_ethaud_cfg,client fl_manage_if if_fl_manage,client file_server_if if_fs,
                  client uart_tx_buffered_if if_uart_tx,client uart_rx_if if_uart_rx,client image_upgrade_if i_image){
    unsafe{
	//----------------------------------------------
	// wifi模块进入串口AT指令模式
    p_wifi_on <: 0x00; //
    delay_milliseconds(100);
    p_wifi_on <: 0x02; //
    g_sys_val.wifi_mode = 0x02;
    delay_milliseconds(100);
    dhcp_dis();
    //-------------------------------------------------
	unsigned data_len=0;		// set xtcp recive data len
   
	i_user_xtcp = (client xtcp_if * unsafe) &i_xtcp;
	i_user_flash = (client fl_manage_if * unsafe) &if_fl_manage;
	i_ethaud_cfg = (client ethaud_cfg_if * unsafe) &if_ethaud_cfg;
    i_fs_user = (client file_server_if * unsafe) &if_fs;
    i_uart_tx = &if_uart_tx;
   
	//----------------------------------------------------
	// init fun 
    while(if_fl_manage.is_flash_init_complete());
	init_funlist_len();     //系统函数列表初始化
    user_fldat_init();      //初始化flash
	sys_info_init();	    //系统信息 列表 获取及初始化
    ds1302_init();          //ds1302 及日期初始化
    maschine_code_init();   //生成机器码
	//----------------------------------------------------------------------
	// Config xtcp ethernet info
	//----------------------------------------------------------------------
	//协议栈初始化
    xtcp_ipconfig_t ipconfig={0};
    if(!host_info.dhcp_en){
        debug_printf("te %d\n",host_info.div_type[0]);
        debug_printf("st ip,%d,%d,%d,%d\n",host_info.ipconfig.ipaddr[0],host_info.ipconfig.ipaddr[1],host_info.ipconfig.ipaddr[2],host_info.ipconfig.ipaddr[3]);
        debug_printf("mac %x,%x,%x,%x,%x,%x\n",host_info.mac[0],host_info.mac[1],host_info.mac[2],host_info.mac[3],host_info.mac[4],host_info.mac[5]);
        memcpy(&ipconfig,&host_info.ipconfig,12);
    }
    if(mac_factory_init(i_xtcp,host_info.mac)==0){
	    i_xtcp.xtcp_init(ipconfig,host_info.mac);
    }
	//---------------------------------
	// build listening form eth PORT
	i_xtcp.listen(ETH_COMMUN_PORT, XTCP_PROTOCOL_UDP);
    unsafe { 
        pi_fs = &if_fs; 
        pi_image = &i_image;
    }
    tftp_init(i_xtcp);
    //--------------------------------------------------------------
    // timer set
    timer systime, tftptime;
    unsigned time_tmp, time_tftp;
    systime :> time_tmp; 
	tftptime :> time_tftp;
    //
    create_todaytask_list(g_sys_val.time_info); //生成当天任务列表
    delay_milliseconds(100);
    // 显示日期
    user_disp_data();
    delay_milliseconds(100);
    // 显示版本
    user_disp_version();
    delay_milliseconds(100);
    user_dispunti_init();
    delay_milliseconds(100);
    // TEXT
    //audio_moudle_set();
    //task_music_config_play(0,01);
    // 云服务连接
    g_sys_val.could_ip[0] = 39;
    g_sys_val.could_ip[1] = 98;
    g_sys_val.could_ip[2] = 189;
    g_sys_val.could_ip[3] = 224;
    
    //g_sys_val.could_ip[0] = 172;
    //g_sys_val.could_ip[1] = 16;
    //g_sys_val.could_ip[2] = 110;
    //g_sys_val.could_ip[3] = 183;
    //g_sys_val.colud_connect_f = 1;

    //g_sys_val.could_ip[2] = 13;
    //g_sys_val.could_ip[3] = 224;
    #if COULD_TCP_EN
    //g_sys_val.colud_port = i_xtcp.connect(TCP_COULD_PROT,g_sys_val.could_ip,XTCP_PROTOCOL_TCP);
    #endif
    
    // 建立广播连接
    memset(ipconfig.ipaddr,255,4);
    i_xtcp.connect_udp(ETH_COMMUN_PORT,ipconfig.ipaddr,g_sys_val.broadcast_conn);

    // 初始化发送buff指针
    xtcp_tx_buf = all_tx_buf+CLH_HEADEND_BASE;

    //====================================================================================================
	//main loop process 
	//====================================================================================================
	while(1){
		select {
            case if_fs.notify2user():
            {
                static uint8_t last_ch=0;
                file_server_notify_data_t data;
                if_fs.get_notify(data);
                for(int i=0; i<MUSIC_CHANNEL_NUM; i++)
                {
                    if(data.music_status[i].is_new)
                    {
                        last_ch = i;
                        if((data.music_status[i].status != MUSIC_DECODER_START)&&(data.music_status[i].status != MUSIC_DECODER_STOP)){
                            if((data.music_status[i].status==MUSIC_DECODER_ERROR1)||(data.music_status[i].status==MUSIC_DECODER_ERROR2)){
                                g_sys_val.play_error_inc[i]++;
                                debug_printf("error inc %d\n", g_sys_val.play_error_inc[i]); 
                            }
                            if(data.event == 0)
                                task_musicevent_change(i,data.event,data.result);
                        }
                        debug_printf("music_status[%d]:%d %d\n", i, 
                        data.music_status[i].status,g_sys_val.play_error_inc[i]);
#if 0
                        if(data.music_status[i].status == MUSIC_DECODER_FILE_END)
                        {
                            start_single_mp3_test(if_ethaud_cfg, if_fs, i);
                        }
#endif
                    }
                }
                if(data.event == FOE_FUPLOAD &&
                   data.uplaod_reply_type != FOU_REPLY_IDLE)
                {
                    tftp_upload_reply_deal(i_xtcp, data.uplaod_reply_type, data.uplaod_reply_data);
                }
                else
                {
                    debug_printf("sdcard_status:%d event:%d result:%d error_code:%d \n", data.sdcard_status, data.event, data.result, data.error_code);
                    if(data.error_code==4){
                        g_sys_val.play_error_inc[last_ch]++;
                        debug_printf("change er music %d\n",g_sys_val.play_error_inc[last_ch]);
#if 0
                        if_fs.music_stop(0);
                        user_audio_send_dis(0);
#endif
                        task_musicevent_change(last_ch,data.event,data.result);
                    }
                    if(data.music_status[last_ch].status == MUSIC_DECODER_START){
                        g_sys_val.play_error_inc[last_ch] = 0;
                    }
                    if(g_sys_val.sd_state != data.sdcard_status){
                        g_sys_val.sd_state = data.sdcard_status;
                        if(data.sdcard_status)
                        {
                            stop_all_timetask();
                            user_disptask_refresh();
                            g_mp3_test_start_flag = 0;
                        }
                        else
                        {
                            //g_mp3_test_start_flag = 1;
                        }
                            
                    }
                    if(data.result!=255){
                        file_bat_contorl_event(data.error_code);
                        file_contorl_ack_decode(data.error_code);
                    }
                    g_sys_val.play_ok = 1;
                }
                break;
            }
            case i_image.image_upgrade_ready():
            {
                tftp_upgrade_reply_deal(i_xtcp, i_image);
                break;
            }     
            case tftptime when timerafter(time_tftp+500000):> time_tftp:
            {            
                tftp_tmr_poll(i_xtcp, 5);
                break;
            }
        			
			//-----------------------------------------------------------------------------
	  		// Respond to an event from the tcp server
	  		//-----------------------------------------------------------------------------
	 		case i_xtcp.packet_ready():
				i_xtcp.get_packet(conn, all_rx_buf, RX_BUFFER_SIZE, data_len);
                tftp_handle_event(i_xtcp, conn, all_rx_buf, data_len);
                mac_fatctory_xtcp_event(i_xtcp, conn, all_rx_buf, data_len);
                gateway_event(i_xtcp);
                // 网关模式不进入系统
				switch (conn.event){
					case XTCP_IFUP:
						// Show the IP address of the interface
            			i_xtcp.get_ipconfig(ipconfig);
						debug_printf("Link up\n IP Adr: %d,%d,%d,%d\n",ipconfig.ipaddr[0],
																	   ipconfig.ipaddr[1],
																	   ipconfig.ipaddr[2],
																	   ipconfig.ipaddr[3]);    
						g_sys_val.eth_link_state = 1;
                        //audio_moudle_set();
                        user_disp_ip(ipconfig);
                        //i_xtcp.connect(ETH_COMMUN_PORT,a,XTCP_PROTOCOL_UDP);
						break;
		  			case XTCP_IFDOWN:
						g_sys_val.eth_link_state = 0;
						debug_printf("Link down\n");
						break;
		  			case XTCP_NEW_CONNECTION:
						debug_printf("New connection:%x\n",conn.id);
                        debug_printf("New :%d,%d,%d,%d\n",conn.remote_addr[0],conn.remote_addr[1],conn.remote_addr[2],conn.remote_addr[3]);
                        #if COULD_TCP_EN
                        if(conn.protocol==XTCP_PROTOCOL_TCP){
                            debug_printf("\n\n TPC NEW \n could id%d\n",g_sys_val.could_conn.id);
                            if(g_sys_val.could_conn.id==0){
                                g_sys_val.could_conn = conn;
                                register_could_chk();
                            }
                            else{
                                i_xtcp.close(conn);
                            }
                            break;
                        }
                        #endif
                        //------------------------------------------------------------------------------
                        // 建立长连接不关节点
                        if(conn_long_decoder())
                            break;
                        //
                        create_conn_node(&conn);    //新建一个conn节点
                        debug_printf("out\n");
						break;
		 			case XTCP_RECV_DATA:
                        // 判断是否云命令
                        //===================================================================================
                        #if 0
                        debug_printf("recive dat\n\n");
                        for(uint8_t i=0;i<all_rx_buf[CLH_LEN_BASE]+6;i++){
                            debug_printf("%x ",all_rx_buf[i]);
                        }
                        debug_printf("\n\n");
                        debug_printf("dat");
                        #endif
                        // 从服务器收到云命令处理
                        if(((uint32_t *)all_rx_buf)[CLH_TYPE_BASE/4] == COLUD_HEADER_TAG){
                            //获取真实数据
                            #if 0
                            debug_printf("could rec\n");
                            for(uint8_t i=0;i<CLH_HEADEND_BASE;i++){
                                debug_printf("%x ",all_rx_buf[i]);
                            }
                            debug_printf("\n");
                            #endif
                            xtcp_rx_buf = all_rx_buf+CLH_HEADEND_BASE;
                            // 云包头强制置云标志
                            xtcp_rx_buf[POL_COULD_S_BASE] = 1;
                            // 云ID转移
                            memcpy(&xtcp_rx_buf[POL_ID_BASE],&all_rx_buf[CLH_CONTORL_ID_BASE],6);
                            //判断是否透传
                            if(!ip_cmp(&all_rx_buf[CLH_DESIP_BASE],host_info.ipconfig.ipaddr)){
                                //透传数据
                                conn_list_t *unsafe conn_list_tmp;
                                xtcp_ipaddr_t ip_tmp;
                                ip_tmp[0] = all_rx_buf[CLH_DESIP_BASE];
                                ip_tmp[1] = all_rx_buf[CLH_DESIP_BASE+1];
                                ip_tmp[2] = all_rx_buf[CLH_DESIP_BASE+2];
                                ip_tmp[3] = all_rx_buf[CLH_DESIP_BASE+3];
                                conn_list_tmp = get_conn_for_ip(ip_tmp);
                                //
                                if(conn_list_tmp!=null){
                                    debug_printf("forward: %d,%d,%d,%d\n",ip_tmp[0],ip_tmp[1],ip_tmp[2],ip_tmp[3]);
                                    user_sending_len = data_len - CLH_HEADEND_BASE;
                                    memcpy(xtcp_tx_buf,&all_rx_buf[CLH_HEADEND_BASE],user_sending_len);
                                    //重校验
                                    uint16_t sum;
                                    sum = chksum_8bit(0,&xtcp_tx_buf[POL_LEN_BASE],(user_sending_len-6));
                                    xtcp_tx_buf[user_sending_len-4] = sum;
                                    xtcp_tx_buf[user_sending_len-3] = sum>>8;
                                    //
                                    for(uint8_t i=0;i<user_sending_len;i++){
                                        debug_printf("%x ",xtcp_tx_buf[i]);
                                        if(i%20==0&&i!=0){
                                            debug_printf("\n");
                                        }
                                    }
                                    debug_printf("\n");
                                    user_xtcp_send(conn_list_tmp->conn,0);
                                }
                                break;
                            }
                            debug_printf("is could dat\n");
                        }
                        //===================================================================================
                        // 局域网命令处理
                        else{
                            xtcp_rx_buf = all_rx_buf;
                            //是否透传云命令
                            #if COULD_TCP_EN
                            if(xtcp_rx_buf[POL_COULD_S_BASE]){
                                user_sending_len = data_len;
                                memcpy(xtcp_tx_buf,xtcp_rx_buf,user_sending_len);
                                user_xtcp_send(g_sys_val.could_conn,1);
                                break;
                            }
                            #endif
                        }
                        //===================================================================================
                        if(((uint16_t *)xtcp_rx_buf)[POL_COM_BASE/2]!=0xD000&&(conn.remote_addr[3]!=214)){                        
                            debug_printf("rec ip %d,%d,%d,%d %x\n",conn.remote_addr[0],conn.remote_addr[1],conn.remote_addr[2],conn.remote_addr[3],((uint16_t *)xtcp_rx_buf)[POL_COM_BASE/2]);
                        }
                        conn_decoder();
						break;
					case XTCP_RESEND_DATA:	
                        //user_xtcp_send(conn);
						debug_printf("resend_data:\n");
						break;
					case XTCP_SENT_DATA:
                        if(conn.id == g_sys_val.could_conn.id){
                            g_sys_val.could_send_cnt = 0;
                            debug_printf("could send_end\n");
                        }
                        //debug_printf("send id %d\n",conn.id );
						xtcp_sending_decoder();
                        mes_send_decode();
					  	break;
					case XTCP_TIMED_OUT:    //tcp only
    					user_xtcp_connect_tcp(g_sys_val.could_ip);
    					debug_printf("\n\ntime out:%x\n\n",conn.id);
                        break;
					case XTCP_ABORTED:
                        //i_xtcp.close(g_sys_val.could_conn);
                        g_sys_val.could_conn.id = 0;
                        g_sys_val.colud_connect_f = 0;
                        debug_printf("\n\naborted:%x\n\n",conn.id);
                        break;
					case XTCP_CLOSED:
						debug_printf("Closed connection:%x\n",conn.id);
					  	break;
				}
			break;
		//-----------------------------------------------------------------------------
		// other process
		//----------------------------------------------------------------------------- 	
        case systime when timerafter(time_tmp+10000000):> time_tmp:	//10hz process
            uint8_t tmp;
            //--------------------------------------------------
            // 10hz process
            timee10hz_process();
            static uint8_t reset_ethtim=0;
            static uint8_t p_tmp=0;
            if((g_sys_val.eth_link_state==0)||(p_tmp)){
                reset_ethtim++;
                if(reset_ethtim>35){
                    p_eth_reset <: p_tmp; 
                    if((p_tmp)&&(reset_ethtim>40)){
                        reset_ethtim=0;
                    }
                    p_tmp ^=1;
                    debug_printf("canl %d\n",p_tmp);
                }
            }
            //--------------------------------------------------
            //1HZ Process
            static uint8_t time_count=0;
            time_count++;
            g_sys_val.sys_timinc++;
            if(g_sys_val.key_delay)
                g_sys_val.key_delay--;
            if(g_sys_val.key_reselse){
                g_sys_val.key_wait_inc++;
                if(g_sys_val.key_wait_inc>60){
                    unsigned init_string;
                    // 用户信息初始化
                	init_string = 0;
                    user_fl_sector_read(USER_DAT_SECTOR);
                	sys_dat_write((char*)(&init_string),4,FLASH_ADR_INIT);
                    user_fl_sector_write(USER_DAT_SECTOR);
                    //--------------------------------------------------------------
                    //delay_milliseconds(3000);
                    while(!(if_fl_manage.is_flash_write_complete()));    
                    //device_reboot();
                    debug_printf("device_reboot\n");
                }
            } 
            //
            if(time_count>(10-1)){
                time_count=0;
        	    second_process();
                //------------------------------------
                // 系统重启
                if(g_sys_val.reboot_f){
                    g_sys_val.reboot_inc++;
                    if(g_sys_val.reboot_inc>3){
                        while(!(if_fl_manage.is_flash_write_complete()));    
                        //device_reboot();
                        debug_printf("device_reboot\n");
                        g_sys_val.reboot_f = 0;
                    }
                }
                //-------------------------------------
                // 网关超时
                if(g_sys_val.gateway_standy==0){
                    g_sys_val.gateway_time++;
                    if(g_sys_val.gateway_time>5){
                        read_link_up_event();
                        memset(gateway_mac,0xFF,6);
                        audio_moudle_set();
                    }
                }
                //--------------------------------------
                // 批量超时
                if(g_sys_val.file_bat_contorl_s){
                    g_sys_val.file_bat_tim++;
                    if(g_sys_val.file_bat_tim>1000){
                        g_sys_val.file_bat_contorl_s=0;
                    }
                
}
            }
            //---------------------------------------------------
            break;
        //------------------------------------------------------------------------------
        // KEY process
        //------------------------------------------------------------------------------
        case  p_wifi_chk when pinsneq(g_sys_val.key_state) :> g_sys_val.key_state:
            // wifi key
            if((g_sys_val.key_state&0x04)&&(g_sys_val.key_delay == 0)){
                g_sys_val.key_delay = 3;
                g_sys_val.wifi_mode ^= 1;
                p_wifi_on <: g_sys_val.wifi_mode;
            }
            // reset key
            if(((g_sys_val.key_state&0x01)==0)&&(g_sys_val.key_delay == 0)){
                g_sys_val.key_delay = 3;
                g_sys_val.key_reselse = 1;
                g_sys_val.key_wait_inc = 0;
            }
            if((g_sys_val.key_state&0x01)){
                g_sys_val.key_reselse = 0;
            }
            break;
		}// end select
	}
    }//unsafe
}
                  
void gateway_event(client xtcp_if i_xtcp){
    static uint8_t gateresend_inc=0;
    static xtcp_ipconfig_t ipconfig;
    
    if(g_sys_val.gateway_standy)
        return;    
    switch (conn.event){
        case XTCP_IFUP:
            i_xtcp.get_ipconfig(ipconfig);    
            i_xtcp.connect(ETH_COMMUN_PORT,ipconfig.gateway,XTCP_PROTOCOL_UDP);
            break;
        case XTCP_IFDOWN:
            break;
        case XTCP_NEW_CONNECTION:
            //-------------------------------------------------------------------------------
            // 网关获取模式
            if(charncmp(conn.remote_addr,ipconfig.gateway,4)&&(gateway_conn.id==0)){
                gateway_conn = conn;
                user_sending_len =64;
                user_xtcp_send(conn,0);
            }          
            //-------------------------------------------------------------------------------
            break;
        case XTCP_RESEND_DATA:  
            if(gateway_conn.id == conn.id){
                user_sending_len =64;
                user_xtcp_send(conn,0);
                gateresend_inc++;
                if(gateresend_inc>5){
                    read_link_up_event();
                    memset(gateway_mac,0xFF,6);
                    audio_moudle_set();
                }
            }
            break;
        case XTCP_SENT_DATA:
            if(gateway_conn.id == conn.id){
                xtcp_mac_t mac_tmp;
                read_link_up_event();
                i_xtcp.xtcp_arpget(ipconfig.gateway,mac_tmp);
                memcpy(gateway_mac,&mac_tmp,6);
                audio_moudle_set();
            }
            break;
        case XTCP_TIMED_OUT:    //tcp only
        case XTCP_ABORTED:
        case XTCP_CLOSED:
            break;
    }
}

