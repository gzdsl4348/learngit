#include "conn_process.h"
#include "list_contorl.h"
#include "protocol_adrbase.h"
#include "list_instance.h"
#include "user_xccode.h"
#include "user_unti.h"
#include <string.h>
#include "kfifo.h"
#include "user_messend.h"
#include "checksum.h"
#include "could_serve.h"
#include "debug_print.h"
#include "sys_log.h"

//

#define DEBUG_LIST_EN 01

//===========================================================================
// rece xtcp data  指令接收处理线程
//===========================================================================
void conn_decoder(){
    conn_list_t *conn_list_tmp;
    #if 1
    if(((uint16_t *)xtcp_rx_buf)[POL_COM_BASE/2]!=0xD000&&
        ((uint16_t *)xtcp_rx_buf)[POL_COM_BASE/2]!=0xB905&&
        ((uint16_t *)xtcp_rx_buf)[POL_COM_BASE/2]!=0xB90C&&
        (conn.remote_addr[3]!=214)){                        
        xtcp_debug_printf("rec ip %d,%d,%d,%d %x ID %x,%x,%x,%x,%x,%x\n",conn.remote_addr[0],conn.remote_addr[1],conn.remote_addr[2],conn.remote_addr[3],((uint16_t *)xtcp_rx_buf)[POL_COM_BASE/2],
					   xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_ID_BASE+1],xtcp_rx_buf[POL_ID_BASE+2],xtcp_rx_buf[POL_ID_BASE+3],xtcp_rx_buf[POL_ID_BASE+4],xtcp_rx_buf[POL_ID_BASE+5]);
    }
    #endif
    //if(((uint16_t *)xtcp_rx_buf)[1]>1420)
    //    return;
    //if((xtcp_rx_buf[POL_LEN_BASE+((uint16_t *)xtcp_rx_buf)[1]]!=0x55)&&(xtcp_rx_buf[POL_LEN_BASE+1+((uint16_t *)xtcp_rx_buf)[1]]!=0xAA))
    //    return;
    for(uint16_t i=0;i<fun_list_len;i++){
        if(((uint16_t *)xtcp_rx_buf)[POL_COM_BASE/2]==rec_fun_lis[i].cmd){
            conn_list_tmp = get_conn_info_p(conn.id);
            if((conn_list_tmp != null)&&(ip_cmp(conn_list_tmp->conn.remote_addr,conn.remote_addr))){
                conn_list_tmp->over_time=0;
            }
            rec_fun_lis[i].cmd_fun();
        }
    }
}

//===========================================================================
// UDP接收处理
//===========================================================================
void udp_xtcp_recive_decode(uint16_t data_len){
    xtcp_rx_buf = all_rx_buf;
    // DNS 处理
    if(conn.id == g_sys_val.dns_conn.id){
        xtcp_debug_printf("rec dns\n");
        dns_domain_recive_decode();
        return;
    }
    //
    //if(xtcp_rx_buf[POL_COM_BASE]==0x06 && xtcp_rx_buf[POL_COM_BASE+1]==0xB4 ){
    //    debug_printf("\n\nB406 vol %d\n\n",xtcp_rx_buf[POL_DAT_BASE+4+6+32]);
    //}
    
    if(((uint16_t *)xtcp_rx_buf)[0]!=0x55AA)
        return;
    //是否透传云命令
    if(xtcp_rx_buf[POL_COULD_S_BASE]){
        #if COULD_TCP_EN
        user_sending_len = data_len;
        memcpy(xtcp_tx_buf,xtcp_rx_buf,user_sending_len);
        user_xtcp_send(g_sys_val.could_conn,1);
        #endif
    }
    else{
        conn_decoder();
    }
}

