#include "divlist_decode.h"
#include "protocol_adrbase.h"
#include "ack_build.h"
#include "xtcp.h"
#include "user_xccode.h"
#include "debug_print.h"
#include "list_contorl.h"
#include "user_unti.h"
#include "fl_buff_decode.h"
#include "user_messend.h"
#include "task_decode.h"
#include "conn_process.h"
#include "sys_log.h"
#include "user_log.h"

uint16_t rttask_run_state_set(uint8_t state,uint8_t mac[]);

//---------------------------------------------------------------------------
// 分区列表发送
//--------------------------------------------------------------------------
void arealist_request_rec(){
    uint8_t list_num = list_sending_init(AREA_GETREQUEST_CMD,AREA_LIST_SENDING,&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE]);
    if(list_num == LIST_SEND_INIT)
        return;
    //
    if(g_sys_val.list_sending_f==0){
		g_sys_val.list_sending_f = 1;
	    user_sending_len = area_list_send_build(t_list_connsend[list_num].list_info.arealist.cmd,list_num);
	    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
	}
}
//---------------------------------------------
// 分区列表连发处理
//---------------------------------------------
void arealist_sending_decode(uint8_t list_num){
    user_sending_len = area_list_send_build(t_list_connsend[list_num].list_info.arealist.cmd,list_num);
    user_xtcp_send(t_list_connsend[list_num].conn,t_list_connsend[list_num].could_s);
}

