// Copyright (c) 2011-2016, XMOS Ltd, All rights reserved
#include <platform.h>
#include <string.h>
#include <stdlib.h>
#include <debug_print.h>
#include <print.h>
#include "tftp.h"
#include "tftp_app.h"


static enum
{
    TFTP_IDLE,
    TFTP_WAITING_FOR_CONNECTION,
    TFTP_WAITING_FOR_DATA,
    TFTP_SENDING_ACK,
    TFTP_COMPLETE
} tftp_state = TFTP_IDLE;

static unsigned short local_tid = TFTP_SOURCE_TID_SEED;

static unsigned short block_num;
static unsigned short prev_block_num;

unsigned char tftp_tx_buf[TFTP_TX_BUFFER_SIZE];
#define tx_buffer tftp_tx_buf

static int num_tx_bytes;

static xtcp_connection_t tftp_conn;

static int write_mode = 0;
static int signal_error;
static int signal_complete;

static unsigned int tftp_tmr_poll_flag = 0;

void tftp_init(client xtcp_if i_xtcp)
{
    write_mode = 0;
    signal_error = 0;
    signal_complete = 0;

    block_num = 0;
    prev_block_num = 0;

    num_tx_bytes = 0;

    tftp_conn.id = -1;

    i_xtcp.listen(TFTP_DEFAULT_PORT, XTCP_PROTOCOL_UDP);

}

void tftp_close(client xtcp_if i_xtcp)
{
    if(tftp_conn.id != -1) 
    {
        i_xtcp.close(tftp_conn);
    };
    tftp_conn.id = -1;
    
    write_mode = 0;
    signal_error = 0;
    signal_complete = 0;
    block_num = 0;
    prev_block_num = 0;
    num_tx_bytes = 0;

    tftp_tmr_poll_flag = 0;
}




void tftp_handle_event(client xtcp_if i_xtcp, xtcp_connection_t &conn, unsigned char rxbuff[], unsigned int rxlen)
{
    int response_len;

    // If the state is complete, we aren't accepting any new TFTP connections
    // if (tftp_state == TFTP_COMPLETE) return;

    // Check if this connection is TFTP
    if (conn.event != XTCP_IFUP && conn.event != XTCP_IFDOWN &&
        conn.local_port != TFTP_DEFAULT_PORT && conn.local_port != local_tid)
    {
        return;
    }
    
    switch (conn.event)
    {
        case XTCP_IFUP:
        {
#if TFTP_DEBUG_PRINT
            debug_printf("TFTP: IP Up\n");
#endif
            // When the network interface comes up, we are ready to accept a TFTP connection
            tftp_state = TFTP_WAITING_FOR_CONNECTION;

            break;
        }
        case XTCP_IFDOWN:
        {
            // If the interface goes down during a transfer, we should flag an error to the application
            // layer and close the active connection.
            if (tftp_state == TFTP_WAITING_FOR_DATA || tftp_state == TFTP_SENDING_ACK)
            {
#if TFTP_DEBUG_PRINT
                debug_printf("TFTP: IP Down\n");
#endif
                tftp_app_transfer_error();
            }

            tftp_close(i_xtcp);

            tftp_state == TFTP_IDLE;            
            break;
        }
        case XTCP_NEW_CONNECTION:
        {
#if 1||TFTP_DEBUG_PRINT
            debug_printf("TFTP: New connection to listening port:%d tftp_state:%d\n", conn.local_port,tftp_state);
#endif

            if (tftp_state == TFTP_WAITING_FOR_CONNECTION && conn.local_port == TFTP_DEFAULT_PORT)
            {
                // We always reply from a new (random) port
                // local_tid++;
                
                tftp_conn = conn;
                //xtcp_set_poll_interval(c_xtcp, tftp_conn, TFTP_TIMEOUT_SECONDS * 1000);
                tftp_tmr_poll_flag = 1;

                // Bind the local port to the destination TID of the received packet
                // i_xtcp.bind_local_udp(tftp_conn, local_tid);


                // We set the state to indicate that we expect the WRQ packet as the next XTCP_RECV_DATA event
                // - not an actual DATA packet in this instance
                tftp_state = TFTP_WAITING_FOR_DATA;

                prev_block_num = 0;
            }
            else
            {
                //可返回繁忙提示
                i_xtcp.close(conn);
            }
            break;
        }

        case XTCP_RECV_DATA:
        {
            response_len = rxlen>TFTP_RX_BUFFER_SIZE?TFTP_RX_BUFFER_SIZE:rxlen;

            if (tftp_state == TFTP_WAITING_FOR_DATA && conn.id == tftp_conn.id)
            {
                int res = SHORTLY_ACK_SUCCEED;
                int can_put_data = 0;

                num_tx_bytes = tftp_process_packet(tx_buffer, rxbuff, response_len, block_num, write_mode, can_put_data, signal_error, signal_complete);

                if(!signal_error && (rxbuff[1]==TFTP_OPCODE_WRQ || rxbuff[1]==TFTP_OPCODE_RRQ))
                {
                    int tsize = tftp_get_tsize(rxbuff+2);
                    int blksize = tftp_get_blksize(rxbuff+2);
                    
                    if(blksize < 0) blksize = 512;
                    
                    res = tftp_app_transfer_begin(rxbuff+2, (tsize<0)?0:tsize, blksize, write_mode);
                    if(res == SHORTLY_ACK_FAILED)
                    {
                        num_tx_bytes = tftp_process_app_error(tx_buffer, 0);
                        signal_error = 1;
                    }
                }

                
                if (!signal_error && rxbuff[1]==TFTP_OPCODE_DATA)
                {
                    // 判断数据是否能填充数据，出现数据包重发时，会有重复数据
                    if(can_put_data)
                    {
                        res = tftp_app_process_data_block(rxbuff+TFTP_MIN_PKT_SIZE, response_len - TFTP_MIN_PKT_SIZE);
                        if(res == SHORTLY_ACK_FAILED)
                        {
                            num_tx_bytes = tftp_make_error_pkt(tx_buffer, TFTP_ERROR_ACCESS_VIOLATION, "", signal_error);
                        }
                    }
                    else
                    {
                        res = SHORTLY_ACK_SUCCEED;
                    }
                }
                //TFTP read 操作时, 特别处理远端最后一个请求包, 不回复, 并结束连接
                if(signal_complete && write_mode==0)
                {
                    tftp_app_transfer_complete();

                    tftp_close(i_xtcp);

                    tftp_state = TFTP_WAITING_FOR_CONNECTION;
                    break;
                }
                
                if(num_tx_bytes > 0 && res!= DELAYED_ACK)
                {
                    //debug_printf("tftp shortly send %d\n", num_tx_bytes);
                    i_xtcp.send(conn, tx_buffer, num_tx_bytes);
                    tftp_state = TFTP_SENDING_ACK;
                }
                
                if(num_tx_bytes <= 0)
                {
#if TFTP_DEBUG_PRINT
                    debug_printf("TFTP: Received an error\n");
#endif
                    tftp_app_transfer_error();

                    tftp_close(i_xtcp);

                    tftp_state = TFTP_WAITING_FOR_CONNECTION;
                }
            }

            break;
        }
        case XTCP_RESEND_DATA:
        {
            //XTCP_SENT_DATA and XTCP_RESEND_DATA has problem
            if ((tftp_state==TFTP_SENDING_ACK||tftp_state==TFTP_WAITING_FOR_DATA)&& num_tx_bytes > 0)
            {
                i_xtcp.send(conn, tx_buffer, num_tx_bytes);
            }

            break;
        }
        case XTCP_SENT_DATA:
        {
            //XTCP_SENT_DATA and XTCP_RESEND_DATA has problem
            //num_tx_bytes = 0;
            if (signal_error)
            {
#if TFTP_DEBUG_PRINT
                debug_printf("TFTP: Transfer error\n");
#endif
                tftp_app_transfer_error();

                tftp_close(i_xtcp);

                tftp_state = TFTP_WAITING_FOR_CONNECTION;
                break;
            }

            if (signal_complete)
            {
#if TFTP_DEBUG_PRINT
                debug_printf("TFTP: Transfer complete\n");
#endif
                tftp_app_transfer_complete();

                tftp_close(i_xtcp);

                tftp_state = TFTP_WAITING_FOR_CONNECTION;
                
                break;
            }

            tftp_state = TFTP_WAITING_FOR_DATA;

            break;
        }
        case XTCP_TIMED_OUT:
        case XTCP_ABORTED:
        case XTCP_CLOSED:
        {
#if TFTP_DEBUG_PRINT
            debug_printf("TFTP: Closed connection %d\n", conn.id);
#endif
            break;
        }
    }

    if(conn.event != XTCP_IFUP && conn.event != XTCP_IFDOWN)
    {
        conn.event = XTCP_DNS_RESULT+1;
    }

}