//===========================================================================
// TCP接收处理
//===========================================================================
void tcp_xtcp_recive_decode(uint16_t data_len){
    xtcp_debug_printf("rec tcp\n");
    //获取真实数据
    #if 0
    xtcp_debug_printf("could rec len %d\n",data_len);
    for(uint16_t i=0;i<data_len;i++){
        xtcp_debug_printf("%2x ",all_rx_buf[i]);
        if(i%30==0 && i!=0)
            xtcp_debug_printf("\n");
    }
    xtcp_debug_printf("\nrecive end \n");
    #endif
    //-------------------------------------------------------------------------------------
    //判断是否收到包头 及接收中处理
    uint16_t poldat_len;   //协议长度
    static uint16_t recing_len_tmp; //已接收字节
    uint16_t remaining_byte;    //接收后剩余字节
    uint16_t len_tmp;
    //-------------------------------------------------------------------------------------
    recice_tcp_decode:
    if((((uint32_t *)all_rx_buf)[CLH_TYPE_BASE/4] == COLUD_HEADER_TAG)&&(g_sys_val.tcp_recing_f==0)){
        poldat_len = all_rx_buf[CLH_LEN_BASE]+(all_rx_buf[CLH_LEN_BASE+1]<<8);
        if(poldat_len < RX_BUFFER_SIZE){
            g_sys_val.tcp_recing_f=1;
            g_sys_val.tcp_timout = 0;
            //是否粘包，协议数据长度是否少于接收包长
            len_tmp = (data_len> poldat_len+6)?(poldat_len+6):data_len;
            g_sys_val.tcp_tmp_len = len_tmp;
            // 已接收字节
            recing_len_tmp = len_tmp;
            // 剩余字节
            remaining_byte = data_len-len_tmp;
            // 复制字节
            memcpy(g_sys_val.tcp_buff_tmp,all_rx_buf,len_tmp);
            //xtcp_debug_printf("\nrec head \n\n");
        }
    }
    // tcp 数据接收中
    else if((g_sys_val.tcp_recing_f)){//&&(g_sys_val.tcp_tmp_len+data_len<=RX_BUFFER_SIZE)){
        g_sys_val.tcp_timout = 0;
        //
        poldat_len = g_sys_val.tcp_buff_tmp[CLH_LEN_BASE]+(g_sys_val.tcp_buff_tmp[CLH_LEN_BASE+1]<<8);
        // 需接收字节
        len_tmp = poldat_len-recing_len_tmp+6;
        len_tmp = (data_len>len_tmp)?len_tmp:data_len;
        
        // 已接收字节
        recing_len_tmp += len_tmp;
        // 剩余字节
        remaining_byte = data_len-len_tmp;

        //xtcp_debug_printf("\n dat %d pol %d ned %d hav %d remain %d\n\n",data_len,poldat_len,len_tmp,recing_len_tmp,remaining_byte);
        // 复制字节
        memcpy(&g_sys_val.tcp_buff_tmp[g_sys_val.tcp_tmp_len],all_rx_buf,len_tmp);
        g_sys_val.tcp_tmp_len += len_tmp;
        #if 0
        for(uint16_t i=0;i<g_sys_val.tcp_tmp_len;i++){
            xtcp_debug_printf("%2x ",g_sys_val.tcp_buff_tmp[i]);
            if(i%30==0 && i!=0)
                xtcp_debug_printf("\n");
        }
        xtcp_debug_printf("\nrecive end \n");
        #endif
    }
    else{
        g_sys_val.tcp_recing_f=0;
        return;
    }
    //-------------------------------------------------------------------------------------
    //判断数据长度与包尾
    if(poldat_len >= RX_BUFFER_SIZE)
        return;
    //xtcp_debug_printf("tcp len %d,dat len %d\n",g_sys_val.tcp_tmp_len,rec_len);
    if((g_sys_val.tcp_tmp_len == poldat_len+6)&&
       (g_sys_val.tcp_buff_tmp[CLH_LEN_BASE+poldat_len]==0x55)&&(g_sys_val.tcp_buff_tmp[CLH_LEN_BASE+1+poldat_len]==0xAA)&&g_sys_val.tcp_recing_f){
        g_sys_val.tcp_decode_f = 1;
        g_sys_val.tcp_recing_f = 0;
    }
    else{
        return;
    }
    //==================================================================================
    // 从服务器收到云命令处理
    if((((uint32_t *)g_sys_val.tcp_buff_tmp)[CLH_TYPE_BASE/4] == COLUD_HEADER_TAG) && g_sys_val.tcp_decode_f){
        //获取真实数据
        #if 0
        xtcp_debug_printf("could rec\n");
        for(uint16_t i=0;i<poldat_len+6;i++){
            xtcp_debug_printf("%2x ",g_sys_val.tcp_buff_tmp[i]);
            if(i%30==0 && i!=0)
                xtcp_debug_printf("\n");
        }
        xtcp_debug_printf("\nrecive end \n");
        
        #endif
        xtcp_rx_buf = g_sys_val.tcp_buff_tmp+CLH_HEADEND_BASE;
        // 云包头强制置云标志
        xtcp_rx_buf[POL_COULD_S_BASE] = 1;
        // 云ID转移
        memcpy(&xtcp_rx_buf[POL_ID_BASE],&g_sys_val.tcp_buff_tmp[CLH_CONTORL_ID_BASE],6);
        //
        //if(xtcp_rx_buf[POL_COM_BASE]==0x05 && xtcp_rx_buf[POL_COM_BASE+1]==0xB4 ){
        //    debug_printf("\n\nB405 vol %d %d\n\n",xtcp_rx_buf[POL_DAT_BASE+4+1],xtcp_rx_buf[POL_DAT_BASE+4+2]);
        //}
        
        //判断是否透传
        if(!ip_cmp(&g_sys_val.tcp_buff_tmp[CLH_DESIP_BASE],host_info.ipconfig.ipaddr)){
            //透传数据
            conn_list_t *conn_list_tmp;
            xtcp_ipaddr_t ip_tmp;
            ip_tmp[0] = g_sys_val.tcp_buff_tmp[CLH_DESIP_BASE];
            ip_tmp[1] = g_sys_val.tcp_buff_tmp[CLH_DESIP_BASE+1];
            ip_tmp[2] = g_sys_val.tcp_buff_tmp[CLH_DESIP_BASE+2];
            ip_tmp[3] = g_sys_val.tcp_buff_tmp[CLH_DESIP_BASE+3];
            conn_list_tmp = get_conn_for_ip(ip_tmp);
            //
            if(conn_list_tmp!=null){
                user_sending_len = data_len - CLH_HEADEND_BASE;
                memcpy(xtcp_tx_buf,&g_sys_val.tcp_buff_tmp[CLH_HEADEND_BASE],user_sending_len);
                //重校验
                uint16_t sum;
                sum = chksum_8bit(0,&xtcp_tx_buf[POL_LEN_BASE],(user_sending_len-6));
                xtcp_tx_buf[user_sending_len-4] = sum;
                xtcp_tx_buf[user_sending_len-3] = sum>>8;
                //
                #if 0
                for(uint8_t i=0;i<user_sending_len;i++){
                    xtcp_debug_printf("%x ",xtcp_tx_buf[i]);
                    if(i%20==0&&i!=0){
                        xtcp_debug_printf("\n");
                    }
                }
                xtcp_debug_printf("\n");
                #endif
                user_xtcp_send(conn_list_tmp->conn,0);
            }
        }
        // 本机指令
        else{
            conn_decoder();
        }
        // 粘包处理
        g_sys_val.tcp_decode_f = 0;
        //
        if(remaining_byte){
            len_tmp = data_len - remaining_byte;
            //判断包头
            unsigned head_tmp = all_rx_buf[len_tmp] | (all_rx_buf[len_tmp+1]<<8) | (all_rx_buf[len_tmp+2]<<16) | (all_rx_buf[len_tmp+3]<<24);
            if(head_tmp==COLUD_HEADER_TAG){
                // 复制字节
                memmove(all_rx_buf,&all_rx_buf[len_tmp],remaining_byte);
                data_len = remaining_byte;
                goto recice_tcp_decode;
            }
        }        
        
    }
}

