#include "sys_text_serve.h"
#include "user_xccode.h"
#include "ack_build.h"
#include "user_unti.h"
#include "debug_print.h"
#include "sys_log.h"

/*
// 查看收发包数量 C001
void text_get_txpage_recive(){
	user_sending_len = chk_txpage_cnt_build();
	user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}
*/

// 网络打印开启关闭   C002
void eth_debug_contorl_recive(){
    g_sys_val.eth_debug_f=0;
	if(xtcp_rx_buf[POL_DAT_BASE]==0){
		debug_conn_connect((uint8_t *)&xtcp_rx_buf[POL_DAT_BASE+1]);
	}
	else{
		//关闭网络打印
		//xtcp_debug_printf("debug_off\n");
		g_sys_val.eth_debug_f=0;
		debug_conn_colse();
	}
}