//========================================================================================
// 分区配置指令
//========================================================================================
void area_config_recive(){
    div_node_t *div_tmp_p = div_list.div_head_p;
    //
    uint16_t rx_sn;
    uint8_t i;
    uint8_t ack_state=1;
    uint8_t div_fail_cnt=0;
    uint16_t tmp_area_id = xtcp_rx_buf[AREASET_AREA_SN_B]|(xtcp_rx_buf[AREASET_AREA_SN_B+1]<<8);
    //---------------------------------------------------------------------------------------------------
    // 重名判断
    if(xtcp_rx_buf[AREASET_CONFIG_BYE_B]!=1){
        
        for(uint8_t i=0;i<MAX_AREA_NUM;i++){
            if((area_info[i].area_sn != 0xFFFF) && charncmp(area_info[i].area_name,&xtcp_rx_buf[AREASET_AREA_NAME_B],DIV_NAME_NUM)){
                if(xtcp_rx_buf[AREASET_CONFIG_BYE_B]==0){
                    ack_state = 2;
                    goto area_config_decode_end;
                }
                if(area_info[i].area_sn!=tmp_area_id){
                    ack_state = 2;
                    goto area_config_decode_end;                    
                }
            }
        }
    }
    // 分区列表配置
    if(xtcp_rx_buf[AREASET_CONFIG_BYE_B]==0){   //添加分区
        for(i=0;i<MAX_AREA_NUM;i++){
            if(area_info[i].area_sn == 0xFFFF){
                area_info[i].area_sn = i;
                rx_sn = i;
                area_info[i].account_id = xtcp_rx_buf[AREASET_ACCONUT_ID];
                memcpy(area_info[i].area_name,&xtcp_rx_buf[AREASET_AREA_NAME_B],DIV_NAME_NUM);
                break;
            }
        }
    }
    else{   //编辑分区
        rx_sn = xtcp_rx_buf[AREASET_AREA_SN_B]|(xtcp_rx_buf[AREASET_AREA_SN_B+1]<<8);
        for(i=0;i<MAX_AREA_NUM;i++){
            if(area_info[i].area_sn == rx_sn ){
                //删除分区
                if(xtcp_rx_buf[AREASET_CONFIG_BYE_B]==1){
                    area_info[i].area_sn = 0xFFFF;    
                }
                //修改分区
                else{
                    memcpy(area_info[i].area_name,&xtcp_rx_buf[AREASET_AREA_NAME_B],DIV_NAME_NUM);
                }
                break;
            }    
        }
    }
    if(i==MAX_AREA_NUM){
        user_sending_len = area_config_ack_build(0xFFFF,0,xtcp_rx_buf[AREASET_CONFIG_BYE_B],0); //分区已满
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        return;
    }
    fl_area_write();    // 烧写保存分区信息
    //---------------------------------------------------------------------------------------------------------
    //设备列表配置
    while(div_tmp_p != null){    //设备列表轮询
        uint16_t mac_adr;
        uint8_t state,i;
        uint16_t area_contorl;
        mac_adr = AREASET_DIV_BASE_B; 
        state=0;
        for(i=0;i<xtcp_rx_buf[AREASET_DIV_TOL_B];i++){  // 配置mac地址轮询
            // 设置分区设备
            if(mac_cmp(div_tmp_p->div_info.mac,&xtcp_rx_buf[mac_adr])){
                area_contorl =  (xtcp_rx_buf[mac_adr+6])+(xtcp_rx_buf[mac_adr+7]<<8);
                state = 1;
                break;
            }
            mac_adr+=8;
        }
        // 设备在所属分区
        if(state){
            uint8_t num=0xFF;
            for(i=0;i<MAX_DIV_AREA;i++){
                if(div_tmp_p->div_info.area[i]==rx_sn){
                    num=i;
                    break;
                }
                else if((div_tmp_p->div_info.area[i]==0xFFFF)&&(num==0xFF)){    //找空位置添加设备分区
                    num=i;
                }
            }
            if((i == MAX_DIV_AREA)&&(num==0xFF)){
                //user_sending_len = area_config_ack_build(0xFFFF,0,xtcp_rx_buf[AREASET_CONFIG_BYE_B]); //分区配置失败
                //user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
                //return;
                div_fail_cnt++;
            }
            else{
                div_tmp_p->div_info.area[num] = rx_sn;
                div_tmp_p->div_info.area_contorl[num] = area_contorl;
            }
        }
        // 设备不在所属分区
        else{
            for(i=0;i<MAX_DIV_AREA;i++){
                if(div_tmp_p->div_info.area[i]==rx_sn){
                    div_tmp_p->div_info.area[i]=0xFFFF;
                };
            }
        }
        div_tmp_p = div_tmp_p->next_p;
    } 
    fl_divlist_write(); //保存列表信息
    //----------------------------------------------------------------------------------------------------------
    area_config_decode_end:
    user_sending_len = area_config_ack_build(rx_sn,ack_state,xtcp_rx_buf[AREASET_CONFIG_BYE_B],div_fail_cnt); //scuess
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    //
    log_divarea_config();
    mes_send_listinfo(AREALIS_INFO_REFRESH,0);
    //
}

