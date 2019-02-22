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

//---------------------------------------------------------------------------
// 分区列表发送
//--------------------------------------------------------------------------
void arealist_send(uint16_t cmd){
    //是否有列表正在发送
    if(conn_sending_s.id!=null)
        return;
    //-----------------------------------
    conn_sending_s.conn_sending_tim = 0;
    //-------------------------------------------------------------------------------------
    // get area total num
    conn_sending_s.arealist.pack_total=0;
    for(uint8_t i=0;i<MAX_AREA_NUM;i++){
        if(area_info[i].area_sn!=0xFFFF)
            conn_sending_s.arealist.pack_total++;
    }
    conn_sending_s.arealist.pack_total = conn_sending_s.arealist.pack_total/AREA_SEND_NUM;
    if(((conn_sending_s.arealist.pack_total%AREA_SEND_NUM)!=0)||conn_sending_s.arealist.pack_total==0)
        conn_sending_s.arealist.pack_total++;
    //-------------------------------------------------------------------------------------
    // Send area list contorl
    conn_sending_s.arealist.pack_inc=0;
    conn_sending_s.arealist.area_inc=0;
    memcpy(conn_sending_s.arealist.id,&xtcp_rx_buf[POL_ID_BASE],6);
    conn_sending_s.arealist.cmd = cmd;
    //
    conn_sending_s.conn_state |= AREA_LIST_SENDING;
    //
    conn_sending_s.could_s = xtcp_rx_buf[POL_COULD_S_BASE];
    could_list_init();
    //
    user_sending_len = area_list_send_build(cmd);
    user_xtcp_send(conn,conn_sending_s.could_s);
}
//---------------------------------------------
// APP 请求获取分区列表
//---------------------------------------------
void arealist_request_rec(){
    arealist_send(AREA_GETREQUEST_CMD);
}
//---------------------------------------------
// 列表连发处理
//---------------------------------------------
void arealist_sending_decode(){
    user_sending_len = area_list_send_build(AREA_GETREQUEST_CMD);
    user_xtcp_send(conn,conn_sending_s.could_s);
}

//========================================================================================
// 分区配置指令
//========================================================================================
void area_config_recive(){
    div_node_t *div_tmp_p = div_list.div_head_p;
    //
    uint16_t rx_sn;
    uint8_t i;
    //---------------------------------------------------------------------------------------------------
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
        user_sending_len = area_config_ack_build(0xFFFF,0,xtcp_rx_buf[AREASET_CONFIG_BYE_B]); //分区配置失败
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
        return;
    }
    area_fl_write();    // 烧写保存分区信息
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
                user_sending_len = area_config_ack_build(0xFFFF,0,xtcp_rx_buf[AREASET_CONFIG_BYE_B]); //分区配置失败
                user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
                return;
            }
                
            div_tmp_p->div_info.area[num] = rx_sn;
            div_tmp_p->div_info.area_contorl[num] = area_contorl;
            
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
    divlist_fl_write(); //保存列表信息
    //----------------------------------------------------------------------------------------------------------
    mes_send_listinfo(AREALIS_INFO_REFRESH,0);
    user_sending_len = area_config_ack_build(rx_sn,1,xtcp_rx_buf[AREASET_CONFIG_BYE_B]); //scuess
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    //
    debug_printf("area_config\n");
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
void divlist_list_send(uint16_t cmd){
    //是否有列表正在发送
    if(conn_sending_s.id!=null)
        return;
    //-----------------------------------------------------------------
    conn_sending_s.conn_sending_tim = 0;
    conn_sending_s.divlist.pack_total = div_list.div_tol/DIV_SEND_NUM;
    if(((div_list.div_tol%DIV_SEND_NUM)!=0)||(div_list.div_tol==0)){
        conn_sending_s.divlist.pack_total++;
    }
    conn_sending_s.divlist.pack_inc=0;
    memcpy(conn_sending_s.divlist.id,&xtcp_rx_buf[POL_ID_BASE],6);
    conn_sending_s.conn_state |= DIV_LIST_SENDING;
    //
    conn_sending_s.divlist.div_list_p=div_list.div_head_p;
    //
    conn_sending_s.divlist.cmd = cmd;

    conn_sending_s.could_s = xtcp_rx_buf[POL_COULD_S_BASE];
    could_list_init();
    //
    user_sending_len = div_list_resend_build(conn_sending_s.divlist.cmd,&conn_sending_s.divlist.div_list_p,DIV_SEND_NUM);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}


