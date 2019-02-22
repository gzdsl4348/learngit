#include "conn_process.h"
#include "list_contorl.h"
#include "protocol_adrbase.h"
#include "list_instance.h"
#include "user_xccode.h"
#include "user_unti.h"
#include <string.h>
#include "debug_print.h"
//
//===========================================================================
// rece xtcp data  指令接收处理线程
//===========================================================================
void conn_decoder(){
    conn_list_t *conn_list_tmp;
    if(((uint16_t *)xtcp_rx_buf)[0]!=0x55AA)
        return;
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
//==============================================================================
// sending xtcp data  发送完成与数据连发处理线程
//==============================================================================
void xtcp_sending_decoder(){
    //debug_printf("cid %d rcid %d\n",conn_sending_s.id,conn.id);
    if(conn_sending_s.id == conn.id){
        uint8_t i=0;
        uint16_t conn_state = conn_sending_s.conn_state;
        debug_printf("send list\n");
        while(conn_state){
            if((conn_state&01)!=0){
                sending_fun_lis[i].sending_fun();
                return;
            }
            i++;
            conn_state >>=1;
        }
    }
}


//============================================================================
// UDP 长连接初始化
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
            debug_printf("new long connect %d\n",conn_long_list.lconn[i].id);
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

//------------------------------------------------------------------------

//==============================================================================
// 连接超时检测
//==============================================================================
#define CONN_OVERTIME 12
#define CONN_LONGCON_TIME 30

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
            debug_printf("conn timeout %x\n",conn_list_tmp->conn.id);
            user_xtcp_close(conn_list_tmp->conn);
            delete_conn_node(conn_list_tmp->conn.stack_conn);
        }    
        //-------------------------------------------------------
        conn_list_tmp = conn_next_p;
    }
    // 关闭长连接
    for(uint8_t i=0;i<MAX_LONG_CONNET;i++){
        if(conn_long_list.lconn[i].id!=0xFF){
            conn_long_list.lconn[i].tim_inc++;
            if(conn_long_list.lconn[i].tim_inc>CONN_LONGCON_TIME){
                conn_long_list.lconn[i].id = 0xFF;
                debug_printf("close long\n");
                user_xtcp_close(conn_long_list.lconn[i].conn);
            }
        }
    }
    // 列表发送超时
    if(conn_sending_s.id!=null){
        conn_sending_s.conn_sending_tim++;
        if(conn_sending_s.conn_sending_tim>=3){
            debug_printf("reset conn sending\n");
            conn_sending_s.id=null;
        }
    }
}


