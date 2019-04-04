#include "user_xccode.h"
#include "debug_print.h"
#include "flash_user.h"
#include "list_instance.h"
#include "file.h"
#include "debug_print.h"
#include "music_play.h"
#include "user_unti.h"
#include "fl_buff_decode.h"
#include "uart.h"
#include "conn_process.h"
#include "kfifo.h"
#include <string.h>

extern client interface xtcp_if  * unsafe i_user_xtcp;
extern client interface fl_manage_if  * unsafe i_user_flash;
extern client interface file_server_if *unsafe i_fs_user;
extern client interface ethaud_cfg_if  * unsafe i_ethaud_cfg;
extern client interface flash_if *unsafe i_core_flash;
extern client interface uart_tx_buffered_if *unsafe i_uart_tx;

extern audio_txlist_t t_audio_txlist;

static uint8_t user_audio_txen[MAX_MUSIC_CH]={0};

#define MUSIC_FNAME_NUM (MUSIC_NAME_NUM+PATCH_NAME_NUM)

uint8_t f_name[MUSIC_FNAME_NUM];


on tile[1]: out port p_wifi_io = XS1_PORT_4B; 
void wifi_ioset(uint8_t io_tmp){
    p_wifi_io <: io_tmp;
}

void user_lan_uart0_tx(uint8_t *data,uint8_t len,uint8_t mode){
    unsafe{
    i_user_flash->uart0_tx(data,len,mode);
    }
}

void user_uart_tx(uint8_t *data,uint8_t len){
    unsafe{
    uint16_t dat_l;
    dat_l = i_uart_tx->get_available_buffer_size();
    //debug_printf("uart l %d \n",dat_l);
    if(dat_l<len)
        return;
    for(uint8_t i=0;i<len;i++){
        
        i_uart_tx->write(data[i]);
    }
    }
}

void stop_all_timetask(){
    unsafe{
    debug_printf("stop all task\n");
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        timetask_now.ch_state[i]=0xFF;
    }
    i_fs_user->music_stop_all();
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        user_audio_txen[i] = 0;
    }
    i_ethaud_cfg->set_audio_txen(user_audio_txen,g_sys_val.tx_timestamp);
    }
}

uint8_t gateway_mac[6];
void audio_moudle_set(){
    unsafe{
    audio_ethinfo_t t_audio_ethinfo;
    // 获取主机MAC
    memcpy(t_audio_ethinfo.macaddress,host_info.mac,6);
    // ipconfig
    memcpy(t_audio_ethinfo.ipaddr,host_info.ipconfig.ipaddr,4);
    memcpy(t_audio_ethinfo.ipgate,host_info.ipconfig.gateway,4);
    memcpy(t_audio_ethinfo.ipmask,host_info.ipconfig.netmask,4);
    // gateway mac
    memcpy(t_audio_ethinfo.ipgate_macaddr,gateway_mac,6);
    i_ethaud_cfg->set_audio_ethinfo(&t_audio_ethinfo);
    // 配置音频类型
    //enum AUDIO_TYPE_E audio_type[NUM_MEDIA_INPUTS]={AUX_P4,AUX_P4,AUX_P4,AUX_P4};
    //i_ethaud_cfg->set_audio_type(audio_type);
    //i_ethaud_cfg->
    // text 
    /*
    t_audio_txlist.num_info =1;
    memset(t_audio_txlist.t_des_info[0].ip,0xFF,4);
    memset(t_audio_txlist.t_des_info[0].mac,0xFF,6);
    i_ethaud_cfg->set_audio_desip_infolist(&t_audio_txlist,0);
    i_ethaud_cfg->set_audio_txen(1);
    */
    }
}

void set_audio_type(enum AUDIO_TYPE_E aux_type[]){
    unsafe{
    i_ethaud_cfg->set_audio_type(aux_type);
    }
}

void set_audio_vol(uint8_t ch,uint8_t vol){
    unsafe{
    static uint8_t audio_val[NUM_MEDIA_INPUTS];
    audio_val[ch] = vol;
	i_ethaud_cfg->set_audio_txvol(audio_val);	//0-100
    }
}

void user_music_play(uint8_t ch){
    unsafe{
        //i_fs_user->music_start(ch,f_name,MUSIC_FNAME_NUM,0);
        i_fs_user->music_start(ch,f_name,MUSIC_FNAME_NUM,0);
    }
}

void user_music_stop(uint8_t ch){
    unsafe{
        i_fs_user->music_stop(ch);
    }
}

void user_audio_desip_set(uint8_t ch){
    unsafe{
        i_ethaud_cfg->set_audio_desip_infolist(&t_audio_txlist,ch);
    }
}

void user_updatip_set(uint8_t mac[],uint8_t ip[]){
    unsafe{
        i_ethaud_cfg->update_audio_desip_info(mac,ip);
    }
}