//==============================================================================
// 查找下一个发送列表
//==============================================================================
void find_next_sendlist(){
    j_listsend_decode:
	g_sys_val.list_sending_f=0;
	if(t_list_connsend[g_sys_val.list_sending_cnt].conn_state == LIST_SEND_END){
		t_list_connsend[g_sys_val.list_sending_cnt].conn_state = LIST_SEND_INIT;
		xtcp_debug_printf("list send end\n");
	}
    // 发送超时清零
    t_list_connsend[g_sys_val.list_sending_cnt].tim_cnt=0;
    // 发送下一个连接的列数据包
    for(uint8_t cnt=0;cnt<MAX_SEND_LIST_NUM;cnt++){
        g_sys_val.list_sending_cnt++;
        if(g_sys_val.list_sending_cnt>=MAX_SEND_LIST_NUM){
            g_sys_val.list_sending_cnt=0;
        }
        //找到下一个有效连接
        if(t_list_connsend[g_sys_val.list_sending_cnt].conn_state==LIST_SEND_END)
            goto j_listsend_decode;
        if(t_list_connsend[g_sys_val.list_sending_cnt].conn_state!=LIST_SEND_INIT){
			g_sys_val.list_sending_f=1;
			xtcp_debug_printf("send next list %d conn id %d lis %d\n",g_sys_val.list_sending_cnt,t_list_connsend[g_sys_val.list_sending_cnt].conn.id,t_list_connsend[g_sys_val.list_sending_cnt].conn_state);
            sending_fun_lis[t_list_connsend[g_sys_val.list_sending_cnt].conn_state].sending_fun(g_sys_val.list_sending_cnt);
            break;
        }
    }   
}