//=========================================================================================
//---------------------------------------------------------------------------
// 详细信息查询管理
//---------------------------------------------------------------------------
void div_extra_info_recive(){
    user_sending_len = extra_info_build_ack();
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//---------------------------------------------------------------------------
// 设备列表发送列表管理
//---------------------------------------------------------------------------
void divlist_request_recive(){
    uint8_t list_num = list_sending_init(DIVLIST_REQUEST_CMD,DIV_LIST_SENDING,&xtcp_rx_buf[POL_ID_BASE],xtcp_rx_buf[POL_COULD_S_BASE]);
    if(list_num==LIST_SEND_INIT)
        return;
    //
	if(g_sys_val.list_sending_f==0){
		g_sys_val.list_sending_f = 1;
	    user_sending_len = div_list_resend_build(DIVLIST_REQUEST_CMD,&t_list_connsend[list_num].list_info.divlist.div_list_p,DIV_SEND_NUM,list_num);
	    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
	}
}

void div_sending_decode(uint8_t list_num){
    user_sending_len = div_list_resend_build(t_list_connsend[list_num].list_info.divlist.cmd,
                                             &t_list_connsend[list_num].list_info.divlist.div_list_p,DIV_SEND_NUM,list_num);
    user_xtcp_send(t_list_connsend[list_num].conn,t_list_connsend[list_num].could_s);
}


//---------------------------------------------------------------------------
// 设备配置管理
//---------------------------------------------------------------------------
void div_info_set_recive()
{
    div_node_t *div_tmp_p;
    if(mac_cmp(&xtcp_rx_buf[DIVSET_MAC_B],host_info.mac)){
        // 加快信息刷新
        g_sys_val.could_heart_timcnt = 8;
        
        //配置本机信息
        //-----------------------------------------------------------------
        if(xtcp_rx_buf[DIVSET_SETBITMASK_B]&01){ //SET NAME
            memcpy(host_info.name,&xtcp_rx_buf[DIVSET_NAME_B],DIV_NAME_NUM);
        }
        //-----------------------------------------------------------------
         //SET IP INFO
        if((xtcp_rx_buf[DIVSET_SETBITMASK_B]>>1)&01){ //SET IP INFO
            if(charncmp(host_info.ipconfig.ipaddr,&xtcp_rx_buf[DIVSET_IP_B],4)==0){
                g_sys_val.reboot_f = 1;
            }
            host_info.dhcp_en = xtcp_rx_buf[DIVSET_DHCPEN_B];
            memcpy(host_info.ipconfig.ipaddr,&xtcp_rx_buf[DIVSET_IP_B],4);    
            memcpy(host_info.ipconfig.gateway,&xtcp_rx_buf[DIVSET_GATEWAY_B],4);    
            memcpy(host_info.ipconfig.netmask,&xtcp_rx_buf[DIVSET_NETMASK_B],4);    
            user_xtcp_ipconfig(host_info.ipconfig);
            g_sys_val.gateway_standy=0;
            //=================================================================================================
            // 同步主机IP
            div_node_t *node_tmp=null;
            conn_list_t *conn_tmp=null;
            xtcp_connection_t new_conn;
            //
            node_tmp = div_list.div_head_p;
            
            while(node_tmp != null){   
                conn_tmp = get_conn_for_ip(node_tmp->div_info.ip);
                if(conn_tmp==null){
                    if(user_xtcp_connect_udp(8805,node_tmp->div_info.ip,&new_conn)==0){
                        user_sending_len = sync_hostip_build(node_tmp->div_info.mac,host_info.ipconfig.ipaddr);
                        user_xtcp_send(new_conn,0);
                        user_xtcp_close(new_conn);
                    }
                }
                else{
                    new_conn = conn_tmp->conn;
                    user_sending_len = sync_hostip_build(node_tmp->div_info.mac,host_info.ipconfig.ipaddr);
                    user_xtcp_send(new_conn,0);
                }
                node_tmp = node_tmp->next_p;
            }
            
            #if 0
            // 广播主机IP
            xtcp_debug_printf("set ip\n");
            uint8_t mac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
            user_sending_len = sync_hostip_build(mac,host_info.ipconfig.ipaddr);
            user_xtcp_send(g_sys_val.broadcast_conn,0);
            #endif
        }
        //-----------------------------------------------------------------
        //if((xtcp_rx_buf[DIVSET_SETBITMASK_B]>>2)&01){ //SET AUXTYPE
        //    host_info.aux_type = xtcp_rx_buf[DIVSET_AUXTYPE_B];
        //    xtcp_debug_printf("aux type\n");
        //}
        //-----------------------------------------------------------------    
        //if((xtcp_rx_buf[DIVSET_SETBITMASK_B]>>3)&01){ //SET SLIENT
        //    host_info.slient_en = xtcp_rx_buf[DIVSET_SLIENT_EN_B];
        //    if((host_info.slient_en&0x80)!=0)
        //        host_info.slient_lv = xtcp_rx_buf[DIVSET_SLIENT_LV_B];
        //    xtcp_debug_printf("set slient\n");
        //}
        //-----------------------------------------------------------------
        //SET VOL   NONE 
        //if((xtcp_rx_buf[DIVSET_SETBITMASK_B]>>4)&01){ //SET VOL
        //  ;  
        //}
        //-----------------------------------------------------------------
        //if((xtcp_rx_buf[DIVSET_SETBITMASK_B]>>5)&01){ //SET SN
        //    if(sn_cmp(&xtcp_rx_buf[DIVSET_OLDSN_B],host_info.sn)){
        //        memcpy(host_info.sn,&xtcp_rx_buf[DIVSET_NEWSN_B],SYS_PASSWORD_NUM);
        //        xtcp_debug_printf("set sn\n");
        //    }
        //}
        //-------------------------------------------------------
        // flash info 
        log_hostinfo_config();
        fl_hostinfo_write();    //烧写主机信息
        //-----------------------------------------------------------------
    }
    else if((xtcp_rx_buf[DIVSET_SETBITMASK_B]>>6)&01){ //SET DELETE DIV
        //删除设备
        div_tmp_p = get_div_info_p(&xtcp_rx_buf[DIVSET_MAC_B]);
        if(div_tmp_p != null){
            if(div_tmp_p->div_info.div_state==OFFLINE){
                log_div_del(div_tmp_p);
                delete_div_node(div_tmp_p->div_info.mac);
                fl_divlist_write(); //保存列表信息
                mes_send_listinfo(DIVLIS_INFO_REFRESH,0);
                xtcp_rx_buf[DIVSET_SETBITMASK_B] = 0x40;
            }
        }
    }
    user_sending_len = onebyte_ack_build(xtcp_rx_buf[DIVSET_SETBITMASK_B],DIV_INFO_SET_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//---------------------------------------------------------------------------
// 心跳存活管理           0xD000
//---------------------------------------------------------------------------
void div_heart_recive(){
    //----------------------------------------------------------------------
    div_node_t *div_info_p;
    div_info_p = get_div_info_p(&xtcp_rx_buf[POL_DAT_BASE]); //查找列表设备
    uint8_t need_send=0;
    uint8_t state = DIVLIS_INFO_REFRESH;
    //-----------------------------------------------------------------------
    if(div_info_p == null)
        return;
    div_info_p->over_time =0;
    //设备IP是否改变
    if(charncmp(div_info_p->div_info.ip,conn.remote_addr,4)==0){
        //xtcp_debug_printf("ipc\n");
        //xtcp_debug_printf("%d,%d,%d,%d\n",div_info_p->div_info.ip[0],div_info_p->div_info.ip[1],div_info_p->div_info.ip[2],div_info_p->div_info.ip[3]);
        //xtcp_debug_printf("%d,%d,%d,%d\n",conn.remote_addr[0],conn.remote_addr[1],conn.remote_addr[2],conn.remote_addr[3]);
        // 设备IP改变 需刷新任务
        rttask_playlist_updata_init(conn.remote_addr,div_info_p);
        user_updatip_set(div_info_p->div_info.mac,conn.remote_addr);
        memcpy(div_info_p->div_info.ip,conn.remote_addr,4);                         //获取设备IP
        need_send = 1;
    }
    if(div_info_p->div_info.vol != xtcp_rx_buf[HEART_VOL_B]){
        //xtcp_debug_printf("vc  \n");
        div_info_p->div_info.vol = xtcp_rx_buf[HEART_VOL_B];              //获取设备默认音量
        need_send = 1;
    }
    if(charncmp(div_info_p->div_info.name,&xtcp_rx_buf[HEART_NAME_B],DIV_NAME_NUM)==0){
        //xtcp_debug_printf("nc\n");
        memcpy(div_info_p->div_info.name,&xtcp_rx_buf[HEART_NAME_B],DIV_NAME_NUM);  //获取设备名称
        need_send = 1;
    }
    //--------------------------
    #if 0
    xtcp_debug_printf("div name :");
    for(uint8_t i=0;i<DIV_NAME_NUM;i++ ){
        xtcp_debug_printf("%x ",div_info_p->div_info.name[i]);
    }
    xtcp_debug_printf("\n");
    xtcp_debug_printf("rec name :");
    for(uint8_t i=0;i<DIV_NAME_NUM;i++ ){
        xtcp_debug_printf("%x ",xtcp_rx_buf[HEART_NAME_B+i]);
    }
    xtcp_debug_printf("\n");
    #endif 
    //--------------------------
    static uint8_t tmp=0;
	// 设备上线
    if((div_info_p->div_info.div_onlineok)&&(div_info_p->div_info.div_state == 0)&&(xtcp_rx_buf[HEART_STATE_B]!=0)){
        div_info_p->div_info.div_state = ONLINE;
        // 更新日志
        log_divonline(div_info_p);
        //设备上线 异常即时任务恢复
        uint16_t task_id = rttask_run_state_set(01,div_info_p->div_info.mac);
        if(task_id!=0xFFFF){
			mes_send_rttaskinfo(task_id,02,0);
        }
        need_send = 1;
    }
    //
    //xtcp_debug_printf("change state %d\n",div_info_p->div_info.div_state);
    if((div_info_p->div_info.div_state!=0)&&(div_info_p->div_info.div_state!=xtcp_rx_buf[HEART_STATE_B])){
        div_info_p->div_info.div_state = xtcp_rx_buf[HEART_STATE_B];    //获取设备状态
        if(div_info_p->div_info.div_state==0){
            div_info_p->div_info.div_onlineok = 0;
            tmp=1;
            xtcp_debug_printf("divchk offline\n");
        }
        need_send = 1;
    }
    //xtcp_debug_printf("heart info state %d\n",need_send);
    if(need_send){
        mes_send_listinfo(state,!xtcp_rx_buf[HEART_NEEDACK_B]);
    }
    // 应答回复
    if((xtcp_rx_buf[HEART_NEEDACK_B])){
        if(div_info_p->div_info.div_state){
            //xtcp_debug_printf("stamp %d\n",g_sys_val.sys_timinc);
            user_sending_len = heart_ack_build(host_info.hostmode);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        }
        else{// 设备离线，需要重新上线
            user_sending_len = heart_ack_build(2);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        }
    }    
}
//---------------------------------------------------------------------------

//===============================================================================
// 设备录入管理       0xB000
//===============================================================================
uint8_t div_online_tolist(){
    //------------------------------------------------------------------------
    div_node_t *div_info_p;
    div_info_p = get_div_info_p(&xtcp_rx_buf[ONLINE_MAC_B]); //查找列表设备
    //------------------------------------------------------------------------
    // 创建设备节点
    if(div_info_p==null)    //列表里没有设备 
    {
        //xtcp_debug_printf("crea new %x,%x,%x,%x,%x,%x\n",xtcp_rx_buf[ONLINE_MAC_B],xtcp_rx_buf[ONLINE_MAC_B+1],xtcp_rx_buf[ONLINE_MAC_B+2],
        //                                            xtcp_rx_buf[ONLINE_MAC_B+3],xtcp_rx_buf[ONLINE_MAC_B+4],xtcp_rx_buf[ONLINE_MAC_B+5]);
        if(!create_div_node()){ //节点已满
            //xtcp_debug_printf("online full\n");
            return 0; //上线失败
        }
        div_info_p = div_list.div_end_p; //尾插法 获得节点
        // 初始化设备区号
        memset(div_info_p->div_info.area,0xFF,20);
        memset(div_info_p->div_info.area_contorl,0xFF,20);
        
    }
    div_info_p->over_time =0; //重置时间
    //-------------------------------------------------------------------------
    memcpy(div_info_p->div_info.mac,&xtcp_rx_buf[ONLINE_MAC_B],6);  //获取设备MAC
    // 判断IP是否改变
    if(charncmp(div_info_p->div_info.ip,conn.remote_addr,4)==0){
        user_updatip_set(div_info_p->div_info.mac,conn.remote_addr);
        // 设备IP改变 需刷新任务
        rttask_playlist_updata_init(conn.remote_addr,div_info_p);
    }
    memcpy(div_info_p->div_info.ip,conn.remote_addr,4);             //获取设备IP
    div_info_p->div_info.vol = xtcp_rx_buf[ONLINE_VOL_B];  //获取设备默认音量
    
    memcpy(div_info_p->div_info.version,&xtcp_rx_buf[ONLINE_VERSION_B],2);  //获取设备版本号
    memcpy(div_info_p->div_info.name,&xtcp_rx_buf[ONLINE_NAME_B],DIV_NAME_NUM); //获取设备名称
    memcpy(div_info_p->div_info.div_type,&xtcp_rx_buf[ONLINE_DIV_TYPE_B],DIV_TYPE_NUM); //获取设备类型
    //
    //fl_divlist_write(); //保存列表信息 设备状态随后改变
    //  
    /*
    xtcp_debug_printf("pass: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",
                                host_info.sn[0],host_info.sn[1],host_info.sn[2],host_info.sn[3],host_info.sn[4],host_info.sn[5],
                                host_info.sn[6],host_info.sn[7],host_info.sn[8],host_info.sn[9],host_info.sn[10],host_info.sn[11]);
    xtcp_debug_printf("pass2: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",
                                xtcp_rx_buf[ONLINE_PASSWORD_B+0],xtcp_rx_buf[ONLINE_PASSWORD_B+1],xtcp_rx_buf[ONLINE_PASSWORD_B+2],
                                xtcp_rx_buf[ONLINE_PASSWORD_B+3],xtcp_rx_buf[ONLINE_PASSWORD_B+4],xtcp_rx_buf[ONLINE_PASSWORD_B+5],
                                xtcp_rx_buf[ONLINE_PASSWORD_B+6],xtcp_rx_buf[ONLINE_PASSWORD_B+7],xtcp_rx_buf[ONLINE_PASSWORD_B+8],
                                xtcp_rx_buf[ONLINE_PASSWORD_B+9],xtcp_rx_buf[ONLINE_PASSWORD_B+10],xtcp_rx_buf[ONLINE_PASSWORD_B+11]);
    */
    if(sn_cmp(&xtcp_rx_buf[ONLINE_PASSWORD_B],host_info.sn)){
        div_info_p->div_info.div_onlineok = 1;
        return 1;
    }
    //div_info_p->div_info.div_state = SN_ER;                                     
    return 0;
}

//------------------------------------------------------------------------------
void div_online_recive()
{
    user_sending_len = online_request_ack_build((div_online_tolist()),host_info.hostmode);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    return;
}


//=====================================================================================================
// 即时任务 查找 与置为异常或正常状态  1 正常 2异常
//=====================================================================================================
uint16_t rttask_run_state_set(uint8_t state,uint8_t mac[]){
    //即时任务状态异常
    rttask_info_t *tmp_p = rttask_lsit.run_head_p;
    while(tmp_p!=null){
        //比较MAC
        fl_rttask_read(&g_tmp_union.rttask_dtinfo,tmp_p->rttask_id);
        if(charncmp(g_tmp_union.rttask_dtinfo.src_mas,mac,6)){
            //有运行中任务 任务置为异常状态
            tmp_p->run_state = state;
            return tmp_p->rttask_id;
        }
        tmp_p = tmp_p->run_next_p;
    }
    return 0xFFFF;
}

//=====================================================================================================
// 设备掉线       心跳掉线超时检测
//=====================================================================================================
#define DIV_OVERTIME 6
void div_heart_overtime_close(){
    div_node_t *div_node_tmp = div_list.div_head_p;
    if(div_node_tmp != null){
        for(uint8_t i=0;i<MAX_DIV_LIST;i++){    
            div_node_tmp->over_time++;
            if((div_node_tmp->div_info.div_state != OFFLINE)&&(div_node_tmp->over_time>DIV_OVERTIME)){
                //-------------------------------------------------------
                //连接超时,设备离线
                div_node_tmp->div_info.div_state = OFFLINE;
                div_node_tmp->div_info.div_onlineok = OFFLINE;
                uint8_t state = DIVLIS_INFO_REFRESH;
                // 更新日志
                log_divoffline(div_node_tmp);
                //----------------------------------------------------------------------------------
                uint16_t task_id = rttask_run_state_set(02,div_node_tmp->div_info.mac);
				if(task_id!=0xFFFF){
					mes_send_rttaskinfo(task_id,02,0);
				}
				#if 0
                if(rttask_run_state_set(02,div_node_tmp->div_info.mac)){
                    state = RTTASKERROR_INFO_REFRESH;
                    mes_send_listinfo(state,0);
                }
				#endif 
				//
				mes_send_listinfo(DIVLIS_INFO_REFRESH,0);
                //---------------------------------------------------------------------------------------                    
                //
                fl_divlist_write(); //保存列表信息 设备状态随后改变
                xtcp_debug_printf("div_offline\n");
                //-------------------------------------------------------
            }
            if(div_node_tmp->next_p==null){
                break;
            }
            div_node_tmp = div_node_tmp->next_p;
        }
    }
}

//=====================================================================================================
// 设备 IP MAC列表查询
//=====================================================================================================
void div_ip_mac_check_recive(){
    user_sending_len = div_ipmac_list_send();
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//=====================================================================================================
// 搜索设备
//=====================================================================================================
void research_lan_div(){
    user_sending_len = onebyte_ack_build(0,LAN_DIVRESEARCH_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(g_sys_val.broadcast_conn,0);
}

void research_lan_revice(){
    if(g_sys_val.divsreach_f==0)
        return;
    
    memcpy(&g_tmp_union.buff[DIVSRC_MAC_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_MAC_B],6);
    memcpy(&g_tmp_union.buff[DIVSRC_NAME_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_NAME_B],DIV_NAME_NUM);
    g_tmp_union.buff[DIVSRC_STATE_B] = xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_STATE_B];
    g_tmp_union.buff[DIVSRC_VOL_B] = xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_VOL_B];
    memcpy(&g_tmp_union.buff[DIVSRC_PASSWORD_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_PASSWORD_B],SYS_PASSWORD_NUM);
    memcpy(&g_tmp_union.buff[DIVSRC_TYPE_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_TYPE_B],DIV_NAME_NUM);
    g_tmp_union.buff[DIVSRC_VERSION_B] = xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_VERSION_B];
    g_tmp_union.buff[DIVSRC_VERSION_B+1] = xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_VERSION_B+1];
    memcpy(&g_tmp_union.buff[DIVSRC_HOSTIP_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_HOSTIP_B],4);  
    //
    g_tmp_union.buff[DIVSRC_DHCPEN_B] = xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_DHCPEN_B];
    memcpy(&g_tmp_union.buff[DIVSRC_DIVIP_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_DIVIP_B],4);  
    memcpy(&g_tmp_union.buff[DIVSRC_DIVMASK_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_DIVMASK_B],4);  
    memcpy(&g_tmp_union.buff[DIVSRC_DIVGATE_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_DIVGATE_B],4);  

    //
    user_divsrv_write(g_sys_val.search_div_tol,g_tmp_union.buff);
    g_sys_val.search_div_tol++;
    // 关闭广播端口
    if(conn.local_port == LISTEN_BROADCAST_LPPORT){
        xtcp_debug_printf("close conn %x\n",conn.id);
        user_udpconn_close(conn);
    }
    xtcp_debug_printf("have div %d \n",g_sys_val.search_div_tol);
}

