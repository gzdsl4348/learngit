#include "sys_text_serve.h"
#include "user_xccode.h"
#include "ack_build.h"

// 查看收发包数量 C001
void text_get_txpage_recive(){
	user_sending_len = chk_txpage_cnt_build();
	user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}