//==============================================================================
// sending xtcp data  发送完成与数据连发处理线程
//==============================================================================
void xtcp_sending_decoder(){
    //-------------------------------------------------------------------------------------------------------------------------
    // 上一个连接的列表数据包发送已完成
    //xtcp_debug_printf("conn %d listcnt %d\n",conn.id,g_sys_val.list_sending_cnt);
    if(t_list_connsend[g_sys_val.list_sending_cnt].conn.id==conn.id && t_list_connsend[g_sys_val.list_sending_cnt].conn_state!=LIST_SEND_INIT){
        xtcp_debug_printf("have list send %d\n",g_sys_val.list_sending_cnt);
        find_next_sendlist();
    }
    //-------------------------------------------------------------------------------------------------------------------------
}

#define LIST_SENDING_OVERTIMECNT  15

// 发送列表超时处理
void list_sending_overtime(){
	if(t_list_connsend[g_sys_val.list_sending_cnt].conn_state!=LIST_SEND_INIT){
	    t_list_connsend[g_sys_val.list_sending_cnt].tim_cnt++;
    	if(t_list_connsend[g_sys_val.list_sending_cnt].tim_cnt>LIST_SENDING_OVERTIMECNT){
           t_list_connsend[g_sys_val.list_sending_cnt].conn_state = LIST_SEND_INIT;
		   g_sys_val.list_sending_f=0;
           #if DEBUG_LIST_EN
		   xtcp_debug_printf("timeover close list sending\n");
           #endif
           // 发送下一个连接的列数据包
           find_next_sendlist();
        }
    }
}

