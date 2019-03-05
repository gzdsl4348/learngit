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

#endif  //__CONN_PROCESS_H