// 终端请求列表管理
void divlist_request_recive(){
    divlist_list_send(DIVLIST_REQUEST_CMD);
}

void div_sending_decode(){
    user_sending_len = div_list_resend_build(conn_sending_s.divlist.cmd,&conn_sending_s.divlist.div_list_p,DIV_SEND_NUM);
    user_xtcp_send(conn,conn_sending_s.could_s);
}


//---------------------------------------------------------------------------
// 设备配置管理
//---------------------------------------------------------------------------
void div_info_set_recive()
{
    div_node_t *div_tmp_p;
    if(mac_cmp(&xtcp_rx_buf[DIVSET_MAC_B],host_info.mac)){
        //配置本机信息
        //-----------------------------------------------------------------
        if(xtcp_rx_buf[DIVSET_SETBITMASK_B]&01){ //SET NAME
            memcpy(host_info.name,&xtcp_rx_buf[DIVSET_NAME_B],DIV_NAME_NUM);
        }
        //-----------------------------------------------------------------
         //SET IP INFO
        if((xtcp_rx_buf[DIVSET_SETBITMASK_B]>>1)&01){ //SET IP INFO
            host_info.dhcp_en = xtcp_rx_buf[DIVSET_DHCPEN_B];
            memcpy(host_info.ipconfig.ipaddr,&xtcp_rx_buf[DIVSET_IP_B],4);    
            memcpy(host_info.ipconfig.gateway,&xtcp_rx_buf[DIVSET_GATEWAY_B],4);    
            memcpy(host_info.ipconfig.netmask,&xtcp_rx_buf[DIVSET_NETMASK_B],4);    
            user_xtcp_ipconfig(host_info.ipconfig);
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
                    if(user_xtcp_connect_udp(8805,node_tmp->div_info.ip,&new_conn)){
                         create_conn_node(&new_conn);    //新建一个conn节点    conn_tmp->conn
                    }
                }
                else{
                    new_conn = conn_tmp->conn;
                }
                user_sending_len = sync_hostip_build(node_tmp->div_info.mac,host_info.ipconfig.ipaddr);
                user_xtcp_send(new_conn,0);
                node_tmp = node_tmp->next_p;
            }
            
            
            // 广播主机IP
            debug_printf("set ip\n");
            uint8_t mac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
            user_sending_len = sync_hostip_build(mac,host_info.ipconfig.ipaddr);
            user_xtcp_send(g_sys_val.broadcast_conn,0);
            
        }
        //-----------------------------------------------------------------
        //if((xtcp_rx_buf[DIVSET_SETBITMASK_B]>>2)&01){ //SET AUXTYPE
        //    host_info.aux_type = xtcp_rx_buf[DIVSET_AUXTYPE_B];
        //    debug_printf("aux type\n");
        //}
        //-----------------------------------------------------------------    
        //if((xtcp_rx_buf[DIVSET_SETBITMASK_B]>>3)&01){ //SET SLIENT
        //    host_info.slient_en = xtcp_rx_buf[DIVSET_SLIENT_EN_B];
        //    if((host_info.slient_en&0x80)!=0)
        //        host_info.slient_lv = xtcp_rx_buf[DIVSET_SLIENT_LV_B];
        //    debug_printf("set slient\n");
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
        //        debug_printf("set sn\n");
        //    }
        //}
        //-------------------------------------------------------
        // flash info 
        hostinfo_fl_write();    //烧写主机信息
        //-----------------------------------------------------------------
    }
    else if((xtcp_rx_buf[DIVSET_SETBITMASK_B]>>6)&01){ //SET DELETE DIV
        //删除设备
        div_tmp_p = get_div_info_p(&xtcp_rx_buf[DIVSET_MAC_B]);
        if(div_tmp_p != null){
            if(div_tmp_p->div_info.div_state==OFFLINE){
                delete_div_node(div_tmp_p->div_info.mac);
                divlist_fl_write(); //保存列表信息
                xtcp_rx_buf[DIVSET_SETBITMASK_B] = 0x40;
            }
        }
    }
    user_sending_len = onebyte_ack_build(xtcp_rx_buf[DIVSET_SETBITMASK_B],DIV_INFO_SET_CMD);
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
    //-----------------------------------------------------------------------
    if(div_info_p == null)
        return;
    div_info_p->over_time =0;
    if(charncmp(div_info_p->div_info.ip,conn.remote_addr,4)==0){
        //debug_printf("ipc\n");
        //debug_printf("%d,%d,%d,%d\n",div_info_p->div_info.ip[0],div_info_p->div_info.ip[1],div_info_p->div_info.ip[2],div_info_p->div_info.ip[3]);
        //debug_printf("%d,%d,%d,%d\n",conn.remote_addr[0],conn.remote_addr[1],conn.remote_addr[2],conn.remote_addr[3]);
        memcpy(div_info_p->div_info.ip,conn.remote_addr,4);                         //获取设备IP
        need_send = 1;
    }
    if(div_info_p->div_info.vol != xtcp_rx_buf[HEART_VOL_B]){
        //debug_printf("vc\n");
        div_info_p->div_info.vol = xtcp_rx_buf[HEART_VOL_B];              //获取设备默认音量
        need_send = 1;
    }
    if(charncmp(div_info_p->div_info.name,&xtcp_rx_buf[HEART_NAME_B],DIV_NAME_NUM)==0){
        //debug_printf("nc\n");
        memcpy(div_info_p->div_info.name,&xtcp_rx_buf[HEART_NAME_B],DIV_NAME_NUM);  //获取设备名称
        need_send = 1;
    }
    //--------------------------
    #if 0
    debug_printf("div name :");
    for(uint8_t i=0;i<DIV_NAME_NUM;i++ ){
        debug_printf("%x ",div_info_p->div_info.name[i]);
    }
    debug_printf("\n");
    debug_printf("rec name :");
    for(uint8_t i=0;i<DIV_NAME_NUM;i++ ){
        debug_printf("%x ",xtcp_rx_buf[HEART_NAME_B+i]);
    }
    debug_printf("\n");
    #endif 
    //--------------------------
    static uint8_t tmp=0;
    if((div_info_p->div_info.div_onlineok)&&(div_info_p->div_info.div_state == 0)&&(xtcp_rx_buf[HEART_STATE_B]!=0)){
        mes_send_listinfo(DIVLIS_INFO_REFRESH,0);
        debug_printf("divlist ud online\n");
        div_info_p->div_info.div_state = ONLINE;
    }
    //
    if((div_info_p->div_info.div_state!=0)&&(div_info_p->div_info.div_state!=xtcp_rx_buf[HEART_STATE_B])){
        div_info_p->div_info.div_state = xtcp_rx_buf[HEART_STATE_B];    //获取设备状态
        if(div_info_p->div_info.div_state==0){
            div_info_p->div_info.div_onlineok = 0;
            tmp=1;
            debug_printf("divchk offline\n");
        }
        need_send = 1;
    }
    //debug_printf("heart info state %d\n",need_send);
    if(need_send){
        mes_send_listinfo(DIVLIS_INFO_REFRESH,!xtcp_rx_buf[HEART_NEEDACK_B]);
    }
    // 应答回复
    if((xtcp_rx_buf[HEART_NEEDACK_B])&&(div_info_p->div_info.div_state)){
        //debug_printf("stamp %d\n",g_sys_val.sys_timinc);
        user_sending_len = heart_ack_build(host_info.hostmode);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
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
        debug_printf("crea new %x,%x,%x,%x,%x,%x\n",xtcp_rx_buf[ONLINE_MAC_B],xtcp_rx_buf[ONLINE_MAC_B+1],xtcp_rx_buf[ONLINE_MAC_B+2],
                                                    xtcp_rx_buf[ONLINE_MAC_B+3],xtcp_rx_buf[ONLINE_MAC_B+4],xtcp_rx_buf[ONLINE_MAC_B+5]);
        if(!create_div_node()){ //节点已满
            debug_printf("online full\n");
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
    memcpy(div_info_p->div_info.ip,conn.remote_addr,4);             //获取设备IP
    div_info_p->div_info.vol = xtcp_rx_buf[ONLINE_VOL_B];  //获取设备默认音量
    
    memcpy(div_info_p->div_info.version,&xtcp_rx_buf[ONLINE_VERSION_B],2);  //获取设备版本号
    memcpy(div_info_p->div_info.name,&xtcp_rx_buf[ONLINE_NAME_B],DIV_NAME_NUM); //获取设备名称
    memcpy(div_info_p->div_info.div_type,&xtcp_rx_buf[ONLINE_DIV_TYPE_B],DIV_TYPE_NUM); //获取设备类型
    //
    //divlist_fl_write(); //保存列表信息 设备状态随后改变
    //  
    /*
    debug_printf("pass: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",
                                host_info.sn[0],host_info.sn[1],host_info.sn[2],host_info.sn[3],host_info.sn[4],host_info.sn[5],
                                host_info.sn[6],host_info.sn[7],host_info.sn[8],host_info.sn[9],host_info.sn[10],host_info.sn[11]);
    debug_printf("pass2: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",
                                xtcp_rx_buf[ONLINE_PASSWORD_B+0],xtcp_rx_buf[ONLINE_PASSWORD_B+1],xtcp_rx_buf[ONLINE_PASSWORD_B+2],
                                xtcp_rx_buf[ONLINE_PASSWORD_B+3],xtcp_rx_buf[ONLINE_PASSWORD_B+4],xtcp_rx_buf[ONLINE_PASSWORD_B+5],
                                xtcp_rx_buf[ONLINE_PASSWORD_B+6],xtcp_rx_buf[ONLINE_PASSWORD_B+7],xtcp_rx_buf[ONLINE_PASSWORD_B+8],
                                xtcp_rx_buf[ONLINE_PASSWORD_B+9],xtcp_rx_buf[ONLINE_PASSWORD_B+10],xtcp_rx_buf[ONLINE_PASSWORD_B+11]);
    */
    if(sn_cmp(&xtcp_rx_buf[ONLINE_PASSWORD_B],host_info.sn)){
        div_info_p->div_info.div_onlineok = 1;
        return 1;
    }
    div_info_p->div_info.div_state = SN_ER;    
                                                        
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
                divlist_fl_write(); //保存列表信息 设备状态随后改变
                div_node_tmp->div_info.div_state = OFFLINE;
                div_node_tmp->div_info.div_onlineok = OFFLINE;
                //
                mes_send_listinfo(DIVLIS_INFO_REFRESH,0);
                debug_printf("div_offline\n");
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
    user_sending_len = onebyte_ack_build(0,LAN_DIVRESEARCH_CMD);
    user_xtcp_send(g_sys_val.broadcast_conn,0);
}

void research_lan_revice(){
    if(g_sys_val.divsreach_f==0)
        return;
    
    memcpy(&tmp_union.buff[DIVSRC_MAC_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_MAC_B],6);
    memcpy(&tmp_union.buff[DIVSRC_NAME_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_NAME_B],DIV_NAME_NUM);
    tmp_union.buff[DIVSRC_STATE_B] = xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_STATE_B];
    tmp_union.buff[DIVSRC_VOL_B] = xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_VOL_B];
    memcpy(&tmp_union.buff[DIVSRC_PASSWORD_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_PASSWORD_B],SYS_PASSWORD_NUM);
    memcpy(&tmp_union.buff[DIVSRC_TYPE_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_TYPE_B],DIV_NAME_NUM);
    tmp_union.buff[DIVSRC_VERSION_B] = xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_VERSION_B];
    tmp_union.buff[DIVSRC_VERSION_B+1] = xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_VERSION_B+1];
    memcpy(&tmp_union.buff[DIVSRC_HOSTIP_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_HOSTIP_B],4);  
    //
    tmp_union.buff[DIVSRC_DHCPEN_B] = xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_DHCPEN_B];
    memcpy(&tmp_union.buff[DIVSRC_DIVIP_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_DIVIP_B],4);  
    memcpy(&tmp_union.buff[DIVSRC_DIVMASK_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_DIVMASK_B],4);  
    memcpy(&tmp_union.buff[DIVSRC_DIVGATE_B],&xtcp_rx_buf[DIVSRC_DAT_BASE+DIVSRC_DIVGATE_B],4);  

    //
    user_divsrv_write(g_sys_val.search_div_tol,tmp_union.buff);
    g_sys_val.search_div_tol++;
    // 关闭广播端口
    if(conn.local_port == LISTEN_BROADCAST_LPPORT){
        debug_printf("close conn\n");
        user_udpconn_close(conn);
    }
    debug_printf("have div %d \n",g_sys_val.search_div_tol);
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
        debug_printf("%2x ",all_rx_buf[i]);
        if(i%20==0 && i!=0){
            debug_printf("\n");
        }
    }
    debug_printf("\n");
    */
    // 发送搜索设备命令
    research_lan_div();
}

