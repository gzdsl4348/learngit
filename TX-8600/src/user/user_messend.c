#include "user_messend.h"
#include "user_xccode.h"
#include "fl_buff_decode.h"
#include "sys_log.h"

// 添加进网络消息更新队列
// account_f=0 最低优先级处理
// account_f=1 账号登录添加处理
// account_f=2 话筒账号登录添加处理
// return 0 重复添加  1 正常添加  2 已满，无法添加
uint8_t mes_list_add(xtcp_connection_t conn,uint8_t could_f,uint8_t could_id[],uint8_t account_f){
    uint8_t account_cnt=0;
    //xtcp_debug_printf("mes in\n");
    // 检查是否重复添加
    account_cnt=0;
    for(uint8_t i=0;i<MAX_ACCOUNT_CONNET;i++){
        // 云
		if(could_f && mes_send_list.messend_conn[i].state && charncmp(could_id,mes_send_list.messend_conn[i].could_id,6)){
            mes_send_list.messend_conn[i].over_timeinc=0;
            //
            if(account_f==1){
                for(uint8_t j=0;j<MAX_ACCOUNT_CONNET;j++){
                    if(mes_send_list.messend_conn[j].state && mes_send_list.messend_conn[j].account_f==1 && i!=j){
                        account_cnt++;
                    }
                }
                //xtcp_debug_printf("find ac %d\n",account_cnt);
                if(account_cnt>=MAX_ENTER_ACCOUNT){
                    return 2;
                }
                mes_send_list.messend_conn[i].account_f=account_f;
            }
            else if(account_f==2){
                mes_send_list.messend_conn[i].account_f=account_f;
            }
            return 0;
        }
        // 局域网
        else if(could_f==0 && (mes_send_list.messend_conn[i].conn.id == conn.id) && mes_send_list.messend_conn[i].state){
            mes_send_list.messend_conn[i].over_timeinc=0;
            //
            
            //xtcp_debug_printf("eth chk %d\n",account_f);
            if(account_f==1){
                for(uint8_t j=0;j<MAX_ACCOUNT_CONNET;j++){
                    if(mes_send_list.messend_conn[j].state && mes_send_list.messend_conn[j].account_f==1 && i!=j){
                        account_cnt++;
                        //xtcp_debug_printf("eth  %d %d\n",i,j);
                    }
                }
                //xtcp_debug_printf("find ac %d\n",account_cnt);
                if(account_cnt>=MAX_ENTER_ACCOUNT){
                    return 2;
                }
                mes_send_list.messend_conn[i].account_f=account_f;
            }
            else if(account_f==2){
                mes_send_list.messend_conn[i].account_f=account_f;
            }
            return 0;

        }
    }    
    // 找空置账号
    account_cnt=0;
    if(account_f==1){ // 判断最大登录用户数
        for(uint8_t i=0;i<MAX_ACCOUNT_CONNET;i++){
            if(mes_send_list.messend_conn[i].state && mes_send_list.messend_conn[i].account_f==1){
                account_cnt++;
            }
        }
        if(account_cnt>=MAX_ENTER_ACCOUNT){
            return 2;
        }
    }
    // 从空位置中找
    for(uint8_t i=0;i<MAX_ACCOUNT_CONNET;i++){
        if(mes_send_list.messend_conn[i].state==0){
            mes_send_list.messend_conn[i].state = 1;
            mes_send_list.messend_conn[i].conn = conn;
            mes_send_list.messend_conn[i].could_f = could_f;
            memcpy(mes_send_list.messend_conn[i].could_id,could_id,6);
            mes_send_list.messend_conn[i].over_timeinc=0;
            if(account_f){
                mes_send_list.messend_conn[i].account_f=account_f;
            }
            return 1;
        }
    }
    // 从非登录账号中找
    if(account_f){
        for(uint8_t i=0;i<MAX_ACCOUNT_CONNET;i++){
            if(mes_send_list.messend_conn[i].account_f==0){
                mes_send_list.messend_conn[i].state = 1;
                mes_send_list.messend_conn[i].conn = conn;
                mes_send_list.messend_conn[i].could_f = could_f;
                memcpy(mes_send_list.messend_conn[i].could_id,could_id,6);
                mes_send_list.messend_conn[i].over_timeinc=0;
                if(account_f){
                    mes_send_list.messend_conn[i].account_f=account_f;
                }
                return 1;
            }
        }
    }
    return 2;
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
    if(mes_send_list.wrptr-mes_send_list.rpttr!=0){
        for(;mes_send_list.send_inc<MAX_MESSAGE_SEND;mes_send_list.send_inc++){
            if(mes_send_list.messend_conn[mes_send_list.send_inc].state!=0){
                //获取备份发送数据发送
                //memcpy(xtcp_tx_buf,mes_send_list.tx_buff[mes_send_list.rpttr],mes_send_list.len[mes_send_list.rpttr]);
				user_messend_buff_get(mes_send_list.rpttr,xtcp_tx_buf);		
				user_sending_len = mes_send_list.len[mes_send_list.rpttr];
				build_endpage_forid(user_sending_len,mes_send_list.messend_conn[mes_send_list.send_inc].could_id);
				//
				/*
                xtcp_debug_printf("\n\nmessend:%x%x    ",xtcp_tx_buf[POL_COM_BASE+1],xtcp_tx_buf[POL_COM_BASE]);
                xtcp_debug_printf("ip : %d,%d,%d,%d\n",mes_send_list.messend_conn[mes_send_list.send_inc].conn.remote_addr[0],
                                                  mes_send_list.messend_conn[mes_send_list.send_inc].conn.remote_addr[1],
                                                  mes_send_list.messend_conn[mes_send_list.send_inc].conn.remote_addr[2],
                                                  mes_send_list.messend_conn[mes_send_list.send_inc].conn.remote_addr[3]);
                xtcp_debug_printf("mac : %x,%x,%x,%x,%x,%x\n",mes_send_list.messend_conn[mes_send_list.send_inc].could_id[0],
                                                            mes_send_list.messend_conn[mes_send_list.send_inc].could_id[1],
                                                            mes_send_list.messend_conn[mes_send_list.send_inc].could_id[2],
                                                            mes_send_list.messend_conn[mes_send_list.send_inc].could_id[3],
                                                            mes_send_list.messend_conn[mes_send_list.send_inc].could_id[4],
                                                            mes_send_list.messend_conn[mes_send_list.send_inc].could_id[5]);
                */
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
}   

// 任务页面更新通知
void taskview_page_messend(){
    if(mes_send_list.wrptr>=MES_STACK_NUM)
        return;
	mes_send_list.len[mes_send_list.wrptr] = taskview_page_build(TASK_PAGESHOW_B312_CMD);
	user_messend_buff_put(mes_send_list.wrptr,xtcp_tx_buf);
	mes_send_list.wrptr++;
}

// 信息列表更新通知
void mes_send_listinfo(uint8_t type,uint8_t need_send){
    if(mes_send_list.wrptr>=MES_STACK_NUM)
        return;
    for(uint8_t i=0;i<MAX_SEND_LIST_NUM;i++){
        if(t_list_connsend[i].conn_state!=LIST_SEND_INIT)
            return;
    }
    //备份发送数据
    mes_send_list.len[mes_send_list.wrptr] = listinfo_upgrade_build(type);
    //memcpy(mes_send_list.tx_buff[mes_send_list.wrptr] ,xtcp_tx_buf,mes_send_list.len[mes_send_list.wrptr]);
	user_messend_buff_put(mes_send_list.wrptr,xtcp_tx_buf);
	mes_send_list.wrptr++;
    
    //xtcp_debug_printf("iin\n ");

	taskview_page_messend();
    
    //xtcp_debug_printf("oout \n");
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
    //memcpy(mes_send_list.tx_buff[mes_send_list.wrptr] ,xtcp_tx_buf,mes_send_list.len[mes_send_list.wrptr]);
    user_messend_buff_put(mes_send_list.wrptr,xtcp_tx_buf);
    mes_send_list.wrptr++;
}

// 任务更新通知
void mes_send_taskinfo(task_allinfo_tmp_t* task_all_info){
    if(mes_send_list.wrptr>=MES_STACK_NUM)
        return;
    //备份发送数据
    timer_task_read(&g_tmp_union.task_allinfo_tmp,g_sys_val.task_con_id);
    mes_send_list.len[mes_send_list.wrptr] = taskinfo_upgrade_build(task_all_info,g_sys_val.task_config_s,g_sys_val.task_con_id);
    //memcpy(mes_send_list.tx_buff[mes_send_list.wrptr] ,xtcp_tx_buf,mes_send_list.len[mes_send_list.wrptr]);
	user_messend_buff_put(mes_send_list.wrptr,xtcp_tx_buf);
    mes_send_decode();
    #if 0
    for(uint16_t i=0;i<mes_send_list.len[mes_send_list.wrptr] ;i++){
        if(i!=0 && i%20==0)
            xtcp_debug_printf("\n");
        xtcp_debug_printf("%x ",xtcp_tx_buf[i]);
    }
    xtcp_debug_printf("\n");
    #endif
	mes_send_list.wrptr++;
    
    taskview_page_messend();
}

void mes_send_taskinfo_nopage(task_allinfo_tmp_t* task_all_info){
    if(mes_send_list.wrptr>=MES_STACK_NUM)
        return;
    //备份发送数据
    timer_task_read(&g_tmp_union.task_allinfo_tmp,g_sys_val.task_con_id);
    mes_send_list.len[mes_send_list.wrptr] = taskinfo_upgrade_build(task_all_info,g_sys_val.task_config_s,g_sys_val.task_con_id);
    //memcpy(mes_send_list.tx_buff[mes_send_list.wrptr] ,xtcp_tx_buf,mes_send_list.len[mes_send_list.wrptr]);
	user_messend_buff_put(mes_send_list.wrptr,xtcp_tx_buf);
    mes_send_decode();
    #if 0
    for(uint16_t i=0;i<mes_send_list.len[mes_send_list.wrptr] ;i++){
        if(i!=0 && i%20==0)
            xtcp_debug_printf("\n");
        xtcp_debug_printf("%x ",xtcp_tx_buf[i]);
    }
    xtcp_debug_printf("\n");
    #endif
	mes_send_list.wrptr++;
}


// 即时任务更新通知
void mes_send_rttaskinfo(uint16_t id,uint8_t contorl,uint8_t page_state){
    if(mes_send_list.wrptr>=MES_STACK_NUM)
        return;
    //
    mes_send_list.len[mes_send_list.wrptr] = rttaskinfo_upgrade_build(id,contorl);
    //memcpy(mes_send_list.tx_buff[mes_send_list.wrptr] ,xtcp_tx_buf,mes_send_list.len[mes_send_list.wrptr]);
	user_messend_buff_put(mes_send_list.wrptr,xtcp_tx_buf);
	mes_send_list.wrptr++;
    if(page_state){
    	taskview_page_messend();
    }
}

// 方案更新通知
void mes_send_suloinfo(uint16_t id){
    if(mes_send_list.wrptr>=MES_STACK_NUM)
        return;
    //
    mes_send_list.len[mes_send_list.wrptr] = sulo_upgrade_build(id);
    //memcpy(mes_send_list.tx_buff[mes_send_list.wrptr] ,xtcp_tx_buf,mes_send_list.len[mes_send_list.wrptr]);
	user_messend_buff_put(mes_send_list.wrptr,xtcp_tx_buf);
	mes_send_list.wrptr++;

	taskview_page_messend();
}

void mes_send_overtime(){
    if(mes_send_list.tim_inc){
         mes_send_list.tim_inc++;
         if(mes_send_list.tim_inc>8){
             mes_send_decode();
         }
    }
}

