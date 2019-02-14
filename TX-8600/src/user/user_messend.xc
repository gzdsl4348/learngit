#include "user_messend.h"
#include "list_instance.h"
#include "user_unti.h"
#include "ack_build.h"
#include "user_xccode.h"
#include "debug_print.h"

// 开启同步消息发送
void mes_send_begin(){
    g_sys_val.messend_state = 1;
    g_sys_val.messend_inc = 0;
}

// 消息发送同步
void mes_send_decode(){
    unsafe{
    if(g_sys_val.messend_state){
        for(;g_sys_val.messend_inc<MAX_LONG_CONNET;g_sys_val.messend_inc++){
            if(conn_long_list.lconn[g_sys_val.messend_inc].id!=0xFF){
                //获取备份发送数据发送
                memcpy(xtcp_tx_buf,g_sys_val.tx_buff,g_sys_val.messend_len);
                user_sending_len = g_sys_val.messend_len;
                debug_printf("messend:%x%x    ",xtcp_tx_buf[POL_COM_BASE+1],xtcp_tx_buf[POL_COM_BASE]);
                debug_printf("ip : %d,%d,%d,%d\n",conn_long_list.lconn[g_sys_val.messend_inc].conn.remote_addr[0],
                                                     conn_long_list.lconn[g_sys_val.messend_inc].conn.remote_addr[1],
                                                     conn_long_list.lconn[g_sys_val.messend_inc].conn.remote_addr[2],
                                                     conn_long_list.lconn[g_sys_val.messend_inc].conn.remote_addr[3]);
                user_xtcp_send(conn_long_list.lconn[g_sys_val.messend_inc].conn,0);
                g_sys_val.messend_inc++;
                g_sys_val.messend_over_time = 0;
                break;
            }
        }
        if(g_sys_val.messend_inc >= MAX_LONG_CONNET){
            debug_printf("to 0\n");
            g_sys_val.messend_state = 0;
        }
    }
    }//unsafe
}   


void mes_send_listinfo(uint8_t type,uint8_t need_send){
    //备份发送数据
    g_sys_val.messend_len = listinfo_upgrade_build(type);
    memcpy(g_sys_val.tx_buff,xtcp_tx_buf,g_sys_val.messend_len);
    //
    mes_send_begin();
    if(need_send){
        for(;g_sys_val.messend_inc<MAX_LONG_CONNET;g_sys_val.messend_inc++){
            if(conn_long_list.lconn[g_sys_val.messend_inc].id!=0xFF){
                user_sending_len = g_sys_val.messend_len;
                user_xtcp_send(conn_long_list.lconn[g_sys_val.messend_inc].conn,0);  
                break;
            }
        }
    }
}

void mes_send_acinfo(uint16_t id){
    //备份发送数据
    g_sys_val.messend_len = acinfo_upgrade_build(id);
    memcpy(g_sys_val.tx_buff,xtcp_tx_buf,g_sys_val.messend_len);
    mes_send_begin();
}

void mes_send_overtime(){
    if(g_sys_val.messend_state){
         g_sys_val.messend_over_time++;
         if(g_sys_val.messend_over_time>5){
             mes_send_decode();
         }
    }
}

