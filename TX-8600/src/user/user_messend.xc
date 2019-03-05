#include "user_messend.h"

// 添加进网络消息更新队列
uint8_t mes_list_add(xtcp_connection_t conn,uint8_t could_f,uint8_t could_id[]){
    // 检查是否重复添加
    for(uint8_t i=0;i<MAX_ACCOUNT_CONNET;i++){
        if((mes_send_list.messend_conn[i].conn.id == conn.id) && mes_send_list.messend_conn[i].state){
            return 0;
        }
    }
    for(uint8_t i=0;i<MAX_ACCOUNT_CONNET;i++){
        // 找空置账号
        if(mes_send_list.messend_conn[i].state==0){
            mes_send_list.messend_conn[i].state = 1;
            mes_send_list.messend_conn[i].conn = conn;
            mes_send_list.messend_conn[i].could_f = could_f;
            memcpy(mes_send_list.messend_conn[i].could_id,could_id,6);
            return 1;
        }
    }
    return 0;
}

// 超时清除网络消息更新队列 并入conn 清理部分
void mes_list_close(unsigned id){
    for(uint8_t i=0;i<MAX_ACCOUNT_CONNET;i++){
        // 超时清出队列
        if((mes_send_list.messend_conn[i].conn.id == id) && mes_send_list.messend_conn[i].state){
            mes_send_list.messend_conn[i].state = 0;
        }
    }
}

// 消息发送同步
void mes_send_decode(){
    unsafe{
    if(mes_send_list.wrptr-mes_send_list.rpttr!=0){
        for(;mes_send_list.send_inc<MAX_ACCOUNT_CONNET;mes_send_list.send_inc++){
            if(mes_send_list.messend_conn[mes_send_list.send_inc].state!=0){
                //获取备份发送数据发送
                memcpy(xtcp_tx_buf,mes_send_list.tx_buff[mes_send_list.rpttr],mes_send_list.len[mes_send_list.rpttr]);
                user_sending_len = mes_send_list.len[mes_send_list.rpttr];
                debug_printf("messend:%x%x    ",xtcp_tx_buf[POL_COM_BASE+1],xtcp_tx_buf[POL_COM_BASE]);
                debug_printf("ip : %d,%d,%d,%d\n",mes_send_list.messend_conn[mes_send_list.send_inc].conn.remote_addr[0],
                                                  mes_send_list.messend_conn[mes_send_list.send_inc].conn.remote_addr[1],
                                                  mes_send_list.messend_conn[mes_send_list.send_inc].conn.remote_addr[2],
                                                  mes_send_list.messend_conn[mes_send_list.send_inc].conn.remote_addr[3]);
                user_xtcp_send(mes_send_list.messend_conn[mes_send_list.send_inc].conn,mes_send_list.messend_conn[mes_send_list.send_inc].could_f);
                mes_send_list.send_inc++;
                mes_send_list.tim_inc = 0;
                break;
            }
        }
        if(mes_send_list.send_inc >= MAX_ACCOUNT_CONNET){
            mes_send_list.send_inc=0;
            mes_send_list.rpttr++;
            if(mes_send_list.rpttr==mes_send_list.wrptr){
                mes_send_list.rpttr=0;
                mes_send_list.wrptr=0;
            }
        }
    }
    }//unsafe
}   

// 信息列表更新通知
void mes_send_listinfo(uint8_t type,uint8_t need_send){
    if((mes_send_list.wrptr>=MES_STACK_NUM)&&(conn_sending_s.id!=null))
        return;
    //备份发送数据
    mes_send_list.len[mes_send_list.wrptr] = listinfo_upgrade_build(type);
    memcpy(mes_send_list.tx_buff[mes_send_list.wrptr] ,xtcp_tx_buf,mes_send_list.len[mes_send_list.wrptr]);
    mes_send_list.wrptr++;
    //
    if(need_send)
        mes_send_decode();
}

// 账号更新通知
void mes_send_acinfo(uint16_t id){
    if(mes_send_list.wrptr>=MES_STACK_NUM)
        return;
    //备份发送数据
    mes_send_list.len[mes_send_list.wrptr] = acinfo_upgrade_build(id);
    memcpy(mes_send_list.tx_buff[mes_send_list.wrptr] ,xtcp_tx_buf,mes_send_list.len[mes_send_list.wrptr]);
    mes_send_list.wrptr++;
}

// 任务更新通知
void mes_send_taskinfo(){
    if(mes_send_list.wrptr>=MES_STACK_NUM)
        return;
    //备份发送数据
    mes_send_list.len[mes_send_list.wrptr] = taskinfo_upgrade_build(&g_sys_val.tmp_union.task_allinfo_tmp,g_sys_val.task_config_s,g_sys_val.task_con_id);
    memcpy(mes_send_list.tx_buff[mes_send_list.wrptr] ,xtcp_tx_buf,mes_send_list.len[mes_send_list.wrptr]);
    mes_send_list.wrptr++;
}

// 即时任务更新通知
void mes_send_rttaskinfo(uint16_t id){
    if(mes_send_list.wrptr>=MES_STACK_NUM)
        return;
    //
    mes_send_list.len[mes_send_list.wrptr] = rttaskinfo_upgrade_build(id);
    memcpy(mes_send_list.tx_buff[mes_send_list.wrptr] ,xtcp_tx_buf,mes_send_list.len[mes_send_list.wrptr]);
    mes_send_list.wrptr++;
}

// 方案更新通知
void mes_send_suloinfo(uint16_t id){
    if(mes_send_list.wrptr>=MES_STACK_NUM)
        return;
    //
    mes_send_list.len[mes_send_list.wrptr] = sulo_upgrade_build(id);
    memcpy(mes_send_list.tx_buff[mes_send_list.wrptr] ,xtcp_tx_buf,mes_send_list.len[mes_send_list.wrptr]);
    mes_send_list.wrptr++;
}



void mes_send_overtime(){
    if(mes_send_list.tim_inc){
         mes_send_list.tim_inc++;
         if(mes_send_list.tim_inc>8){
             mes_send_decode();
         }
    }
}