uint8_t list_sending_init(uint16_t cmd,uint8_t list_state,uint8_t could_id[],uint8_t could_s){
    uint8_t i;
    //---------------------------------------------------------------
    // 找是否有重复连接
    for(i=0;i<MAX_SEND_LIST_NUM;i++){
        if(t_list_connsend[i].conn_state!=LIST_SEND_INIT){
            //查是否重复
            if(conn.id==t_list_connsend[i].conn.id && charncmp(t_list_connsend[i].could_id,could_id,6)){
                #if DEBUG_LIST_EN 
				xtcp_debug_printf("list repet conn %d id%x,%x,%x,%x,%x,%x\n",conn.id ,t_list_connsend[i].could_id[0],t_list_connsend[i].could_id[1],t_list_connsend[i].could_id[2],
																t_list_connsend[i].could_id[3],t_list_connsend[i].could_id[4],t_list_connsend[i].could_id[5]);
                #endif
                // 覆盖发送状态
                break;
            }
        }
    }
	// 没有重复连接
	if(i==MAX_SEND_LIST_NUM){
	    // 找空闲的列表发送
	    for(i=0;i<MAX_SEND_LIST_NUM;i++){
	        if(t_list_connsend[i].conn_state==LIST_SEND_INIT){
	            break;
	        }
	    }
	    // 没有空闲的列表发送
	    if(i==MAX_SEND_LIST_NUM){
	        xtcp_debug_printf("list send full\n");
	        return LIST_SEND_INIT;
	    }
	}
    debug_printf("\n\n find num %d \n\n",i);
    //----------------------------------------------------------------
    //初始化发送状态
    t_list_connsend[i].pack_inc=0;
    t_list_connsend[i].pack_tol=0;
    t_list_connsend[i].tim_cnt = 0;
    t_list_connsend[i].conn_state = list_state;
    memcpy(t_list_connsend[i].could_id,could_id,6);
    t_list_connsend[i].could_s = could_s;
    t_list_connsend[i].conn = conn;
    //不同列表发送处理
    switch(list_state){
        // 设备列表
        case DIV_LIST_SENDING:
            // 计算总包数
            t_list_connsend[i].pack_tol = div_list.div_tol/DIV_SEND_NUM;
            if(div_list.div_tol%DIV_SEND_NUM || t_list_connsend[i].pack_tol==0){
                t_list_connsend[i].pack_tol++;
            }
            //初始化设备列表发送信息
            t_list_connsend[i].list_info.divlist.div_list_p = div_list.div_head_p;
            t_list_connsend[i].list_info.divlist.cmd = cmd;
            break;
        //---------------------------------------------------
        // 分区列表
        case AREA_LIST_SENDING:{
            // 获取分区总数
            uint8_t area_tmp=0;
            for(uint8_t i=0;i<MAX_AREA_NUM;i++){
                if(area_info[i].area_sn!=0xFFFF)
                    area_tmp++;
            }
            // 计算总包数
            xtcp_debug_printf("area_tol %d\n",area_tmp);
            t_list_connsend[i].pack_tol = area_tmp/AREA_SEND_NUM;
            if(area_tmp%AREA_SEND_NUM || t_list_connsend[i].pack_tol==0){
                t_list_connsend[i].pack_tol++;
            }
            //初始化分区列表发送信息
            t_list_connsend[i].list_info.arealist.cmd = cmd;
            t_list_connsend[i].list_info.arealist.area_inc = 0;
            break;
            }
        //---------------------------------------------------
        // 任务列表
        case TASK_LIST_SENDING:
            t_list_connsend[i].list_info.tasklist.task_p = timetask_list.all_timetask_head;
            t_list_connsend[i].list_info.tasklist.cmd = cmd;
            //
            break;
        // 任务详细信息列表
        case TASK_DTINFO_SENDING:
            t_list_connsend[i].list_info.task_dtinfo.music_inc = 0;
            t_list_connsend[i].list_info.task_dtinfo.task_id = xtcp_rx_buf[TASK_DTG_TASK_ID+1]<<8 |xtcp_rx_buf[TASK_DTG_TASK_ID];
            break;
        // 即时任务列表
        case RTTASK_LIST_SENDING:
            t_list_connsend[i].list_info.rttasklist.rttask_p = rttask_lsit.all_head_p;
            break;
        // 文件夹列表
        case PATCH_LIST_SENDING:
            t_list_connsend[i].list_info.patchlist.patch_inc=0;
            break;
        // 音乐文件名列表    
        case MUSICNAME_LIST_SENDING:
            memcpy(t_list_connsend[i].list_info.musiclist.music_patch_name,&xtcp_rx_buf[MUS_LIBHCK_CHKPATCH_NAME],PATCH_NAME_NUM);
            break;
        // 账号列表
        case AC_LIST_SENDING:
            t_list_connsend[i].list_info.ac_list.account_inc = 0;
            //------------------------------------------------
            // 获取总包数
            uint8_t total_user=0;
            for(uint8_t i=0;i<MAX_ACCOUNT_NUM;i++){
                if(account_info[i].id!=0xFF){
                    total_user++;
                }
            }
            t_list_connsend[i].pack_tol = total_user/MAX_SEND_ACCOUNT_NUM_FORPACK;
            if(total_user%MAX_SEND_ACCOUNT_NUM_FORPACK || t_list_connsend[i].pack_tol==0)
                t_list_connsend[i].pack_tol++;
            break;
        // 设备搜索列表
        case DIVSRC_LIST_SENDING:
            break;
    }
	if(g_sys_val.list_sending_f==0)
		g_sys_val.list_sending_cnt = i;
    return i;
}

