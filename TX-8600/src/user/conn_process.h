#ifndef __CONN_PROCESS_H
#define __CONN_PROCESS_H

#include "stdint.h"

//--------------------------------------
// 指令接收处理
void conn_decoder();
//--------------------------------------
// 发送完成及数据连发处理
void xtcp_sending_decoder();
//--------------------------------------
// 连接超时处理
void conn_overtime_close();
// 长连接建立
uint8_t conn_long_decoder();

void connlong_list_init();

uint8_t user_longconnect_build(uint8_t *ipaddr);


#endif  //__CONN_PROCESS_H