static void xtcp_poll(client xtcp_if i_xtcp)
{
    // Handles timeouts
    if (tftp_state == TFTP_WAITING_FOR_DATA && prev_block_num == block_num)
    {
#if 1||TFTP_DEBUG_PRINT
        debug_printf("TFTP: Connection timed out\n");
#endif
        tftp_close(i_xtcp);

        tftp_app_transfer_error();

        tftp_state = TFTP_WAITING_FOR_CONNECTION;
    }
    else
    {
        prev_block_num = block_num;
    }
}


void tftp_tmr_poll(client xtcp_if i_xtcp, int interval_ms)
{
    static unsigned int tick = 0;
    
    tftp_app_timer(interval_ms);
    
    if(tftp_tmr_poll_flag)
    {
        if(tick++ > TFTP_TIMEOUT_SECONDS*(1000/interval_ms))
        {
            tick = 0;
            xtcp_poll(i_xtcp);
        }    
    }
    else
    {
        tick = 0;
    }
}

void tftp_send_ack(client xtcp_if i_xtcp, unsigned char ack_code, unsigned char error_data)
{
    if(tftp_state == TFTP_WAITING_FOR_CONNECTION)
    {
        debug_printf("tftp_send_ack error\n");
        return;
    }
    if(ack_code == TFTP_ACK_SUCCEED)
    {
        i_xtcp.send(tftp_conn, tx_buffer, num_tx_bytes);
        tftp_state = TFTP_SENDING_ACK;
    }
    else if(ack_code == TFTP_ACK_FAILED)
    {
        num_tx_bytes = tftp_process_app_error(tx_buffer, error_data);
        i_xtcp.send(tftp_conn, tx_buffer, num_tx_bytes);
        tftp_state = TFTP_SENDING_ACK;
        signal_error = 1;
    }
}