//============================================================================
// UDP 长连接初始化
#if 0
void connlong_list_init(){
    for(uint8_t i=0;i<MAX_LONG_CONNET;i++){
        conn_long_list.lconn[i].id = 0xFF;
    }
}
// UDP 长连接开启
uint8_t user_longconnect_build(uint8_t *ipaddr){
    for(uint8_t i=0;i<MAX_LONG_CONNET;i++){
        if(conn_long_list.lconn[i].id!=0xFF){
            if(charncmp(conn_long_list.lconn[i].conn.remote_addr,conn.remote_addr,4))
                return 0;
        }
    }
    memcpy(g_sys_val.connect_ip,ipaddr,4);
    g_sys_val.connect_build_f = 1;
    return 1;
}
// UDP 长连接节点建立
uint8_t conn_long_decoder(){
    if(g_sys_val.connect_build_f==0)
        return 0;
    if(charncmp(g_sys_val.connect_ip,conn.remote_addr,4)){
        g_sys_val.connect_build_f = 0;
        for(uint8_t i=0;i<MAX_LONG_CONNET;i++){
            xtcp_debug_printf("new long connect %d\n",conn_long_list.lconn[i].id);
            if(conn_long_list.lconn[i].id==0xFF){
                conn_long_list.lconn[i].conn = conn;
                conn_long_list.lconn[i].id = i;
                conn_long_list.lconn[i].tim_inc =0;
                return 1;
            }
        }
    }
    return 0;
}
#endif
//------------------------------------------------------------------------
//每分钟发送广播包
void broadcast_for_minute(){
    static uint8_t time_cnt=0;
    time_cnt++;
    if(time_cnt>60){
        time_cnt = 0;
        memset(xtcp_tx_buf,0,64);
        user_sending_len = 64;
        user_xtcp_send(g_sys_val.broadcast_conn,0);
    }
}

//------------------------------------------------------------------------
//每15秒检测IP冲突
void ipconflict_for_15s(){
    g_sys_val.ipchk_timecnt++;
    if(g_sys_val.ipchk_timecnt>15){
        user_arp_clear(g_sys_val.ipchk_conn.remote_addr);
		g_sys_val.ipchk_timecnt=0;
        memset(xtcp_tx_buf,0,64);
        user_sending_len = 64;
        user_xtcp_send(g_sys_val.ipchk_conn,0);
	}
}

//==============================================================================
// 连接超时检测
//==============================================================================
#define CONN_OVERTIME 10

void conn_overtime_close(){
    // 关闭连接 conn
    conn_list_tmp = conn_list_head;   
    conn_list_t *conn_next_p = conn_list_head;
    while(conn_list_tmp!=null){
        conn_list_tmp->over_time++;
        conn_next_p = conn_list_tmp->next_p;
        //-------------------------------------------------------
        //连接超时，删除节点，关闭连接
        if(conn_list_tmp->over_time>CONN_OVERTIME){
            if((conn_list_tmp->conn.id != g_sys_val.debug_conn.id) || (g_sys_val.eth_debug_f==0)){  
                xtcp_debug_printf("conn timeout %x\n",conn_list_tmp->conn.id);
                mes_list_close(conn_list_tmp->conn.id);               
                user_xtcp_close(conn_list_tmp->conn);
                delete_conn_node(conn_list_tmp->conn.stack_conn);
            }
        }    
        //-------------------------------------------------------
        conn_list_tmp = conn_next_p;
    }
    // 关闭超时列表更新
    for(uint8_t i=0;i<MAX_ACCOUNT_CONNET;i++){
      if(mes_send_list.messend_conn[i].state){
        mes_send_list.messend_conn[i].over_timeinc++;
        if(mes_send_list.messend_conn[i].over_timeinc>CONN_OVERTIME){
            mes_send_list.messend_conn[i].over_timeinc=0;
            mes_send_list.messend_conn[i].state = 0;
        }
    }
    }
}
#if 1
void xtcp_buff_fifo_put(uint8_t tx_rx_f,uint8_t *buff,xtcp_fifo_t *kf){
    #if 1
    unsigned len = MIN(1, kf->size-kf->in_index+kf->out_index);
    unsigned modtmp = (kf->in_index & (kf->size - 1));
    unsigned l = MIN(len, kf->size - modtmp);
    if(l){
        user_xtcp_fifo_put(modtmp,buff,tx_rx_f);
        g_sys_val.tx_fifo_len[modtmp] = user_sending_len;
    }
    if(len-l){
        user_xtcp_fifo_put(0,buff,tx_rx_f);
        g_sys_val.tx_fifo_len[0] = user_sending_len;
    }
    kf->in_index+=len;
    #endif
}