//=====================================================================================================
// 获取搜索设备列表
//=====================================================================================================
void sysset_divfound_recive(){
    g_sys_val.search_div_tol = 0;
    g_sys_val.divsearch_conn = conn;
    g_sys_val.divsreach_tim_inc = 0;
    g_sys_val.divsreach_f=1;
    g_sys_val.divsreach_could_f = xtcp_rx_buf[POL_COULD_S_BASE];
    memcpy(g_sys_val.contorl_id,&xtcp_rx_buf[POL_ID_BASE],6);
    /*
    for(uint16_t i=0;i<all_rx_buf[CLH_LEN_BASE]+(all_rx_buf[CLH_LEN_BASE+1]<<16)+6;i++){
        xtcp_debug_printf("%2x ",all_rx_buf[i]);
        if(i%20==0 && i!=0){
            xtcp_debug_printf("\n");
        }
    }
    xtcp_debug_printf("\n");
    */
    // 发送搜索设备命令
    research_lan_div();
}

// 搜索设备上传
void divfound_over_timeinc(){
    if(g_sys_val.divsreach_f){
        g_sys_val.divsreach_tim_inc++;
        if(g_sys_val.divsreach_tim_inc>=30){ //3
            //查找是否有空列表
            uint8_t list_num = list_sending_init(SYSSET_DIVFOUNT_CMD,DIVSRC_LIST_SENDING,g_sys_val.contorl_id,g_sys_val.divsreach_could_f);
            if(list_num==LIST_SEND_INIT)
                return;
            g_sys_val.divsreach_f=0;
            //-----------------------------------
            //搜索列表特殊处理
            xtcp_debug_printf("send divsreach list %d\n",g_sys_val.search_div_tol);
            t_list_connsend[list_num].conn = g_sys_val.divsearch_conn;
            t_list_connsend[list_num].list_info.divsrc_list.div_inc = 0;
            //
        	if(g_sys_val.list_sending_f==0){
				g_sys_val.list_sending_f = 1;
	            user_sending_len = divsrc_list_build(list_num);
	            user_xtcp_send(g_sys_val.divsearch_conn,t_list_connsend[list_num].could_s);
        	}
    	}
    }
}
//---------------------------------------------
// 列表连发处理
//---------------------------------------------
void divsrc_sending_decode(uint8_t list_num){
    user_sending_len = divsrc_list_build(AREA_GETREQUEST_CMD);
    user_xtcp_send(t_list_connsend[list_num].conn,t_list_connsend[list_num].could_s);
}

