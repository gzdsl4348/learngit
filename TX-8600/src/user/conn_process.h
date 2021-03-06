#ifndef __CONN_PROCESS_H
#define __CONN_PROCESS_H

#include "stdint.h"
#include "user_unti.h"

//--------------------------------------
// 指令接收处理
void conn_decoder();

void udp_xtcp_recive_decode(uint16_t data_len);

void tcp_xtcp_recive_decode(uint16_t data_len);

//--------------------------------------
// 发送完成及数据连发处理
void xtcp_sending_decoder();
// 列表发送初始化
uint8_t list_sending_init(uint16_t cmd,uint8_t list_state,uint8_t could_id[],uint8_t could_s);
// 列表发送超时管理
void list_sending_overtime(); //10hz
//--------------------------------------
// 连接超时处理
void conn_overtime_close();

void xtcp_buff_fifo_put(uint8_t tx_rx_f,uint8_t *buff,xtcp_fifo_t *kf);

void xtcp_buff_fifo_get(uint8_t tx_rx_f,uint8_t *buff,xtcp_fifo_t *kf,uint8_t clear_f);

uint8_t xtcp_check_fifobuff(xtcp_fifo_t *kf);

void xtcp_fifobuff_throw(xtcp_fifo_t *kf);

void xtcp_bufftimeout_check_10hz();

uint8_t xtcp_sendend_decode();

void user_xtcp_fifo_send();

void xtcp_tx_fifo_put();

void xtcp_rx_fifo_put();

void xtcp_tx_fifo_get();

void xtcp_rx_fifo_get();

void xtcp_resend_decode();

void user_xtcp_sendfifo_init();

void broadcast_for_minute();

#endif  //__CONN_PROCESS_H