void divfound_over_timeinc(){
    if(g_sys_val.divsreach_f){
        g_sys_val.divsreach_tim_inc++;
        if(g_sys_val.divsreach_tim_inc>=3){
             //是否有列表正在发送
            if(conn_sending_s.id!=null)
                return;
            g_sys_val.divsreach_f=0;
            //-----------------------------------
            debug_printf("send divsreach list %d\n",g_sys_val.search_div_tol);
            conn_sending_s.id = g_sys_val.divsearch_conn.id;
            conn_sending_s.conn_sending_tim = 0;
            conn_sending_s.could_s = g_sys_val.divsreach_could_f;
            memcpy(conn_sending_s.divsrc_list.id,g_sys_val.contorl_id,6);
            conn_sending_s.divsrc_list.div_inc = 0;
            conn_sending_s.divsrc_list.pack_inc = 0;
            conn_sending_s.conn_state |= DIVSRC_LIST_SENDING;
            user_sending_len = divsrc_list_build();
            debug_printf("srclis len %d\n",user_sending_len);
            user_xtcp_send(g_sys_val.divsearch_conn,conn_sending_s.could_s);
        }
    }
}
//---------------------------------------------
// 列表连发处理
//---------------------------------------------
void divsrc_sending_decode(){
    user_sending_len = divsrc_list_build(AREA_GETREQUEST_CMD);
    user_xtcp_send(g_sys_val.divsearch_conn,conn_sending_s.could_s);
}