void user_audio_senden(uint8_t ch){
    user_audio_txen[ch] = 1;
    unsafe{
        g_sys_val.tx_timestamp[ch] = g_sys_val.sys_timinc;
        i_ethaud_cfg->set_audio_txen(user_audio_txen,g_sys_val.tx_timestamp);
        /*
        unsigned max_send_page[NUM_MEDIA_INPUTS];
        unsigned have_send_num[NUM_MEDIA_INPUTS];
        max_send_page[0] = 10000;
        i_ethaud_cfg->send_text_en(user_audio_txen,g_sys_val.tx_timestamp,max_send_page,have_send_num);
        */
    }
}

void user_audio_send_dis(uint8_t ch){
    user_audio_txen[ch] = 0;
    unsafe{
        i_ethaud_cfg->set_audio_txen(user_audio_txen,g_sys_val.tx_timestamp);
    }
}

void user_xtcp_fifo_send(){
    unsafe{
    if(g_sys_val.tcp_sending==0){
        g_sys_val.tcp_sending = 1;
        g_sys_val.tx_fifo_timout = 0;
        i_user_xtcp->send(g_sys_val.could_conn,all_tx_buf,user_sending_len);
    }
    }
}

void user_could_send(uint8_t pol_type){
    unsafe{
    if(g_sys_val.could_conn.id==0)
        return;
     user_sending_len+=CLH_HEADEND_BASE;
     //
    ((uint32_t *)all_tx_buf)[CLH_TYPE_BASE] = COLUD_HEADER_TAG;
    all_tx_buf[CLH_LEN_BASE] = (user_sending_len-6);
    all_tx_buf[CLH_LEN_BASE+1] = (user_sending_len-6)>>8;
    memcpy(&all_tx_buf[CLH_DESIP_BASE],host_info.ipconfig.ipaddr,4);
    memcpy(&all_tx_buf[CLH_HOSTMAC_BASE],host_info.mac,6);
    all_tx_buf[CLH_CONTORL_ID_BASE] = all_tx_buf[CLH_HEADEND_BASE+POL_ID_BASE];
    all_tx_buf[CLH_CONTORL_ID_BASE+1] = all_tx_buf[CLH_HEADEND_BASE+POL_ID_BASE+1];
    all_tx_buf[CLH_CONTORL_ID_BASE+2] = all_tx_buf[CLH_HEADEND_BASE+POL_ID_BASE+2];
    all_tx_buf[CLH_CONTORL_ID_BASE+3] = all_tx_buf[CLH_HEADEND_BASE+POL_ID_BASE+3];
    all_tx_buf[CLH_CONTORL_ID_BASE+4] = all_tx_buf[CLH_HEADEND_BASE+POL_ID_BASE+4];
    all_tx_buf[CLH_CONTORL_ID_BASE+5] = all_tx_buf[CLH_HEADEND_BASE+POL_ID_BASE+5];
    all_tx_buf[CLH_TRANTYPE_BASE] = pol_type;
    all_tx_buf[CLH_DIVTYPE_BASE] = 0;   //主机类型
    //
    #if 0
    for(uint16_t i=0;i<user_sending_len;i++){
        debug_printf("%2x ",all_tx_buf[i]);
        if((i%20==0)&&(i!=0))
            debug_printf("\n");
    }
    debug_printf("\n");
    debug_printf("end\n");
    #endif
    //xtcp_buff_fifo_put(1,all_tx_buf,&g_sys_val.tx_buff_fifo);
    //
    //user_xtcp_fifo_send();
    i_user_xtcp->send(g_sys_val.could_conn,all_tx_buf,user_sending_len);
    }//unsafe
}

void user_xtcp_send(xtcp_connection_t conn,uint8_t colud_f){
	unsafe{
        if(user_sending_len>1472)
            user_sending_len = 1472;
        #if COULD_TCP_EN
        #else
        colud_f = 0;
        #endif 
        if(colud_f){
             //云包头    
            debug_printf("could cmd send %2x%2x\n",xtcp_tx_buf[POL_COM_BASE+1],xtcp_tx_buf[POL_COM_BASE]);
            user_could_send(0);
        }
        else{
            //debug_printf("send dat\n");
            if(conn.id != g_sys_val.could_conn.id)
                i_user_xtcp->send(conn,&all_tx_buf[CLH_HEADEND_BASE],user_sending_len);
        }
	}
}

void user_xtcp_unlisten(unsigned port_number){
	unsafe{
		i_user_xtcp->unlisten(port_number);
	}
}

void user_xtcp_connect_tcp(xtcp_ipaddr_t ipaddr){
    unsafe{
    static int colud_prot;
    user_xtcp_unlisten(colud_prot);
    colud_prot = i_user_xtcp->connect(TCP_COULD_PROT, ipaddr, XTCP_PROTOCOL_TCP);
    //debug_printf("lis %x\n",colud_prot);
    }
}

void user_xtcp_connect(uint8_t ipaddr[]){
	unsafe{
		i_user_xtcp->connect(ETH_COMMUN_PORT,ipaddr,XTCP_PROTOCOL_UDP);
	}
}

void user_xtcp_close(xtcp_connection_t conn){
	unsafe{
        debug_printf("close\n");
		i_user_xtcp->close(conn);
	}
}

void user_udpconn_close(xtcp_connection_t conn){
    unsafe{
        unsafe{
            i_user_xtcp->close_udp(conn);
        }
    }
}