//=====================================================================================================
// 配置所选搜索设备 目标主机IP
//=====================================================================================================
void divresearch_hostset_recive(){
    uint16_t addr_base=SYSSET_HOSTIP_DIVMAC_B;
    if(xtcp_rx_buf[SYSSET_HOSTIP_DIVTOL_B]>=MAX_DIV_LIST){
        return;
    }
    //xtcp_debug_printf("tol %d\n",xtcp_rx_buf[SYSSET_HOSTIP_DIVTOL_B]);
    //xtcp_debug_printf("%d %d %d %d\n",xtcp_rx_buf[SYSSET_HOSTIP_HOSTIP_B],xtcp_rx_buf[SYSSET_HOSTIP_HOSTIP_B+1],xtcp_rx_buf[SYSSET_HOSTIP_HOSTIP_B+2],xtcp_rx_buf[SYSSET_HOSTIP_HOSTIP_B+3]);
    for(uint8_t i=0;i<xtcp_rx_buf[SYSSET_HOSTIP_DIVTOL_B];i++){
        //xtcp_debug_printf("div mac %x %x %x %x %x %x\n",xtcp_rx_buf[addr_base],xtcp_rx_buf[addr_base+1],xtcp_rx_buf[addr_base+2],
        //                                          xtcp_rx_buf[addr_base+3],xtcp_rx_buf[addr_base+4],xtcp_rx_buf[addr_base+5]);
        user_sending_len = sync_hostip_build(&xtcp_rx_buf[addr_base],&xtcp_rx_buf[SYSSET_HOSTIP_HOSTIP_B]);
        user_xtcp_send(g_sys_val.broadcast_conn,0);    
        addr_base  += 6;
    }
    user_sending_len = onebyte_ack_build(1,SYSSET_DIV_HOSTSET_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

// 
void divlist_ipchk_recive(){
    user_sending_len = divlist_ipchk_ack_build();
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}