//=====================================================================================================
// 配置所选搜索设备 目标主机IP
//=====================================================================================================
void divresearch_hostset_recive(){
    uint16_t addr_base=SYSSET_HOSTIP_DIVMAC_B;
    debug_printf("tol %d\n",xtcp_rx_buf[SYSSET_HOSTIP_DIVTOL_B]);
    debug_printf("%d %d %d %d\n",xtcp_rx_buf[SYSSET_HOSTIP_HOSTIP_B],xtcp_rx_buf[SYSSET_HOSTIP_HOSTIP_B+1],xtcp_rx_buf[SYSSET_HOSTIP_HOSTIP_B+2],xtcp_rx_buf[SYSSET_HOSTIP_HOSTIP_B+3]);
    for(uint8_t i=0;i<xtcp_rx_buf[SYSSET_HOSTIP_DIVTOL_B];i++){
        debug_printf("div mac %x %x %x %x %x %x\n",xtcp_rx_buf[addr_base],xtcp_rx_buf[addr_base+1],xtcp_rx_buf[addr_base+2],
                                                  xtcp_rx_buf[addr_base+3],xtcp_rx_buf[addr_base+4],xtcp_rx_buf[addr_base+5]);
        user_sending_len = sync_hostip_build(&xtcp_rx_buf[addr_base],&xtcp_rx_buf[SYSSET_HOSTIP_HOSTIP_B]);
        user_xtcp_send(g_sys_val.broadcast_conn,0);    
        addr_base  += 6;
    }
    user_sending_len = onebyte_ack_build(1,SYSSET_DIV_HOSTSET_CMD);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}