void user_xtcp_ipconfig(xtcp_ipconfig_t ipconfig)
{
	unsafe{
		i_user_xtcp->xtcp_ipconfig(ipconfig);
	}
}

int user_xtcp_connect_udp(unsigned port_number, xtcp_ipaddr_t ipaddr, xtcp_connection_t *new_conn){
    unsafe{
        xtcp_connection_t conn;
        int tmp;
        tmp = i_user_xtcp->connect_udp(port_number,ipaddr,conn);
        memcpy(new_conn,&conn,sizeof(xtcp_connection_t));
        return tmp;
    }
}


void user_fl_sector_read(unsigned sector_num){
	unsafe{
		i_user_flash->flash_sector_read(sector_num,tmp_union.buff);
	}
}

void user_fl_sector_write(unsigned sector_num){
	unsafe{
		i_user_flash->flash_sector_write(sector_num,tmp_union.buff);
	}
}

void user_fl_get_patchlist(uint8_t *buff){
    int len;
    unsafe{
    i_user_flash->read_dirtbl(buff,0,len);
    }
}

void user_fl_get_musiclist(uint8_t index,uint8_t *buff){
    int len;
    unsafe{
    i_user_flash->read_musictbl(index,buff,0,len);
    }
}

int user_file_rename(uint8_t *fsrc,uint8_t *fdes){
    unsafe{
    return i_fs_user->file_rename(fsrc,PATCH_NAME_NUM,fdes,PATCH_NAME_NUM);
    }
}

int user_file_copy(uint8_t *fsrc,unsigned n1,uint8_t *fdes,unsigned n2){
    unsafe{
    return i_fs_user->file_copy(fsrc,n1,fdes,n2,F_COPY_COVER);
    }
}

int user_file_delete(uint8_t *fsrc,unsigned n){
    unsafe{
    return i_fs_user->file_delete(fsrc,n);
    }
}

int user_file_move(uint8_t *fsrc,unsigned n1,uint8_t *fdes,unsigned n2){
    unsafe{
    return i_fs_user->file_move(fsrc,n1,fdes,n2,F_COPY_COVER);
    }
}

int user_file_mir(uint8_t *f_name){
    unsafe{
    return i_fs_user->file_mkdir(f_name, PATCH_NAME_NUM);
    }
}

void user_file_stop(){
    unsafe{
    i_fs_user->fcopy_forced_stop();
    }
}


void user_file_bat_write(uint8_t num,uint8_t *buff){
    unsafe{
    i_user_flash->if_fl_music_tmpbuf_write(num, buff);
    }
}

void user_file_bat_read(uint8_t num,uint8_t *buff){
    unsafe{
    i_user_flash->if_fl_music_tmpbuf_read(num, buff);
    }
}

void user_divsrv_write(uint8_t num,uint8_t *buff){
    unsafe{
    i_user_flash->if_fl_divinfo_tmpbuf_write(num,buff);
    }
}

void user_divsrv_read(uint8_t num,uint8_t *buff){
    unsafe{
    i_user_flash->if_fl_divinfo_tmpbuf_read(num,buff);
    }

}



int user_filefolder_del(uint8_t *f_name){
    unsafe{
    uint8_t tmp[PATCH_NAME_NUM];
    memcpy(tmp,f_name,PATCH_NAME_NUM);
    for(uint8_t i=0;i<PATCH_NAME_NUM/2;i++){
        debug_printf("%x ",((uint16_t *)tmp)[i]);
    }
    debug_printf("end\n");
    
    return i_fs_user->file_delete(tmp,PATCH_NAME_NUM);
    }
}

uint8_t user_file_progress(uint8_t *progress){
    unsafe{
    uint8_t tmp,tmp1;
    tmp = i_fs_user->get_fcopy_progress(tmp1);
    *progress = tmp1;
    return tmp;
    }//unsafe
}

uint8_t start_sysdat_backup(){
    unsafe{
    #if 1
    i_user_flash->start_write_backup2flash();
    return 1;
    #else
    return 1;    
    #endif
    }
}

void backup_system_chk(uint8_t *state,uint8_t *bar){
    unsafe{
        uint32_t total,writed;
        uint8_t complete;
        #if 1
        i_user_flash->get_write_backup2flash_progress(complete,total,writed);
        *state = complete;
        *bar = (writed*100)/total;
        debug_printf("abck bar[%d] wri[%d] com[%d]\n", *bar, writed, complete);
        #else
        debug_printf("backup bar %d\n",*bar);
        if((*bar)>=100){
            *state=0;
        }
        else{
            *bar= (*bar) + 10;
            *state=1;
        }
        #endif
    }
}

void user_xtcp_fifo_get(uint8_t num,uint8_t buff[],uint8_t tx_rx_f){
    unsafe{
        i_user_flash->xtcp_buff_fifo_get(num, buff, tx_rx_f);
    }
}

void user_xtcp_fifo_put(uint8_t num,uint8_t buff[],uint8_t tx_rx_f){
    unsafe{
        i_user_flash->xtcp_buff_fifo_put(num, buff, tx_rx_f);
    }
}