void xtcp_buff_fifo_get(uint8_t tx_rx_f,uint8_t *buff,xtcp_fifo_t *kf,uint8_t clear_f){
    unsigned len = MIN(1, kf->in_index-kf->out_index);
    unsigned modtmp = (kf->out_index & (kf->size - 1));
    unsigned l = MIN(len, kf->size - modtmp);
    if(l){
        user_xtcp_fifo_get(modtmp,buff,tx_rx_f);
        user_sending_len = g_sys_val.tx_fifo_len[modtmp];
    }
    if(len-l){
        user_xtcp_fifo_get(0,buff,tx_rx_f);
        user_sending_len = g_sys_val.tx_fifo_len[0];
    }
    if(clear_f)
        kf->out_index+=len;
}

void xtcp_tx_fifo_put(){
    xtcp_buff_fifo_put(1,all_tx_buf,&g_sys_val.tx_buff_fifo);
}

void xtcp_rx_fifo_put(){
    xtcp_buff_fifo_put(0,all_rx_buf,&g_sys_val.rx_buff_fifo);
}

void xtcp_tx_fifo_get(){
    xtcp_buff_fifo_get(1,all_tx_buf,&g_sys_val.tx_buff_fifo,0);
}

void xtcp_rx_fifo_get(){
    xtcp_buff_fifo_get(0,all_tx_buf,&g_sys_val.tx_buff_fifo,1);
}

uint8_t xtcp_check_fifobuff(xtcp_fifo_t *kf){
    if(kf->in_index-kf->out_index)
        return 1;
    else
        return 0;
}

void xtcp_fifobuff_throw(xtcp_fifo_t *kf){
    unsigned len = MIN(1, kf->in_index-kf->out_index);
    unsigned l = MIN(len, kf->size - (kf->out_index & (kf->size - 1)));
    kf->out_index+=len;
}

void xtcp_bufftimeout_check_10hz(){
    g_sys_val.tx_fifo_timout++;
    if(g_sys_val.tx_fifo_timout>15){
        g_sys_val.tx_fifo_timout=0;
        if(xtcp_check_fifobuff(&g_sys_val.tx_buff_fifo)){
            if(g_sys_val.tcp_sending){
                g_sys_val.tcp_sending = 0;
                xtcp_fifobuff_throw(&g_sys_val.tx_buff_fifo);   
            }
            if(xtcp_check_fifobuff(&g_sys_val.tx_buff_fifo))
                user_xtcp_fifo_send();
        }   
    }
}

uint8_t xtcp_sendend_decode(){
    if(g_sys_val.tcp_sending){
        g_sys_val.tcp_sending = 0;
        g_sys_val.tcp_resend_cnt =0;
        g_sys_val.tx_fifo_timout=0;
        xtcp_fifobuff_throw(&g_sys_val.tx_buff_fifo);
    }
    xtcp_debug_printf("could send_end\n");
    if(xtcp_check_fifobuff(&g_sys_val.tx_buff_fifo)){
        user_xtcp_fifo_send(); 
        return 1;
    }
    return 0;
}

void xtcp_resend_decode(){
    g_sys_val.tcp_resend_cnt++;
    g_sys_val.tcp_sending = 0;
    g_sys_val.tx_fifo_timout=0;
    if(g_sys_val.tcp_resend_cnt>3){
        g_sys_val.tcp_resend_cnt =0;
        xtcp_fifobuff_throw(&g_sys_val.tx_buff_fifo);
    }
    if(xtcp_check_fifobuff(&g_sys_val.tx_buff_fifo)){
        user_xtcp_fifo_send();
    }   
}

void user_xtcp_fifo_send(){
    if(g_sys_val.tcp_sending==0){
        xtcp_debug_printf("send fifo tcp\n");
        g_sys_val.tcp_sending = 1;
        g_sys_val.tx_fifo_timout = 0;
        xtcp_tx_fifo_get();
        user_xtcp_send_could();
    }
}

void user_xtcp_sendfifo_init(){
    g_sys_val.tx_buff_fifo.in_index  = 0;
    g_sys_val.tx_buff_fifo.out_index = 0;
    g_sys_val.tcp_resend_cnt = 0;
    g_sys_val.tcp_sending = 0;
}

void user_xtcp_rxfifo_decode10hz(){
    ;
}

#endif
