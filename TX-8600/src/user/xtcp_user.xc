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
#include "pc_config_tool.h"
#include "could_serve.h"
#include "sys_log.h"
#include "user_log.h"

#include "debug_print.h"
#include "string.h"
#include "stdio.h"

void wifi_contorl_mode();

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
	i_user_flash->flash_sector_read(USER_DAT_SECTOR,g_tmp_union.buff);
	sys_dat_read((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);//主机信息读取
    //
    memcpy(host_info.mac,macadr,6);
	sys_dat_write((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);
    //
    //while(i_user_flash->is_flash_write_complete());
    user_fl_sector_write(USER_DAT_SECTOR);
    //
    xtcp_debug_printf("write: %x,%x,%x,%x,%x,%x\n",macadr[0],macadr[1],macadr[2],macadr[3],macadr[4],macadr[5]);
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
//static uint16_t tftp_upgrade_dev_type[] = {'T','X','-','8','6','0','0',0};

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
        xtcp_debug_printf("tftp_app_transfer_begin %s %d\n", filename);
        
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
                xtcp_debug_printf("start_write_backup\n");
                //start_write_backup
                i_user_flash->start_write_backup();
            }
            else
            {
                tftp_type = TFTP_TYPE_READ_BACKUP;
                xtcp_debug_printf("start_read_backup\n");
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
        //xtcp_debug_printf("tftp_app_process_data_block num:%d\n", num_bytes);
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
                    xtcp_debug_printf("tftp_upgrade_jude_dev_type error\n");
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
    return SHORTLY_ACK_FAILED;
}
int last_block_num = 0;
int tftp_app_process_send_data_block(unsigned char tx_buf[], int block_num, int block_size, int &complete)
{
#define BACKUP_START_ADDRESS    (0)
#define BACKUP_SIZE             (SDRAM_FLASH_SECTOR_MAX_NUM*4096-BACKUP_START_ADDRESS)
    unsafe {

        //xtcp_debug_printf("tftp_app_process_send_data_block #%d %d\n", block_num, block_size);
        i_user_flash->read_backup(BACKUP_START_ADDRESS+(block_num-1)*block_size, block_size, tx_buf);
        
        last_block_num = block_num;
        
        if(block_num*block_size > BACKUP_SIZE) 
        {
            xtcp_debug_printf("tftp_app_process_send_data_block complete\n");
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
        g_sys_val.reboot_f=1;
    }
    else if(tftp_type==TFTP_TYPE_WRITE_BACKUP)
    {
        //i_user_flash->start_write_backup2flash();
    }
    xtcp_debug_printf("tftp_app_transfer_complete %d\n", tftp_type);
    }
}

void tftp_app_transfer_error(void)
{
    xtcp_debug_printf("tftp_app_transfer_error [tftp_block_wait %d] [last_block_num %d]\n",tftp_block_wait,last_block_num);
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
	//MAC 写入
	// 72 4B
	#if 0
    i_user_flash->flash_sector_read(USER_DAT_SECTOR,tmp_union.buff);
    sys_dat_read((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);//主机信息读取
    //
    host_info.mac[0]=0x42;
    host_info.mac[1]=0x4C;
    host_info.mac[2]=0x45;
    host_info.mac[3]=0x00;
    host_info.mac[4]=0x71;
    host_info.mac[5]=0x3B;
    
	sys_dat_write((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);
    user_fl_sector_write(USER_DAT_SECTOR);
    #endif
    // sn
    //------------------------------------------------------------
    i_user_flash->flash_sector_read(USER_DAT_SECTOR,g_tmp_union.buff);
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
        g_tmp_union.account_all_info.account_info=account_info[i];
        account_fl_write(&g_tmp_union.account_all_info,i);
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
    i_user_flash->flash_sector_read(USER_DAT_SECTOR,g_tmp_union.buff);
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
    xtcp_debug_printf("reset flash\n");
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
    for(uint8_t i=0;i<MAX_SEND_LIST_NUM;i++){
	    t_list_connsend[i].conn_state = LIST_SEND_INIT;
    }
}

//--------------------------------------------------------------------------------------
// 系统信息获取与初始化
void sys_info_init(){
    unsafe{
    //-------------------------------------------------------------------------------------------------
	// Get Host info
	i_user_flash->flash_sector_read(USER_DAT_SECTOR,g_tmp_union.buff);
	sys_dat_read((char*)(&host_info),sizeof(host_info_t),FLASH_HOST_INFO);//主机信息读取
	host_info.version[0] = VERSION_H;
    host_info.version[1] = VERSION_L;
	//----------------------------------------------------------------------------------------------------
	i_user_flash->flash_sector_read(SOLUSION_DAT_SECTOR,g_tmp_union.buff);
	sys_dat_read((char*)(&solution_list),sizeof(solution_list_t),FLASH_SOLUSION_LIST); //方案信息读取
    area_fl_read();    //分区表读取
    divlist_fl_read(); //列表读取
    account_list_read();//账户列表读取
    timer_tasklist_read(); //定时任务列表读取
    rt_task_list_read();   //获取即时任务链表
    create_alltask_list(); //建立所有任务
    //建立今日任务
    //
    conn_list_init();   //链表初始化
    sys_gobalval_init(); //全局变量初始化
    }//unsafe
}

//=======================================================================================================
void read_link_up_event(){
    unsafe{
    g_sys_val.gateway_standy=1;
    g_sys_val.gateresend_inc=0;
    g_sys_val.gateway_time=0;
    //
    //if(g_sys_val.could_conn.id==0)
    //    i_user_xtcp->connect(7002,g_sys_val.could_ip,XTCP_PROTOCOL_TCP);
    }
}

//======================================================================================================
void disp_text_conn(client xtcp_if i_xtcp){
    #if 0
    uint8_t div_conn_num=0;
    //
    unsafe{
    conn_list_t *unsafe conn_p = conn_list_head;
    while(conn_p != null){
        div_conn_num++;
        conn_p = conn_p->next_p;
    }
    }
    xtcp_debug_printf("div %d \n",div_conn_num);
    uint8_t tol_num = div_conn_num;
    i_xtcp.xtcp_conn_cmp(tol_num);
    #endif
}

//=======================================================================================================
/*
//--------------------------------------------------------------------------------
		XTCP EVENT USER DECODE PROCESS  
//-------------------------------------------------------------------------------*/

void xtcp_uesr(client xtcp_if i_xtcp,client ethaud_cfg_if if_ethaud_cfg,client fl_manage_if if_fl_manage,client file_server_if if_fs,
                  client uart_tx_buffered_if if_uart_tx,client uart_rx_if if_uart_rx,client image_upgrade_if i_image){
    unsafe{
    //--------------------------------------------------------------------------
    // 接口初始化
	unsigned data_len=0;		// set xtcp recive data len
    //
	i_user_xtcp = (client xtcp_if * unsafe) &i_xtcp;
	i_user_flash = (client fl_manage_if * unsafe) &if_fl_manage;
	i_ethaud_cfg = (client ethaud_cfg_if * unsafe) &if_ethaud_cfg;
    i_fs_user = (client file_server_if * unsafe) &if_fs;
    i_uart_tx = &if_uart_tx;
	//------------------------------------------------------------------------
	memset(&g_sys_val,0x00,sizeof(g_sys_val_t));
    g_sys_val.tx_buff_fifo.size = MAX_TXBUFF_FIFOSIZE;
    g_sys_val.rx_buff_fifo.size = MAX_RXBUFF_FIFOSIZE;
	//------------------------------------------------------------------------
	// wifi模块进入串口AT指令模式 测试
	wifi_ioset(0x00);
	#if 0
	// wifi 关闭 
	p_wifi_io <: 0x03;
    delay_milliseconds(10000);
    while(1){
        p_wifi_io <: 0x01;
        delay_milliseconds(1000);
        p_wifi_io <: 0x03; //
        dhcp_dis();
        delay_milliseconds(1000);
    }
    #endif
	#if 0
	p_wifi_on <: 0x03;
    delay_milliseconds(10000);
    //p_wifi_on <: 0x01; //
    delay_milliseconds(100);
    //p_wifi_on <: 0x03; //
    g_sys_val.wifi_mode = 0x02;
    delay_milliseconds(100);
    dhcp_dis();
    delay_milliseconds(10000);
    dhcp_dis();
    delay_milliseconds(10000);
    dhcp_dis();
    delay_milliseconds(10000);
    dhcp_dis();
    #endif
    //------------------------------------------------------------------------
    // init fun     
    while(if_fl_manage.is_flash_init_complete())delay_milliseconds(50);
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
        xtcp_debug_printf("te %d\n",host_info.div_type[0]);
        xtcp_debug_printf("st ip,%d,%d,%d,%d\n",host_info.ipconfig.ipaddr[0],host_info.ipconfig.ipaddr[1],host_info.ipconfig.ipaddr[2],host_info.ipconfig.ipaddr[3]);
        xtcp_debug_printf("mac %x,%x,%x,%x,%x,%x\n",host_info.mac[0],host_info.mac[1],host_info.mac[2],host_info.mac[3],host_info.mac[4],host_info.mac[5]);
        memcpy(&ipconfig,&host_info.ipconfig,12);
    }
    if(mac_factory_init(i_xtcp,host_info.mac)==0){
        #if 0
        ipconfig.ipaddr[0] = 172;
        ipconfig.ipaddr[1] = 16;
        ipconfig.ipaddr[2] = 13;
        ipconfig.ipaddr[3] = 116;

        ipconfig.gateway[0] = 172;
        ipconfig.gateway[1] = 16;
        ipconfig.gateway[2] = 13;
        ipconfig.gateway[3] = 254;
        #endif
        i_xtcp.xtcp_init(ipconfig,host_info.mac);
    }
	//---------------------------------
	// build listening form eth PORT
	i_xtcp.listen(ETH_COMMUN_PORT, XTCP_PROTOCOL_UDP);
    i_xtcp.listen(LISTEN_BROADCAST_LPPORT, XTCP_PROTOCOL_UDP);
    i_xtcp.listen(PC_CONFIG_TOOL_PORT, XTCP_PROTOCOL_UDP);
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
    // TEX
    //audio_moudle_set();
    //task_music_config_play(0,01);
    // 云服务连接
    g_sys_val.could_ip[0] = 39;
    g_sys_val.could_ip[1] = 98;
    g_sys_val.could_ip[2] = 189;
    g_sys_val.could_ip[3] = 224;
	// DNS 地址
    g_sys_val.dns_ip[0] = 114;
    g_sys_val.dns_ip[1] = 114;
    g_sys_val.dns_ip[2] = 114;
    g_sys_val.dns_ip[3] = 114;
	
    #if 0
    g_sys_val.could_ip[0] = 172;
    g_sys_val.could_ip[1] = 16;
    g_sys_val.could_ip[2] = 110;
    g_sys_val.could_ip[3] = 183;
    #endif
    #if 0
    g_sys_val.could_ip[0] = 172;
    g_sys_val.could_ip[1] = 16;
    g_sys_val.could_ip[2] = 13;
    g_sys_val.could_ip[3] = 224;
    #endif
    
    //g_sys_val.colud_connect_f = 1;

    //g_sys_val.could_ip[2] = 13;
    //g_sys_val.could_ip[3] = 224;
    #if COULD_TCP_EN
    //g_sys_val.colud_port = i_xtcp.connect(TCP_COULD_PROT,g_sys_val.could_ip,XTCP_PROTOCOL_TCP);
    #endif
    
    // 建立广播连接
    memset(ipconfig.ipaddr,255,4);
    i_xtcp.connect_udp(ETH_COMMUN_PORT,ipconfig.ipaddr,g_sys_val.broadcast_conn);
    i_xtcp.bind_local_udp(g_sys_val.broadcast_conn,LISTEN_BROADCAST_LPPORT);

	// 建立dns服务器连接
    i_xtcp.connect_udp(ETH_DNS_PROT,g_sys_val.dns_ip,g_sys_val.dns_conn);

	// 建立自身IP连接
    i_xtcp.connect_udp(ETH_COMMUN_PORT,host_info.ipconfig.ipaddr,g_sys_val.ipchk_conn);

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
                                xtcp_debug_printf("error inc %d\n", g_sys_val.play_error_inc[i]); 
                            }
                            if(data.event == 0)
                                task_musicevent_change(i,data.event,data.result);
                        }
                        xtcp_debug_printf("music_status[%d]:%d %d\n", i, 
                        data.music_status[i].status,g_sys_val.play_error_inc[i]);
                    }
                }
                if(data.event == FOE_FUPLOAD &&
                   data.uplaod_reply_type != FOU_REPLY_IDLE)
                {
                    tftp_upload_reply_deal(i_xtcp, data.uplaod_reply_type, data.uplaod_reply_data);
                }
                else
                {
                    xtcp_debug_printf("sdcard_status:%d event:%d result:%d error_code:%d \n", data.sdcard_status, data.event, data.result, data.error_code);
                    if(data.error_code==4){
                        g_sys_val.play_error_inc[last_ch]++;
                        xtcp_debug_printf("change er music %d\n",g_sys_val.play_error_inc[last_ch]);
                        task_musicevent_change(last_ch,data.event,data.result);
                    }
                    if(data.music_status[last_ch].status == MUSIC_DECODER_START){
                        g_sys_val.play_error_inc[last_ch] = 0;
                    }
                    if(g_sys_val.sd_state != data.sdcard_status){
                        g_sys_val.sd_state = data.sdcard_status;
                        if(data.sdcard_status)
                            stop_all_timetask();
                            user_disptask_refresh();
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

                if(pc_config_handle(i_xtcp,conn,all_rx_buf,data_len)){
                    user_xtcp_ipconfig(host_info.ipconfig);
                    hostinfo_fl_write();
                }
                tftp_handle_event(i_xtcp, conn, all_rx_buf, data_len);
                mac_fatctory_xtcp_event(i_xtcp, conn, all_rx_buf, data_len);
                gateway_event(i_xtcp);
                // 网关模式不进入系统
				switch (conn.event){
					case XTCP_IFUP:
						// Show the IP address of the interface
            			i_xtcp.get_ipconfig(ipconfig);
						xtcp_debug_printf("Link up\n IP Adr: %d,%d,%d,%d\n",ipconfig.ipaddr[0],
																	   ipconfig.ipaddr[1],
																	   ipconfig.ipaddr[2],
																	   ipconfig.ipaddr[3]);    
						g_sys_val.eth_link_state = 1;
                        //audio_moudle_set();
                        user_disp_ip(ipconfig);
                        disp_text_conn(i_xtcp);
						#if COULD_TCP_EN
						g_sys_val.dns_resend_cnt=3;
						dns_couldip_chk_send();
						#endif
                        //i_xtcp.connect(ETH_COMMUN_PORT,a,XTCP_PROTOCOL_UDP);
                        // 重新查找网关
                        g_sys_val.gateway_standy=0;
						break;
		  			case XTCP_IFDOWN:
						g_sys_val.eth_link_state = 0;
						xtcp_debug_printf("Link down\n");
                        disp_text_conn(i_xtcp);
						break;
		  			case XTCP_NEW_CONNECTION:
						xtcp_debug_printf("New connection:%x\n",conn.id);
                        xtcp_debug_printf("New :%d,%d,%d,%d\n",conn.remote_addr[0],conn.remote_addr[1],conn.remote_addr[2],conn.remote_addr[3]);
                        #if COULD_TCP_EN
                        if(conn.protocol==XTCP_PROTOCOL_TCP){
                            xtcp_debug_printf("TPC NEW \n");
                            if(g_sys_val.could_conn.id==0){
                                xtcp_debug_printf("could id%d\n\n",g_sys_val.could_conn.id);
                                g_sys_val.could_conn = conn;
                                disp_couldstate(1);
                                //注册查询
                                register_could_chk();
                                //时间同步
								cld_timesysnc_request();
                                //向云推送账号列表
                                account_list_updat();
                                // 日志更新
                                log_could_online();
                            }
                            else{
                                i_xtcp.close(conn);
                            }
                            break;
                        }
                        #endif
                        //------------------------------------------------------------------------------
                        // 建立长连接不关节点
                        //if(conn_long_decoder())
                        //    break;
                        // 广播端口不建立节点
                        if(conn.local_port != LISTEN_BROADCAST_LPPORT){
                            //新建一个conn节点
                            xtcp_debug_printf("creat conect\n");
                            if(create_conn_node(&conn)==0){
                                xtcp_debug_printf("\nuser conn is full\n");
                            };    
                        }
                        else{
                            if(g_sys_val.brocast_rec_conn.id!=0){
                                i_xtcp.close(conn);
                            }
                            else{
                                g_sys_val.brocast_rec_conn = conn;
                            }
                        }
                        disp_text_conn(i_xtcp);
						break;
		 			case XTCP_RECV_DATA:
                        //===================================================================================
                        #if 0
                        //获取真实数据
                        xtcp_debug_printf("could rec len %d\n",data_len);
                        for(uint16_t i=0;i<data_len;i++){
                            xtcp_debug_printf("%2x ",all_rx_buf[i]);
                            if(i%30==0 && i!=0)
                                xtcp_debug_printf("\n");
                        }
                        xtcp_debug_printf("\nrecive end \n");
                        #endif
                        //==================================================================================
                        // tcp 包处理
                        if(conn.protocol == XTCP_PROTOCOL_TCP){
                            tcp_xtcp_recive_decode(data_len);
                        }
                        //===================================================================================
                        // 局域网命令处理
                        else{
                            udp_xtcp_recive_decode(data_len);
                        }
                        //===================================================================================
						break;
					case XTCP_RESEND_DATA:	
						if(g_sys_val.ipchk_conn.id==conn.id){
							g_sys_val.ipchk_ipconflict_f = 0;
							ip_conflict_disp(g_sys_val.ipchk_ipconflict_f);
						}
                        //user_xtcp_send(conn);
                        xtcp_resend_decode();
						xtcp_debug_printf("resend_data:%x\n",conn.id);
						break;
					case XTCP_SENT_DATA:
						// ip 冲突
						if(g_sys_val.ipchk_conn.id==conn.id){
							g_sys_val.ipchk_ipconflict_f = 1;
							ip_conflict_disp(g_sys_val.ipchk_ipconflict_f);
						}
                        //-------------------------------------------------
                        //#if LIST_TEXT_DEBUG
						#if LIST_TEXT_DEBUG
						if(conn.id==g_sys_val.could_conn.id)
							xtcp_debug_printf("send event tcp\n");
						else
							xtcp_debug_printf("send event udp\n");
						#endif
                        //列表发送
						xtcp_sending_decoder();
						//
                        if(conn.id == g_sys_val.could_conn.id){
                            g_sys_val.could_send_cnt = 0;
                            if(xtcp_sendend_decode()){
                                break;
                            }
                        }
                        //-------------------------------------------------
						// 消息推送
						if(g_sys_val.tcp_sending==0)
	                        mes_send_decode();
					  	break;
					case XTCP_TIMED_OUT:    //tcp only
    					//user_xtcp_connect_tcp(g_sys_val.could_ip);
    					//user_xtcp_close(g_sys_val.could_conn);
                        g_sys_val.could_conn.id = 0;
                        g_sys_val.colud_connect_f = 0;
    					xtcp_debug_printf("\n\ntime out:%x\n\n",conn.id);
                        break;
					case XTCP_ABORTED:
                        //i_xtcp.close(g_sys_val.could_conn);
                        g_sys_val.could_conn.id = 0;
                        g_sys_val.colud_connect_f = 0;
                        xtcp_debug_printf("\n\naborted:%x\n\n",conn.id);
                        break;
					case XTCP_CLOSED:
                        if((conn.protocol==XTCP_PROTOCOL_TCP)&&(g_sys_val.could_conn.id==conn.id)){
                            g_sys_val.could_conn.id = 0;
                            g_sys_val.colud_connect_f=0;
                            //user_xtcp_unlisten(g_sys_val.colud_port);
                        }
						xtcp_debug_printf("Closed connection:%x\n",conn.id);
					  	break;
				}
			break;
		//-----------------------------------------------------------------------------
		// other process
		//----------------------------------------------------------------------------- 	
        case systime when timerafter(time_tmp+10000000):> time_tmp:	//10hz process
            //---------------------------------------------------------
            // 1HZ Process
            static uint8_t time_count=0;
            time_count++;
            if(time_count>(10-1)){
                time_count=0;
				//
				second_process();
				// 广播连接关闭处理
                if(g_sys_val.brocast_rec_conn.id!=0){
                    g_sys_val.brocast_rec_timinc++;
                    if(g_sys_val.brocast_rec_timinc>2){
                        g_sys_val.brocast_rec_timinc=0;
                        i_xtcp.close(g_sys_val.brocast_rec_conn);
                        g_sys_val.brocast_rec_conn.id = 0;
                    }
                }
                //---------------------------------------
                // 测试 打印链接
                #if 0
                static uint8_t text_tim=0;
                text_tim++;
                if(text_tim > 2){
                    text_tim=0;
                    disp_text_conn(i_xtcp);
                }
                #endif
                //-----------------------------------------
                // 系统重启
                if(g_sys_val.reboot_f){
                    g_sys_val.reboot_inc++;
                    if(g_sys_val.reboot_inc>3){
                        while(!(if_fl_manage.is_flash_write_complete()));    
                        device_reboot();
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
                //--------------------------------------
                // IP冲突检测
				
            }
            //--------------------------------------------------
            // 10hz process
            //-------------------------------------------------
            // 系统时间戳
            g_sys_val.sys_timinc++;
            // 系统10hz线程
            timee10hz_process();
            //--------------------------------------------------
            // tcp 包分割处理 1.5秒超时
            if(g_sys_val.tcp_recing_f){
                g_sys_val.tcp_timout++;
                if(g_sys_val.tcp_timout>15){
                    g_sys_val.tcp_recing_f = 0;
                    g_sys_val.tcp_tmp_len = 0;
                }
            }
            
            //--------------------------------------------------------
            // 网络芯片 不正常 定时复位处理
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
                }
            }
            //--------------------------------------------------------
            // 按键处理
            if(g_sys_val.key_delay)
                g_sys_val.key_delay--;
            // 复位按键长按
            g_sys_val.key_wait_inc++;
            //长按6秒
            if(g_sys_val.key_wait_release==KEY_RESET_RELEASE && g_sys_val.key_wait_inc>60){
                unsigned init_string;
                // 用户信息初始化
            	init_string = 0;
                user_fl_sector_read(USER_DAT_SECTOR);
            	sys_dat_write((char*)(&init_string),4,FLASH_ADR_INIT);
                user_fl_sector_write(USER_DAT_SECTOR);
                //--------------------------------------------------------------
                delay_milliseconds(3000);
                while(!(if_fl_manage.is_flash_write_complete()));    
                device_reboot();      
            } 
            // wifi按键长按
            if(g_sys_val.key_wait_release==KEY_WIFI_RELEASE && g_sys_val.wifi_mode==WIFI_DHCPDIS_MODE && g_sys_val.key_wait_inc>30){  //长按3秒
                // DHCP使能
                //xtcp_debug_printf("dhcp en\n");
                g_sys_val.wifi_mode=WIFI_DHCPEN_MODE;
                dhcp_disp_en();
            }
            //---------------------------------------------------
            // wifi 模式控制
            wifi_contorl_mode();
            break;
        //------------------------------------------------------------------------------
        // KEY process
        //------------------------------------------------------------------------------
        case  p_wifi_chk when pinsneq(g_sys_val.key_state) :> g_sys_val.key_state:
            //xtcp_debug_printf("key \n");
            //---------------------------------------------------------------------------------
            if(g_sys_val.sys_timinc<20){
                break;
            }
            //xtcp_debug_printf("key have %d %d\n",g_sys_val.key_delay,g_sys_val.key_state);
            // wifi key 按下
            if((g_sys_val.key_state&0x04) && (g_sys_val.key_delay == 0)){
                // 防抖
                g_sys_val.key_delay = 1;
                // 关闭wifi模块
                if(g_sys_val.wifi_mode!=0){ 
                    g_sys_val.wifi_mode = 0;
                    g_sys_val.wifi_contorl_state = 0;
                    g_sys_val.wifi_io_tmp=0;
                    wifi_ioset(0);
                    user_lan_uart0_tx(&g_sys_val.wifi_io_tmp,0,2);
                    dhcp_disp_none();
                    //xtcp_debug_printf("key wifi off\n");
                }
                else{
                    // 开启wifi模块
                    g_sys_val.key_wait_release = KEY_WIFI_RELEASE;
                    g_sys_val.wifi_contorl_state = WIFI_WAIT_POWERON;
                    g_sys_val.key_wait_inc = 0;
                    g_sys_val.wifi_timer = 0;
                    g_sys_val.wifi_mode = WIFI_DHCPDIS_MODE;
                    g_sys_val.wifi_io_tmp = D_IO_WIFI_POWER|D_IO_WIFI_CONTORL;
                    user_lan_uart0_tx(&g_sys_val.wifi_io_tmp,0,1);
                    wifi_ioset(g_sys_val.wifi_io_tmp);
                    //xtcp_debug_printf("key wifi on\n");
                }
            }
            //-----------------------------------------------------------------------------------
            // reset key 按下
            if(((g_sys_val.key_state&0x01)==0)&&(g_sys_val.key_delay == 0)){
                // 防抖
                g_sys_val.key_delay = 1;
                //
                g_sys_val.key_wait_release = KEY_RESET_RELEASE;
                g_sys_val.key_wait_inc = 0;
            }
            // key 松开 11 = 1011B 松开 
            if((g_sys_val.key_state==11)&&(g_sys_val.key_delay == 0)){
                //xtcp_debug_printf("key rel\n");
                g_sys_val.key_wait_release = 0;
                g_sys_val.key_delay = 1;
                //g_sys_val.key_delay = 0;
            }
            //------------------------------------------------------------------------------------
            break;
		}// end select
	}
    }//unsafe
}

                  
void gateway_event(client xtcp_if i_xtcp){
    //static uint8_t gateresend_inc=0;
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
                g_sys_val.gateresend_inc++;
                if(g_sys_val.gateresend_inc>5){
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

