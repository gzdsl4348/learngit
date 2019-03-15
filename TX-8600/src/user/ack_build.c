#include "ack_build.h"
#include "checksum.h"
#include "list_instance.h"
#include "list_contorl.h"
#include "protocol_adrbase.h"
#include "user_unti.h"
#include "fl_buff_decode.h"
#include "debug_print.h" 
#include "file_list.h"
#include "account.h"
#include "user_xccode.h"

//========================================================================================
uint16_t build_endpage_decode(uint16_t len,uint16_t cmd,uint8_t id[]){
    //协议头
    ((uint16_t *)xtcp_tx_buf)[POL_HEAD_BASE/2]=HEADER_TAG;
    ((uint16_t *)xtcp_tx_buf)[POL_LEN_BASE/2]=len; //get all len (no header -2 add checksum +2 )
    ((uint16_t *)xtcp_tx_buf)[POL_COM_BASE/2]=cmd;
    memcpy(&xtcp_tx_buf[POL_MAC_BASE],host_info.mac,6);
    memcpy(&xtcp_tx_buf[POL_ID_BASE],id,6);
    xtcp_tx_buf[POL_COULD_S_BASE] = 0;
    // 9字节保留字段
    //xtcp_tx_buf[POL_NULL_BASE] = 0;
    //协议尾
    // +0 checksum
    uint16_t sum;
    sum = chksum_8bit(0,&xtcp_tx_buf[POL_LEN_BASE],(len-2));
    //
    xtcp_tx_buf[len] = sum;
    xtcp_tx_buf[len+1] = sum>>8;
    // +2 end_tag
    xtcp_tx_buf[len+2] = END_TAG;
    xtcp_tx_buf[len+3] = END_TAG>>8;
    //
    len+=4;
    return len;
}
//========================================================================================
// extra info build ack 
//========================================================================================
uint16_t extra_info_build_ack(){
    uint16_t dat_len;
    //-----------------------------------------------------
    //dat begin
    // set mac
    memcpy(&xtcp_tx_buf[EXTRAINFO_MAC_B],host_info.mac,6);
    // set dhcp en
    xtcp_tx_buf[EXTRAINFO_DHCP_EN_B] = host_info.dhcp_en;
    // set netmask
    memcpy(&xtcp_tx_buf[EXTRAINFO_NETMAK_B],host_info.ipconfig.netmask,4);
    // set gateway
    memcpy(&xtcp_tx_buf[EXTRAINFO_GATEWAY_B],host_info.ipconfig.gateway,4);
    // set aux type
    xtcp_tx_buf[EXTRAINFO_AUXTYPE_B] = host_info.aux_type;
    // set slient
    xtcp_tx_buf[EXTRAINFO_SLIENT_B] = host_info.slient_en;
    xtcp_tx_buf[EXTRAINFO_SLIENT_B+1] = host_info.slient_lv;
    // end 
    dat_len = EXTRAINFO_LEN_END;
    //-----------------------------------------------------
    return build_endpage_decode(dat_len,DIV_EXTRA_INFO_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//========================================================================================
// online request ack 
//========================================================================================
uint16_t online_request_ack_build(uint8_t online_state,uint8_t mode){
    uint16_t dat_len=POL_DAT_BASE;
    //dat begin
    xtcp_tx_buf[ONLINE_ACK_STATE_B] = online_state;
    xtcp_tx_buf[ONLINE_MASTERMODE_B] = mode;
    dat_len +=2;
    //-----------------------------------------------------
    return build_endpage_decode(dat_len,ONLINE_REQUEST_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//========================================================================================
// one byte ack 
//========================================================================================
uint16_t onebyte_ack_build(uint8_t mode,uint16_t cmd,uint8_t id[]){
    uint16_t dat_len=POL_DAT_BASE;
    //-----------------------------------------------------
    //dat begin
    xtcp_tx_buf[POL_DAT_BASE] = mode;
    dat_len ++;
    //-----------------------------------------------------
    return build_endpage_decode(dat_len,cmd,id);
}

//========================================================================================
// two byte ack 
//========================================================================================
uint16_t twobyte_ack_build(uint8_t state1,uint8_t state2,uint16_t cmd){
    uint16_t dat_len=POL_DAT_BASE;
    //-----------------------------------------------------
    //dat begin
    xtcp_tx_buf[POL_DAT_BASE] = state1;
    xtcp_tx_buf[POL_DAT_BASE+1] = state2;
    //-----------------------------------------------------
    return build_endpage_decode(POL_DAT_BASE+2,cmd,&xtcp_rx_buf[POL_ID_BASE]);
}

//========================================================================================
// three byte ack 
//========================================================================================
uint16_t threebyte_ack_build(uint8_t state1,uint8_t state2,uint8_t state3,uint16_t cmd){
    uint16_t dat_len=POL_DAT_BASE;
    //-----------------------------------------------------
    //dat begin
    xtcp_tx_buf[POL_DAT_BASE] = state1;
    xtcp_tx_buf[POL_DAT_BASE+1] = state2;
    xtcp_tx_buf[POL_DAT_BASE+2] = state3;
    //-----------------------------------------------------
    return build_endpage_decode(POL_DAT_BASE+3,cmd,&xtcp_rx_buf[POL_ID_BASE]);
}

void could_list_init(){
    debug_printf("cf %d\n",xtcp_rx_buf[POL_COULD_S_BASE]);
    if(xtcp_rx_buf[POL_COULD_S_BASE]){
        conn_sending_s.id = g_sys_val.could_conn.id;
    }
    else{
        conn_sending_s.id = conn.id;
    }
}

//========================================================================================
// id 3byte ack 
//========================================================================================
uint16_t id_ack_build(uint16_t id,uint8_t state,uint16_t cmd){
    xtcp_tx_buf[POL_DAT_BASE] = id;
    xtcp_tx_buf[POL_DAT_BASE+1] = id>>8;
    xtcp_tx_buf[POL_DAT_BASE+2] = state;
    return build_endpage_decode(POL_DAT_BASE+3,cmd,&xtcp_rx_buf[POL_ID_BASE]);
}

//========================================================================================
// 心跳包回复 D000
//========================================================================================
uint16_t heart_ack_build(uint8_t state){
    xtcp_tx_buf[POL_DAT_BASE] = state;
    xtcp_tx_buf[POL_DAT_BASE+1] = g_sys_val.sys_timinc;
    xtcp_tx_buf[POL_DAT_BASE+2] = g_sys_val.sys_timinc>>8;
    xtcp_tx_buf[POL_DAT_BASE+3] = g_sys_val.sys_timinc>>16;
    xtcp_tx_buf[POL_DAT_BASE+4] = g_sys_val.sys_timinc>>24;
    return build_endpage_decode(POL_DAT_BASE+5,DIV_HEART_CMD,&xtcp_rx_buf[POL_ID_BASE]);

}


//========================================================================================
// 设备列表协议
//========================================================================================
uint16_t div_list_resend_build(uint16_t cmd,div_node_t **div_list_p,uint8_t div_send_num){
    uint8_t div_inc=0;
    //
    //-----------------------------------------------------
    //dat begin
    xtcp_tx_buf[DIVLISTRE_TOTALPACK_B] = conn_sending_s.divlist.pack_total;
    xtcp_tx_buf[DIVLISTRE_CURRENTPACK_B] = conn_sending_s.divlist.pack_inc;
    //
    uint16_t div_info_base = DIVLISTRE_INFO_B;
  
    for(uint8_t i=0; i<div_send_num; i++){
        if((*div_list_p)==null)
            break;
        memcpy(&xtcp_tx_buf[div_info_base],&((*div_list_p)->div_info),DIVLISTRE_AREA_B);

        for(uint8_t i=0;i<MAX_DIV_AREA*2;i++){
            xtcp_tx_buf[div_info_base+DIVLISTRE_AREA_B+i] = 0xFF;
        }
        uint8_t tmp=0;
        for(uint8_t i=0;i<MAX_DIV_AREA;i++){
            if((*div_list_p)->div_info.area[i]!=0xFFFF){
                xtcp_tx_buf[div_info_base+DIVLISTRE_AREA_B+tmp*2]= (*div_list_p)->div_info.area[i];
                xtcp_tx_buf[div_info_base+DIVLISTRE_AREA_B+tmp*2+1]= (*div_list_p)->div_info.area[i]>>8;
                xtcp_tx_buf[div_info_base+DIVLISTRE_AREA_CONTORL_B+tmp*2]= (*div_list_p)->div_info.area_contorl[i];
                xtcp_tx_buf[div_info_base+DIVLISTRE_AREA_CONTORL_B+tmp*2+1]= (*div_list_p)->div_info.area_contorl[i]>>8;
                tmp++;
            }
        }
        div_inc++;
        div_info_base +=DIVLISTRE_DIV_INFO_LEN;
        (*div_list_p)=(*div_list_p)->next_p;
    }
    //
    xtcp_tx_buf[DIVLISTRE_TOTALDIV_B] = div_inc;
    conn_sending_s.divlist.pack_inc++;
    //----------------------------------------------------------------------------
    if(conn_sending_s.divlist.pack_inc == (conn_sending_s.divlist.pack_total)){
        conn_sending_s.id=null;
        conn_sending_s.conn_state ^=DIV_LIST_SENDING;
    }
    return build_endpage_decode(div_info_base,cmd,conn_sending_s.divlist.id);
}

//============================================================================================
//分区列表协议
//============================================================================================
uint16_t area_list_send_build(uint16_t cmd){
    //
    //-----------------------------------------------------
    //dat begin
    xtcp_tx_buf[AREAGET_PACKTOTAL_B] = conn_sending_s.arealist.pack_total;
    xtcp_tx_buf[AREAGET_CURRENTPACK_B] = conn_sending_s.arealist.pack_inc;
    uint8_t area_inc=0;
    uint16_t area_dat_base = AREAGET_DAT_BASE_B; 
    //
    for(;conn_sending_s.arealist.area_inc<MAX_AREA_NUM;conn_sending_s.arealist.area_inc++){
        if(area_info[conn_sending_s.arealist.area_inc].area_sn != 0xFFFF){
            // 获取到分区列表
            xtcp_tx_buf[area_dat_base+AREAGET_AREA_SN] = area_info[conn_sending_s.arealist.area_inc].area_sn;
            xtcp_tx_buf[area_dat_base+AREAGET_AREA_SN+1] = area_info[conn_sending_s.arealist.area_inc].area_sn>>8;
            xtcp_tx_buf[area_dat_base+AREAGET_ACCOUNT_ID] = area_info[conn_sending_s.arealist.area_inc].account_id;
            memcpy(&xtcp_tx_buf[area_dat_base+AREAGET_AREA_NAME],area_info[conn_sending_s.arealist.area_inc].area_name,DIV_NAME_NUM);
            area_dat_base += AREAGET_DAT_END;
            area_inc++;
        }
        if(area_inc>AREA_SEND_NUM){
            break;
        }
    }
    xtcp_tx_buf[AREAGET_TOTALAREA_B] = area_inc;
    //
    conn_sending_s.arealist.pack_inc++;
    if(conn_sending_s.arealist.pack_inc == (conn_sending_s.arealist.pack_total)){
        conn_sending_s.id = null;
        conn_sending_s.conn_state ^=AREA_LIST_SENDING;
    }
    return build_endpage_decode(area_dat_base,cmd,conn_sending_s.arealist.id);
}

uint16_t area_config_ack_build(uint16_t area_sn,uint8_t state,uint8_t contorl){
    uint16_t dat_len=POL_DAT_BASE;
    //-----------------------------------------------------
    //dat begin
    xtcp_tx_buf[POL_DAT_BASE] =  contorl;
    xtcp_tx_buf[POL_DAT_BASE+1] =  area_sn;
    xtcp_tx_buf[POL_DAT_BASE+2] = area_sn>>8;
    xtcp_tx_buf[POL_DAT_BASE+3] = state;
    dat_len +=4;
    //-----------------------------------------------------
    return build_endpage_decode(dat_len,AREA_CONFIG_CMD,&xtcp_rx_buf[POL_ID_BASE]);

}
//==========================================================================================
// 主机注册 回包 0xB904
//==========================================================================================
uint16_t host_resiged_ack_build(uint8_t state){
    xtcp_tx_buf[AC_RES_STATE] = state;
    memcpy(&xtcp_tx_buf[AC_RES_CODE],&xtcp_rx_buf[POL_DAT_BASE],20);
    return build_endpage_decode(AC_RES_LENEND,ACCOUNT_REGISTER_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==================================================================================================
// 账户户登录信息
//==================================================================================================
uint16_t account_login_ack_build(uint8_t log_state,uint8_t user_id,uint8_t *mac_buf,uint16_t cmd){
    //
    //-----------------------------------------------------
    //dat begin
    memcpy(&xtcp_tx_buf[AC_LOGIN_NAME_B],&xtcp_rx_buf[AC_LOGIN_NAME_B],DIV_NAME_NUM);
    //加密
    filename_decoder(&xtcp_tx_buf[AC_LOGIN_NAME_B],DIV_NAME_NUM);
    //
    memcpy(&xtcp_tx_buf[AC_LOGIN_PHONENUM_B],account_info[user_id].phone_num,DIV_NAME_NUM);
    //
    xtcp_tx_buf[AC_LOGIN_STATE_B] = log_state;
    //
    memcpy(&xtcp_tx_buf[AC_LOGIN_SYSSN_B],host_info.sn,SYS_PASSWORD_NUM);
    //
    xtcp_tx_buf[AC_LOGIN_ACCOUNT_TYPE_B] = account_info[user_id].type;
    //
    xtcp_tx_buf[AC_LOGIN_ACCOUNT_ID_B] = account_info[user_id].id;
    //
    memcpy(&xtcp_tx_buf[AC_LOGIN_SYS_MAC_B],host_info.mac,6);
    //
    memcpy(&xtcp_tx_buf[AC_LOGIN_SYS_TYPE_B],host_info.div_type,DIV_TYPE_NUM);
    //
    memcpy(&xtcp_tx_buf[AC_LOGIN_SYS_BRAND_B],host_info.div_brand,DIV_TYPE_NUM);
    //
    memcpy(&xtcp_tx_buf[AC_LOGIN_SYS_NAME_B],host_info.name,DIV_TYPE_NUM);
    //
    xtcp_tx_buf[AC_LOGIN_SYS_VERSION_B] = VERSION_TEN_H;
   
    xtcp_tx_buf[AC_LOGIN_SYS_VERSION_B+1] = VERSION_TEN_L;
    debug_printf("ver %x %x  \n",xtcp_tx_buf[AC_LOGIN_SYS_VERSION_B],xtcp_tx_buf[AC_LOGIN_SYS_VERSION_B+1]);
    //
    xtcp_tx_buf[AC_LOGIN_DHCP_EN_B] = host_info.dhcp_en;
    //
    memcpy(&xtcp_tx_buf[AC_LOGIN_IPMASK_B],host_info.ipconfig.netmask,4);
    //
    memcpy(&xtcp_tx_buf[AC_LOGIN_IPGATE_B],host_info.ipconfig.gateway,4);
    //
    #ifdef NO_NEED_REGISTER
    xtcp_tx_buf[AC_LOGIN_RES_STATE_B] = 2;
    #else
    xtcp_tx_buf[AC_LOGIN_RES_STATE_B] = host_info.regiser_state;
    #endif
    //
    xtcp_tx_buf[AC_LOGIN_RES_DAY_B] = host_info.regiser_days;
    xtcp_tx_buf[AC_LOGIN_RES_DAY_B+1] = host_info.regiser_days>>8;

    memcpy(&xtcp_tx_buf[AC_LOGIN_SYS_MACHCODE_B],g_sys_val.maschine_code,10);
    //
    uint16_t mac_date_base = AC_LOGIN_DIV_MAC_B;

    if(log_state!=0){
        xtcp_tx_buf[AC_LOGIN_DIV_TOL_B] = 0;     
    }
    else if(mac_buf!=null){
        xtcp_tx_buf[AC_LOGIN_DIV_TOL_B] = account_info[user_id].div_tol;
        for(uint8_t i=0;i<account_info[user_id].div_tol; i++){
            memcpy(&xtcp_tx_buf[mac_date_base],mac_buf,6);
            mac_date_base+=6;
            mac_buf+=6;
        }
    }
    return build_endpage_decode(mac_date_base,cmd,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 账户列表查看
//==========================================================================================
uint16_t account_list_ack_build(){
    //
    //-----------------------------------------------------
    //dat begin
    uint16_t dat_base = AC_LISTCK_DAT_BASE;
    uint8_t tmp_inc;
    tmp_inc = conn_sending_s.ac_list.account_inc;
    //-------------------------------------------------
    // 
    uint8_t total_user = 0;
    for(; tmp_inc<MAX_ACCOUNT_NUM&&total_user<10; tmp_inc++){
        if(account_info[tmp_inc].id!=0xFF){
            //debug_printf("ac %d,%d\n",account_info[tmp_inc].id,tmp_inc);
            xtcp_tx_buf[dat_base+AC_LISTCK_TYPE_B] = account_info[tmp_inc].type;
            xtcp_tx_buf[dat_base+AC_LISTCK_NUM_B] = account_info[tmp_inc].id;
            //----------------------------------------------------------------------------------------------------
            memcpy(&xtcp_tx_buf[dat_base+AC_LISTCK_NAME_B],account_info[tmp_inc].name,DIV_NAME_NUM);
            // 加密
            for(uint8_t i=0;i<DIV_NAME_NUM/2;i++){
                if(xtcp_tx_buf[dat_base+AC_LISTCK_NAME_B+i*2]==0 && xtcp_tx_buf[dat_base+AC_LISTCK_NAME_B+i*2+1]==0){
                    break;
                }
                xtcp_tx_buf[dat_base+AC_LISTCK_NAME_B+i*2] = xtcp_tx_buf[dat_base+AC_LISTCK_NAME_B+i*2]^g_sys_val.sn_key[i*2];
                xtcp_tx_buf[dat_base+AC_LISTCK_NAME_B+i*2+1] = xtcp_tx_buf[dat_base+AC_LISTCK_NAME_B+i*2+1]^g_sys_val.sn_key[i*2+1];
            }
            //--------------------------------------------------------------------------------------------------------
            memcpy(&xtcp_tx_buf[dat_base+AC_LISTCK_SN_B],account_info[tmp_inc].sn,SYS_PASSWORD_NUM);
            // 加密
            for(uint8_t i=0;i<SYS_PASSWORD_NUM-2;i++){
                xtcp_tx_buf[dat_base+AC_LISTCK_SN_B+i] = xtcp_tx_buf[dat_base+AC_LISTCK_SN_B+i]^g_sys_val.sn_key[i];
            }    
            //----------------------------------------------------------------------------------------------------------------
            //电话
            memcpy(&xtcp_tx_buf[dat_base+AC_LISTCK_PHONE_NUM_B],account_info[tmp_inc].phone_num,DIV_NAME_NUM);
            // 时间限制
            if((account_info[tmp_inc].time_info.hour>23)||(account_info[tmp_inc].time_info.minute>59)||(account_info[tmp_inc].time_info.second>59))
               memset(&account_info[tmp_inc].time_info,0,3); 
            memcpy(&xtcp_tx_buf[dat_base+AC_LISTCK_TIME_B],&account_info[tmp_inc].time_info,3);
            // 日期限制
            if(((account_info[tmp_inc].date_info.date>31)||(account_info[tmp_inc].date_info.date==0))||
                ((account_info[tmp_inc].date_info.month==0)||(account_info[tmp_inc].date_info.month>12)))
                memset(&account_info[tmp_inc].date_info,1,3);                 
            memcpy(&xtcp_tx_buf[dat_base+AC_LISTCK_DATE_B],&account_info[tmp_inc].date_info,3);
            //
            memcpy(&xtcp_tx_buf[dat_base+AC_LISTCK_BUILD_TIME_B],&account_info[tmp_inc].build_time_info,3);
            memcpy(&xtcp_tx_buf[dat_base+AC_LISTCK_BUILD_DATE_B],&account_info[tmp_inc].build_date_info,3);

            //debug_printf("year %d mon %d day %d\n",xtcp_tx_buf[dat_base+AC_LISTCK_BUILD_DATE_B],
            //    xtcp_tx_buf[dat_base+AC_LISTCK_BUILD_DATE_B+1],xtcp_tx_buf[dat_base+AC_LISTCK_BUILD_DATE_B+2]);

            xtcp_tx_buf[dat_base+AC_LISTCK_DIV_TOL_B] = account_info[tmp_inc].div_tol;
                
            dat_base +=AC_LISTCK_DATLEN_B;
            total_user++;
        }//if 
    }//for
    
    conn_sending_s.ac_list.account_inc = tmp_inc;
    xtcp_tx_buf[AC_LISTCK_PAGENUM_B] = conn_sending_s.ac_list.pack_inc;
    xtcp_tx_buf[AC_LISTCK_TOLNUM_B] = total_user;
    xtcp_tx_buf[AC_LISTCK_TOLPAGE_B] = conn_sending_s.ac_list.pack_tol;

    conn_sending_s.ac_list.pack_inc++;
    
    if(conn_sending_s.ac_list.pack_inc >= (conn_sending_s.ac_list.pack_tol)){
        conn_sending_s.id = null;
        conn_sending_s.conn_state ^=AC_LIST_SENDING;
    }
    
    return build_endpage_decode(dat_base,ACCOUNT_USER_LIST_CMD,conn_sending_s.ac_list.id);
}

//==========================================================================================
// 账户权限MAC列表查看
//==========================================================================================
uint16_t account_maclist_ack_build(account_all_info_t *account_all_info){
    //
    //-----------------------------------------------------
    xtcp_tx_buf[AC_MACLIST_ID_B] = account_all_info->account_info.id;
    xtcp_tx_buf[AC_MACLIST_TOL_B] = account_all_info->account_info.div_tol;
    uint16_t data_base=0;
    for(uint8_t i=0;i<xtcp_tx_buf[AC_MACLIST_TOL_B];i++){
        memcpy(&xtcp_tx_buf[AC_MACLIST_MAC_B+data_base],(account_all_info->mac_list+data_base),6);
        data_base+=6;
    }
    return build_endpage_decode(data_base+AC_MACLIST_MAC_B,ACCOUNT_DIV_LIST_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 账户配置回复
//==========================================================================================
uint16_t account_control_ack_build(uint8_t contorl,uint8_t id,uint8_t state){
    //-----------------------------------------------------
    xtcp_tx_buf[AC_CONFIG_CONTORL_B] = contorl;
    xtcp_tx_buf[AC_CONFIG_ACNUM_B] = id;
    xtcp_tx_buf[AC_CONFIG_ACKS_B] = state;
    //
    return build_endpage_decode(AC_CONFIG_ENDLEN_B,ACCOUNT_CONFIG_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 方案列表查看
//==========================================================================================
uint16_t solution_list_ack_build(uint16_t cmd){
    //-----------------------------------------------------
    //
    uint16_t data_base = SOLU_CK_DAT_BASE;
    xtcp_tx_buf[SOLU_CK_TOLNUM] = 0;
    for(uint8_t i=0; i<(MAX_TASK_SOULTION); i++){
        if(solution_list.solu_info[i].state==0xFF)
            continue;
        xtcp_tx_buf[data_base+SOLU_CK_ID] = solution_list.solu_info[i].id;
        xtcp_tx_buf[data_base+SOLU_CK_STATE] = solution_list.solu_info[i].en;
        memcpy(&xtcp_tx_buf[data_base+SOLU_CK_NAME],solution_list.solu_info[i].name,DIV_NAME_NUM);
        memcpy(&xtcp_tx_buf[data_base+SOLU_CK_BEGDATE],&solution_list.solu_info[i].begin_date,3);
        memcpy(&xtcp_tx_buf[data_base+SOLU_CK_ENDDATE],&solution_list.solu_info[i].end_date,3);
        xtcp_tx_buf[data_base+SOLU_CK_PRIO] = solution_list.solu_info[i].prio; 
        //debug_printf("id %d %d\n", xtcp_tx_buf[data_base+SOLU_CK_ID],xtcp_tx_buf[data_base+SOLU_CK_STATE] );
        //
        xtcp_tx_buf[SOLU_CK_TOLNUM]++; 
        data_base += SOLU_CK_LEN_END;
    }
    return build_endpage_decode(data_base,cmd,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 方案编辑回复
//==========================================================================================
uint16_t solution_config_build(uint16_t id,uint8_t state,uint8_t config){
    xtcp_tx_buf[SOLU_CFGACK_ID] = id;
    xtcp_tx_buf[SOLU_CFGACK_CONFIG] = config;
    xtcp_tx_buf[SOLU_CFGACK_STATE] = state;
    //
    return build_endpage_decode(SOLU_CFGACK_LENEND,SOLUTION_CONFIG_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 任务列表查看
//==========================================================================================
#define MAX_TASK_ONCESEND   10
uint16_t task_list_ack_build(){
    uint8_t i;
    task_coninfo_t * tmp_p;
    //-----------------------------------------------------
    xtcp_tx_buf[TASK_CK_TOLPACK] = (timetask_list.task_total/MAX_TASK_ONCESEND);
    if((timetask_list.task_total%MAX_TASK_ONCESEND!=0)||(timetask_list.task_total==0))
        xtcp_tx_buf[TASK_CK_TOLPACK]++;
    xtcp_tx_buf[TASK_CK_PACK_NUM] =conn_sending_s.tasklist.pack_inc;
    //
    uint16_t data_base = TASK_CK_DAT_BASE;
    //
    for(i=0;i<MAX_TASK_ONCESEND; i++){
        if(conn_sending_s.tasklist.task_p==null)
            break;
        // 取flash数据
        timer_task_read(&tmp_union.task_allinfo_tmp,conn_sending_s.tasklist.task_p->id);
        tmp_p = &tmp_union.task_allinfo_tmp.task_coninfo;
        //
        xtcp_tx_buf[data_base+TASK_CK_SOLU_ID] = tmp_p->solution_sn; 
        xtcp_tx_buf[data_base+TASK_CK_TASK_ID] = conn_sending_s.tasklist.task_p->id; 
        xtcp_tx_buf[data_base+TASK_CK_TASK_ID+1] = tmp_p->task_id>>8;
        memcpy(&xtcp_tx_buf[data_base+TASK_CK_TASK_NAME],tmp_p->task_name,DIV_NAME_NUM); 
        xtcp_tx_buf[data_base+TASK_CK_TASK_STATE] = tmp_p->task_state;
        xtcp_tx_buf[data_base+TASK_CK_TASK_PRIO] = tmp_p->task_prio;
        xtcp_tx_buf[data_base+TASK_CK_TASK_VOL] = tmp_p->task_vol;
        xtcp_tx_buf[data_base+TASK_CK_REPE_MODE] = tmp_p->task_repe_mode;
        xtcp_tx_buf[data_base+TASK_CK_REPE_WEEK] = tmp_p->week_repebit;
        memcpy(&xtcp_tx_buf[data_base+TASK_CK_REPE_DATE],tmp_p->dateinfo,3*MAX_TASK_DATE_NUM);
        memcpy(&xtcp_tx_buf[data_base+TASK_CK_BEG_TIME],&tmp_p->time_info,3);
        xtcp_tx_buf[data_base+TASK_CK_DURA_TIME] =   tmp_p->dura_time/3600;
        xtcp_tx_buf[data_base+TASK_CK_DURA_TIME+1] = (tmp_p->dura_time%3600)/60;
        xtcp_tx_buf[data_base+TASK_CK_DURA_TIME+2] = (tmp_p->dura_time%3600)%60;;
        xtcp_tx_buf[data_base+TASK_CK_PLAY_MODE] = tmp_p->play_mode;
        xtcp_tx_buf[data_base+TASK_CK_PLAY_TOL] = tmp_p->music_tolnum;
        //-------------------------------------------------------
        xtcp_tx_buf[data_base+TASK_CK_TEXTPLAY_S] = 0;
        for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
            if((timetask_now.ch_state[i]!=0xFF)&&(timetask_now.task_musicplay[i].task_id==tmp_p->task_id)){
                xtcp_tx_buf[data_base+TASK_CK_TEXTPLAY_S] = 1;
                //debug_printf("\n\nhave task id %d\n\n",tmp_p->task_id);
            }
        }
        //debug_printf("\n task s: %d \n",xtcp_tx_buf[data_base+TASK_CK_TEXTPLAY_S]);
        //
        conn_sending_s.tasklist.task_p = conn_sending_s.tasklist.task_p->all_next_p;
        data_base +=TASK_CK_LEN_END;
    }
    xtcp_tx_buf[TASK_CK_TASK_TOL] = i;
    conn_sending_s.tasklist.pack_inc++;
    if(conn_sending_s.tasklist.pack_inc==xtcp_tx_buf[TASK_CK_TOLPACK]){
        conn_sending_s.conn_state ^= TASK_LIST_SENDING;
        conn_sending_s.id=null;
    }
    //
    return build_endpage_decode(data_base,TASK_CHECK_CMD,conn_sending_s.tasklist.id);
}

//==========================================================================================
// 任务详细信息查看
//==========================================================================================
#define MAX_SEND_MUSIC  10
uint16_t task_dtinfo_chk_build(){
    uint16_t data_base;
    //-----------------------------------------------------
    xtcp_tx_buf[TASK_DTG_ACK_ID] = tmp_union.task_allinfo_tmp.task_coninfo.task_id;
    xtcp_tx_buf[TASK_DTG_ACK_ID+1] = tmp_union.task_allinfo_tmp.task_coninfo.task_id>>8;
    xtcp_tx_buf[TASK_DTG_TOLPACK] = tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum/MAX_SEND_MUSIC+1;
    if(((tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum%MAX_SEND_MUSIC)!=0)||
        (tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum==0))
    {
        xtcp_tx_buf[TASK_DTG_TOLPACK]++;
    }
    xtcp_tx_buf[TASK_DTG_PACK_NUM] = conn_sending_s.task_dtinfo.pack_inc; 
    //设备总数包
    if(xtcp_tx_buf[TASK_DTG_PACK_NUM]==0){
        xtcp_tx_buf[TASK_DTG_PACK_TYPE]=0;
        xtcp_tx_buf[TASK_DTG_DIV_TOL] = tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum;
        data_base = TASK_DTG_DIV_BASE;
        //
        for(uint8_t i=0;i<xtcp_tx_buf[TASK_DTG_DIV_TOL];i++){
            xtcp_tx_buf[data_base+TASK_DTG_DIV_AREACFG] = tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].zone_control;
            xtcp_tx_buf[data_base+TASK_DTG_DIV_AREACFG+1] = tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].zone_control>>8;
            memcpy(&xtcp_tx_buf[data_base+TASK_DTG_DIV_MAC],tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac,6); 
            data_base += TASK_DTG_DIV_LEN;
        }
    }
    //音乐信息包
    else{
        xtcp_tx_buf[TASK_DTG_PACK_TYPE]=1;
        data_base = TASK_DTG_DIV_BASE;
        uint8_t i;
        for(i=0; (i<MAX_SEND_MUSIC)&&(conn_sending_s.task_dtinfo.music_inc<tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum); i++){
            //获得音乐路径名
            memcpy(&xtcp_tx_buf[data_base+TASK_DTG_MUSIC_PATH],
                    tmp_union.task_allinfo_tmp.task_musiclist.music_info[conn_sending_s.task_dtinfo.music_inc].music_path,PATCH_NAME_NUM);
            //获得音乐名
            memcpy(&xtcp_tx_buf[data_base+TASK_DTG_MUSIC_NAME],
                    tmp_union.task_allinfo_tmp.task_musiclist.music_info[conn_sending_s.task_dtinfo.music_inc].music_name,MUSIC_NAME_NUM);
            //
            conn_sending_s.task_dtinfo.music_inc++;
            data_base += TASK_DTG_MUSIC_LEN;
        }        
        xtcp_tx_buf[TASK_DTG_MUSIC_TOL] = i;
    }
    conn_sending_s.task_dtinfo.pack_inc++;
    if(conn_sending_s.task_dtinfo.pack_inc == xtcp_tx_buf[TASK_DTG_TOLPACK]){
        conn_sending_s.id=null;
        conn_sending_s.conn_state ^= TASK_DTINFO_SENDING;
    }
    return build_endpage_decode(data_base,TASK_DTINFO_CK_CMD,conn_sending_s.task_dtinfo.id);
}

//==========================================================================================
// 任务配置回复
//==========================================================================================
uint16_t task_config_ack_build(uint16_t id,uint8_t state){
    //-----------------------------------------------------
    xtcp_tx_buf[TASKC_CFG_TASK_ID] = id;
    xtcp_tx_buf[TASKC_CFG_TASK_ID+1] = id>>8;
    xtcp_tx_buf[TASKC_CFG_STATE] = state;
    //
    return build_endpage_decode(TASKC_CFG_ACK_LEN,TASK_CONFIG_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 今日任务查询回复
//==========================================================================================
uint16_t todaytask_ack_build(){
    //-----------------------------------------------------
    xtcp_tx_buf[POL_DAT_BASE] = g_sys_val.today_date.week;
    xtcp_tx_buf[POL_DAT_BASE+1] = g_sys_val.today_date.year;
    xtcp_tx_buf[POL_DAT_BASE+2] = g_sys_val.today_date.month;
    xtcp_tx_buf[POL_DAT_BASE+3] = g_sys_val.today_date.date;
    //
    //debug_printf("chk week %d\n",xtcp_tx_buf[POL_DAT_BASE]);
    return build_endpage_decode(POL_DAT_BASE+4,TASK_TODAYWEEK_CK_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 即时任务配置回复
//==========================================================================================
uint16_t rttask_config_ack_build(uint16_t id,uint8_t state){
    //-----------------------------------------------------
    xtcp_tx_buf[RTTASK_CFGC_ID] = id;
    xtcp_tx_buf[RTTASK_CFGC_ID+1] = id>>8;
    xtcp_tx_buf[RTTASK_CFGC_STATE] = state;
    //
    return build_endpage_decode(RTTASK_CFGC_LEN,RTTASK_CONFIG_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 即时任务列表查询
//==========================================================================================
#define MAX_RTTASK_SEND     20
uint16_t rttask_list_chk_build(){
    uint8_t i;
    uint16_t data_base;
    //-----------------------------------------------------
    // 获得总包数
    xtcp_tx_buf[RTTASK_CK_TOLPACK] = (rttask_lsit.rttask_tol/MAX_RTTASK_SEND);
    if((rttask_lsit.rttask_tol%MAX_RTTASK_SEND!=0)||(rttask_lsit.rttask_tol==0))
        xtcp_tx_buf[RTTASK_CK_TOLPACK]++;
    // 当前包序号
    xtcp_tx_buf[RTTASK_CK_PACKNUM] = conn_sending_s.rttasklist.pack_inc;
    // 任务列表
    data_base = RTTASK_CK_BASE;
    //
    for(i=0;(i<MAX_RTTASK_SEND);i++){
        if(conn_sending_s.rttasklist.rttask_p==null)
            break;
        // 获得即时任务详细信息
        rt_task_read(&tmp_union.rttask_dtinfo,conn_sending_s.rttasklist.rttask_p->rttask_id);
        // 获得账户ID
        xtcp_tx_buf[data_base+RTTASK_CK_ACID] = tmp_union.rttask_dtinfo.account_id;
        // 获得即时任务ID
        xtcp_tx_buf[data_base+RTTASK_CK_TASKID] = tmp_union.rttask_dtinfo.rttask_id;
        xtcp_tx_buf[data_base+RTTASK_CK_TASKID+1] = tmp_union.rttask_dtinfo.rttask_id>>8;
        // 获得任务名称
        memcpy(&xtcp_tx_buf[data_base+RTTASK_CK_TASKNAME],tmp_union.rttask_dtinfo.name,DIV_NAME_NUM);
        // 播放源MAC
        memcpy(&xtcp_tx_buf[data_base+RTTASK_CK_SRCMAC],tmp_union.rttask_dtinfo.src_mas,6);
        // 保留 播放优先级
        xtcp_tx_buf[data_base+RTTASK_CK_TASKPRIO]=tmp_union.rttask_dtinfo.prio;
        // 任务音量
        xtcp_tx_buf[data_base+RTTASK_CK_TASKVOL] = tmp_union.rttask_dtinfo.task_vol;
        // 持续时间
        xtcp_tx_buf[data_base+RTTASK_CK_DURATIME] = tmp_union.rttask_dtinfo.dura_time/3600;
        xtcp_tx_buf[data_base+RTTASK_CK_DURATIME+1] = (tmp_union.rttask_dtinfo.dura_time%3600)/60;
        xtcp_tx_buf[data_base+RTTASK_CK_DURATIME+2] = (tmp_union.rttask_dtinfo.dura_time%3600)%60;
        // 遥控按键
        xtcp_tx_buf[data_base+RTTASK_CK_KEYINFO] = tmp_union.rttask_dtinfo.task_key;
        // 任务状态
        //-----------------------------------------------------------------
        rttask_info_t *tmp_p = rttask_lsit.run_head_p;
        xtcp_tx_buf[data_base+RTTASK_CK_STATE] = 0;
        while(tmp_p!=null){
            if(tmp_p->rttask_id == tmp_union.rttask_dtinfo.rttask_id){
                xtcp_tx_buf[data_base+RTTASK_CK_STATE] = rttask_lsit.run_end_p->run_state;
                break;
            }
            tmp_p = tmp_p->run_next_p;
        }
        //-----------------------------------------------------------------
        conn_sending_s.rttasklist.rttask_p = conn_sending_s.rttasklist.rttask_p->all_next_p;
        data_base += RTTASK_CK_LEN;
    }
    xtcp_tx_buf[RTTASK_CK_TASKTOL] = i;
    conn_sending_s.rttasklist.pack_inc++;
    if(conn_sending_s.rttasklist.pack_inc == xtcp_tx_buf[RTTASK_CK_TOLPACK]){
        conn_sending_s.conn_state ^= RTTASK_LIST_SENDING;
        conn_sending_s.id=null;
    }
    return build_endpage_decode(data_base,RTTASK_CHECK_CMD,conn_sending_s.rttasklist.id);
}

//==========================================================================================
// 即时任务详细信息查询
//==========================================================================================
uint16_t rttask_dtinfo_chk_build(uint16_t id){
    uint16_t data_base;
    //-----------------------------------------------------
    rt_task_read(&tmp_union.rttask_dtinfo,id);
    //
    xtcp_tx_buf[RTTASK_DTCK_ACKID] = id;
    xtcp_tx_buf[RTTASK_DTCK_ACKID+1] = id>>8;
    if(tmp_union.rttask_dtinfo.div_tol>100)
        tmp_union.rttask_dtinfo.div_tol = 100;
    xtcp_tx_buf[RTTASK_DTCK_DIVTOL] = tmp_union.rttask_dtinfo.div_tol;
    data_base = RTTASK_DTCK_DATBASE;
    for(uint8_t i=0;i<xtcp_tx_buf[RTTASK_DTCK_DIVTOL];i++){
        //设备MAC
        memcpy(&xtcp_tx_buf[data_base+RTTASK_DTCK_DIVMAC],tmp_union.rttask_dtinfo.des_info[i].mac,6);
        //分区控制标志
        xtcp_tx_buf[data_base+RTTASK_DTCK_AREACONTORL] = tmp_union.rttask_dtinfo.des_info[i].zone_control;
        xtcp_tx_buf[data_base+RTTASK_DTCK_AREACONTORL+1] = tmp_union.rttask_dtinfo.des_info[i].zone_control>>8;
        data_base+=RTTASK_DTCK_LEN;
    }
    return build_endpage_decode(data_base,RTTASK_DTINFO_CHECK_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 即时任务信息建立组包 B407
//==========================================================================================
uint16_t rttask_connect_build(uint8_t contorl,uint16_t ran_id,uint16_t id,uint16_t cmd,uint8_t no_task){
    uint16_t data_base;
    div_node_t *div_tmp_p=null;
    //-----------------------------------------------------
    if(no_task){
        memset(&tmp_union.rttask_dtinfo,0x00,sizeof(rttask_dtinfo_t));
    }
    // 获取任务详细信息
    else{
        rt_task_read(&tmp_union.rttask_dtinfo,id);
    }
    // 任务建立与关闭
    xtcp_tx_buf[RTTASK_BUILD_CONTORL] = contorl;
    //
    xtcp_tx_buf[RTTASK_BUILD_ID] = tmp_union.rttask_dtinfo.rttask_id;
    xtcp_tx_buf[RTTASK_BUILD_ID+1] = tmp_union.rttask_dtinfo.rttask_id>>8;
    debug_printf("rttask id %d\n",tmp_union.rttask_dtinfo.rttask_id);
    // 随机ID
    xtcp_tx_buf[RTTASK_BUILD_CONID] = ran_id;
    xtcp_tx_buf[RTTASK_BUILD_CONID+1] = ran_id>>8;
    // 任务优先级 保留
    xtcp_tx_buf[RTTASK_BUILD_PRIO] = tmp_union.rttask_dtinfo.prio;
    // 任务音量
    xtcp_tx_buf[RTTASK_BUILD_VOL] = tmp_union.rttask_dtinfo.task_vol;
    // 持续时间
    xtcp_tx_buf[RTTASK_BUILD_DURATIME] = tmp_union.rttask_dtinfo.dura_time/3600;
    xtcp_tx_buf[RTTASK_BUILD_DURATIME+1] = (tmp_union.rttask_dtinfo.dura_time%3600)/60;
    xtcp_tx_buf[RTTASK_BUILD_DURATIME+2] = (tmp_union.rttask_dtinfo.dura_time%3600)%60;
    // 设备总量
    xtcp_tx_buf[RTTASK_BUILD_DIVTOL] = tmp_union.rttask_dtinfo.div_tol;
    //
    data_base = RTTASK_BUILD_BASE;

    for(uint8_t i=0; i<tmp_union.rttask_dtinfo.div_tol; i++){
        //取得设备指针
        div_tmp_p = get_div_info_p(tmp_union.rttask_dtinfo.des_info[i].mac);
        if(div_tmp_p==null){
            xtcp_tx_buf[RTTASK_BUILD_DIVTOL]--;
            continue;
        }
        /*
        if(div_tmp_p->div_info.div_state==0){
            xtcp_tx_buf[RTTASK_BUILD_DIVTOL]--;
            continue;
        }*/
        
        //获得设备IP
        memcpy(&xtcp_tx_buf[data_base+RTTASK_BUILD_IP],div_tmp_p->div_info.ip,4);
        //分区控制位
        xtcp_tx_buf[data_base+RTTASK_BUILD_AREACONTORL] = tmp_union.rttask_dtinfo.des_info[i].zone_control;
        xtcp_tx_buf[data_base+RTTASK_BUILD_AREACONTORL+1] = tmp_union.rttask_dtinfo.des_info[i].zone_control>>8;
        //设备MAC
        memcpy(&xtcp_tx_buf[data_base+RTTASK_BUILD_MAC],tmp_union.rttask_dtinfo.des_info[i].mac,6);
        //
        data_base += RTTASK_BUILD_LEN;
    }
    //
    return build_endpage_decode(data_base,cmd,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 即时任务列表刷新 BF0C
//==========================================================================================
uint16_t  rttask_listupdat_build(uint8_t *needsend,uint16_t id,div_node_t *rttask_div_p){
    uint16_t data_base;
    div_node_t *div_tmp_p=null;
    // 读取任务信息
    rt_task_read(&tmp_union.rttask_dtinfo,id);
    // 设备总量
    xtcp_tx_buf[RTTASK_LISTUP_DIVTOL] = tmp_union.rttask_dtinfo.div_tol;
    //
    data_base = RTTASK_LISTUP_LIST_BASE;
    *needsend = 0;
    for(uint8_t i=0; i<tmp_union.rttask_dtinfo.div_tol; i++){
        //取得设备指针
        div_tmp_p = get_div_info_p(tmp_union.rttask_dtinfo.des_info[i].mac);
        if(div_tmp_p==null){
            xtcp_tx_buf[RTTASK_LISTUP_DIVTOL]--;
            continue;
        }
        
        //获得设备IP
        memcpy(&xtcp_tx_buf[data_base+RTTASK_LISTUP_IP],div_tmp_p->div_info.ip,4);
        //分区控制位
        xtcp_tx_buf[data_base+RTTASK_LISTUP_AREACONTORL] = tmp_union.rttask_dtinfo.des_info[i].zone_control;
        xtcp_tx_buf[data_base+RTTASK_LISTUP_AREACONTORL+1] = tmp_union.rttask_dtinfo.des_info[i].zone_control>>8;
        //设备MAC
        memcpy(&xtcp_tx_buf[data_base+RTTASK_LISTUP_MAC],tmp_union.rttask_dtinfo.des_info[i].mac,6);
        //
        //判断是否需要更新
        #if 0
        debug_printf("%x,%x,%x,%x,%x,%x\n%x,%x,%x,%x,%x,%x\n",tmp_union.rttask_dtinfo.des_info[i].mac[0],tmp_union.rttask_dtinfo.des_info[i].mac[1],
                                                              tmp_union.rttask_dtinfo.des_info[i].mac[2],tmp_union.rttask_dtinfo.des_info[i].mac[3],
                                                              tmp_union.rttask_dtinfo.des_info[i].mac[4],tmp_union.rttask_dtinfo.des_info[i].mac[5],
                                                              rttask_div_p->div_info.mac[0],rttask_div_p->div_info.mac[1],rttask_div_p->div_info.mac[2],
                                                              rttask_div_p->div_info.mac[3],rttask_div_p->div_info.mac[4],rttask_div_p->div_info.mac[5]);
        #endif
        if(charncmp(tmp_union.rttask_dtinfo.des_info[i].mac,rttask_div_p->div_info.mac,6)){
            *needsend = 1;
        }        
        data_base += RTTASK_LISTUP_LEN;
    }
    //
    return build_endpage_decode(data_base,0xBF0D,host_info.mac);
}


//==========================================================================================
// 即时任务信息创建包
//==========================================================================================
uint16_t rttask_creat_build(uint16_t ran_id,uint8_t state,uint16_t task_id){
    //-----------------------------------------------------
    xtcp_tx_buf[RTTASKC_PLAY_STATE] = state;
    xtcp_tx_buf[RTTASKC_PLAY_USERID] = ran_id;
    xtcp_tx_buf[RTTASKC_PLAY_USERID+1] = ran_id>>8;
    xtcp_tx_buf[RTTASKC_PLAY_TASKID] = task_id;
    xtcp_tx_buf[RTTASKC_PLAY_TASKID+1] = task_id>>8;
    //
    return build_endpage_decode(RTTASKC_PLAY_END,RTTASK_CONTORL_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 设备 IP MAC 列表 查询回复
//==========================================================================================
uint16_t div_ipmac_list_send(){
    uint16_t data_base;
    //-----------------------------------------------------
    xtcp_tx_buf[DIVIPMAC_TOTALPACK_B] = 1;
    xtcp_tx_buf[DIVIPMAC_CURRENTPACK_B] = 0;
    xtcp_tx_buf[DIVIPMAC_TOTALDIV_B] = div_list.div_tol;
    //
    div_node_t *div_tmp_p = div_list.div_head_p;
    data_base = DIVIPMAC_DAT_BASE;    
    while(div_tmp_p!=null){
        memcpy(&xtcp_tx_buf[data_base+DIVIPMAC_DAT_MAC],div_tmp_p->div_info.mac,6);
        memcpy(&xtcp_tx_buf[data_base+DIVIPMAC_DAT_IP],div_tmp_p->div_info.ip,4);
        div_tmp_p = div_tmp_p->next_p;
        data_base += DIVIPMAC_DTA_LEN;
    }
    return build_endpage_decode(data_base,DIV_IPMAC_CHL_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 话筒 用户 查询回复
//==========================================================================================
uint16_t mic_userlist_ack_build(uint8_t state,account_all_info_t *account_all_info){
    uint16_t data_base;
    //--------------------------------------------------------------
    xtcp_tx_buf[MIC_USERCHK_STATE_B] = state;
    xtcp_tx_buf[MIC_USERCHK_USERID_B] = account_all_info->account_info.id;
    xtcp_tx_buf[MIC_USERCHK_TOL_B] = account_all_info->account_info.div_tol;

    data_base = MIC_USERCHK_DATBASE;
    for(uint8_t i=0; i<xtcp_tx_buf[MIC_USERCHK_TOL_B]; i++){
        memcpy(&xtcp_tx_buf[data_base+MIC_USERCHK_MAC],&account_all_info->mac_list[6*i],6);
        data_base +=MIC_USERCHK_END_LEN;
    }
    return build_endpage_decode(data_base,MIC_USERLIST_CHK_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 音乐文件夹名称列表 查询回复
//==========================================================================================
#define MAX_PATCHNUM_SEND   30
uint16_t music_patchlist_chk_build(){
    uint16_t data_base;
    uint8_t i;
    uint32_t *patch_tol = &tmp_union.buff[0];
    dir_info_t *dir_info = &tmp_union.buff[4];
    //
    if(g_sys_val.sd_state){
        *patch_tol = 0;
    }else{
        user_fl_get_patchlist(tmp_union.buff);
    }
    debug_printf("patch tol %d\n",*patch_tol);
    //======================================================
    if(*patch_tol == -1){
       *patch_tol=0; 
    }
    //-----------------------------------------------------
    xtcp_tx_buf[MUS_PTHCHK_PACKTOL] = *patch_tol/MAX_PATCHNUM_SEND;
    if((*patch_tol%MAX_PATCHNUM_SEND!=0)||(*patch_tol==0))
        xtcp_tx_buf[MUS_PTHCHK_PACKTOL]++;
    xtcp_tx_buf[MUS_PTHCHK_CURRENTPACK] = conn_sending_s.patchlist.pack_inc;
    //
    data_base = MUS_PTHCHK_DATBASE;
    for(i=0;(i<MAX_PATCHNUM_SEND)&&(conn_sending_s.patchlist.patch_inc < *patch_tol);i++){
        memcpy(&xtcp_tx_buf[data_base+MUS_PTHCHK_PATCHNAME],dir_info[conn_sending_s.patchlist.patch_inc].name,PATCH_NAME_NUM);
        xtcp_tx_buf[data_base+MUS_PTHCHK_PATCHMUSICTOL] = dir_info[conn_sending_s.patchlist.patch_inc].music_num;
        xtcp_tx_buf[data_base+MUS_PTHCHK_MUSICOVER_F] = dir_info[conn_sending_s.patchlist.patch_inc].music_num_full;
        /*
        if(dir_info[conn_sending_s.patchlist.patch_inc].music_num>MAX_SDCARD_MUSIC_NUM)
            xtcp_tx_buf[data_base+MUS_PTHCHK_MUSICOVER_F] = 1;
        else
            xtcp_tx_buf[data_base+MUS_PTHCHK_MUSICOVER_F] = 0;
        */
        conn_sending_s.patchlist.patch_inc++;
        data_base += MUS_PTHCHK_DAT_LEN;
    }
    xtcp_tx_buf[MUS_PTHCHK_PATCHTOL]=i;

    conn_sending_s.patchlist.pack_inc++;
    if(conn_sending_s.patchlist.pack_inc == xtcp_tx_buf[MUS_PTHCHK_PACKTOL]){
        conn_sending_s.conn_state ^= PATCH_LIST_SENDING;
        conn_sending_s.id=null;
    }
    return build_endpage_decode(data_base,MUSIC_PATCH_CHK_CMD,conn_sending_s.patchlist.id);
}

//==========================================================================================
// 音乐文名称列表 查询回复
//==========================================================================================
#define MAX_MUSICNUM_SEND   10
uint16_t music_namelist_chk_build(uint8_t state){
    uint16_t data_base;
    uint8_t i;
    uint32_t *music_tol;
    music_info_t *music_info;
    //----------------------------------------
    // 取音乐列表flash 信息
    if(state){
        user_fl_get_musiclist(conn_sending_s.musiclist.sector_index,tmp_union.buff);
        music_tol = &tmp_union.buff[0];
        music_info = &tmp_union.buff[4];
        //-------------------------------------------------------------------------
        xtcp_tx_buf[MUS_LIBCHK_PACKTOL] = *music_tol/MAX_MUSICNUM_SEND;
        if((xtcp_tx_buf[MUS_LIBCHK_PACKTOL]%MAX_MUSICNUM_SEND!=0)||(xtcp_tx_buf[MUS_LIBCHK_PACKTOL]==0))
            xtcp_tx_buf[MUS_LIBCHK_PACKTOL]++;
        xtcp_tx_buf[MUS_PTHCHK_CURRENTPACK] = conn_sending_s.musiclist.pack_inc;
    }
    else{
        *music_tol = 0;
        xtcp_tx_buf[MUS_PTHCHK_CURRENTPACK] = 0;
        xtcp_tx_buf[MUS_LIBCHK_PACKTOL]=1;
    }
    //
    memcpy(&xtcp_tx_buf[MUS_LIBCHK_PATCHNAME],&xtcp_rx_buf[MUS_LIBHCK_CHKPATCH_NAME],PATCH_NAME_NUM);
    //
    data_base = MUS_LIBCHK_DATBASE;
    for(i=0;(i<MAX_MUSICNUM_SEND)&&(conn_sending_s.musiclist.music_inc<*music_tol);i++){
        memcpy(&xtcp_tx_buf[data_base+MUS_LIBCHK_MUSICNAME],music_info[conn_sending_s.musiclist.music_inc].name,MUSIC_NAME_NUM);
        xtcp_tx_buf[data_base+MUS_LIBCHK_DURATIME] = music_info[conn_sending_s.musiclist.music_inc].totsec;
        xtcp_tx_buf[data_base+MUS_LIBCHK_DURATIME+1] = music_info[conn_sending_s.musiclist.music_inc].totsec>>8;
        conn_sending_s.musiclist.music_inc++;
        data_base += MUS_LIBCHK_DAT_LEN;
    }
    xtcp_tx_buf[MUS_LIBCHK_MUSICTOL] = i;
    //
    conn_sending_s.musiclist.pack_inc++;
    if(conn_sending_s.musiclist.pack_inc == xtcp_tx_buf[MUS_LIBCHK_PACKTOL]){
        conn_sending_s.conn_state ^= MUSICNAME_LIST_SENDING;
        conn_sending_s.id=null;
    }
    return build_endpage_decode(data_base,MUSIC_LIB_CHK_CMD,conn_sending_s.musiclist.id);
}

//==========================================================================================
// 批量处理更新 B807
//==========================================================================================
uint16_t file_batinfo_build(uint8_t *patch,uint8_t *file,uint8_t contorl,uint8_t bat_state,uint8_t state){
    xtcp_tx_buf[MUSIC_BATINFO_CONTORL] = contorl;
    xtcp_tx_buf[MUSIC_BATINFO_STATE] = bat_state;
    xtcp_tx_buf[MUSIC_BATINFO_FILESTATE] = state;
    memcpy(&xtcp_tx_buf[MUSIC_BATINFO_PATCH],patch,PATCH_NAME_NUM);
    memcpy(&xtcp_tx_buf[MUSIC_BATINFO_FILE],file,MUSIC_NAME_NUM);

    return build_endpage_decode(MUSIC_BATINFO_LEN_END,MUSIC_BAT_STATE_CMD,g_sys_val.file_bat_id);
}

//==========================================================================================
// 系统进度条回复 B805
//==========================================================================================
uint16_t file_progress_build(uint8_t state,uint8_t progress,uint8_t id[],uint8_t *name,uint8_t *patch)
{   
    xtcp_tx_buf[POL_DAT_BASE] = state;
    xtcp_tx_buf[POL_DAT_BASE+1] = progress;
    memcpy(&xtcp_tx_buf[POL_DAT_BASE+2],patch,PATCH_NAME_NUM);
    memcpy(&xtcp_tx_buf[POL_DAT_BASE+2+PATCH_NAME_NUM],name,MUSIC_NAME_NUM);
    return build_endpage_decode(POL_DAT_BASE+2+PATCH_NAME_NUM+MUSIC_NAME_NUM,MUSIC_PROCESS_BAR_CMD,id);
}



//==========================================================================================
// 系统状态在线查询回复
//==========================================================================================
uint16_t sysonline_chk_build(uint8_t state){
    xtcp_tx_buf[SYS_ONLINE_CHK_DATA_B] = g_sys_val.date_info.year;
    if(g_sys_val.date_info.month>12)
        g_sys_val.date_info.month= 1; 
    xtcp_tx_buf[SYS_ONLINE_CHK_DATA_B+1] = g_sys_val.date_info.month;
    if(g_sys_val.date_info.date>31)
        g_sys_val.date_info.date= 1;
    xtcp_tx_buf[SYS_ONLINE_CHK_DATA_B+2] = g_sys_val.date_info.date;

    xtcp_tx_buf[SYS_ONLINE_CHK_TIME_B] = g_sys_val.time_info.hour;
    xtcp_tx_buf[SYS_ONLINE_CHK_TIME_B+1] = g_sys_val.time_info.minute;
    xtcp_tx_buf[SYS_ONLINE_CHK_TIME_B+2] = g_sys_val.time_info.second;

    xtcp_tx_buf[SYS_ONLINE_CHK_SD_B] = g_sys_val.sd_state;
    //
    xtcp_tx_buf[SYS_ONLINE_CHK_DIVSTATE_B] = state;
    //
    uint16_t data_base = SYS_ONLINE_CHK_TASKID_B;
    uint16_t tol_task = 0;
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if((timetask_now.ch_state[i]!=0xFF)){
            xtcp_tx_buf[data_base] = timetask_now.task_musicplay[i].task_id;
            xtcp_tx_buf[data_base+1] = timetask_now.task_musicplay[i].task_id>>8;
            tol_task++;
            data_base+=2;
        }
    }    
    xtcp_tx_buf[SYS_ONLINE_CHK_TASKTOL_B] = tol_task;
    //
    return build_endpage_decode(data_base,ACCOUNT_SYSONLINE_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 列表更新通知
//==========================================================================================
uint16_t listinfo_upgrade_build(uint8_t type){
    xtcp_tx_buf[POL_DAT_BASE] = type;
    return build_endpage_decode(POL_DAT_BASE+1,LISTINFO_UPDATA_CMD,g_sys_val.con_id_tmp);
}

//==========================================================================================
// 定时任务更新通知
//==========================================================================================
uint16_t taskinfo_upgrade_build(task_allinfo_tmp_t *task_allinfo_tmp,uint8_t contorl,uint16_t task_id){
    debug_printf("send updata\n");
    /*
    xtcp_tx_buf[TASK_SE_CONFIG] = xtcp_rx_buf[TASK_CFG_CONTORL];
    memcpy(&xtcp_tx_buf[TASK_SE_SOLU_ID],&xtcp_rx_buf[TASK_CFG_SOLU_ID],TASK_CFG_LEN_END - TASK_CFG_SOLU_ID);
    xtcp_tx_buf[TASK_SE_TASK_ID] = id;
    xtcp_tx_buf[TASK_SE_TASK_ID+1] = id>>8;
    */
    xtcp_tx_buf[TASK_SE_CONFIG] = contorl;
    xtcp_tx_buf[TASK_SE_SOLU_ID] = task_allinfo_tmp->task_coninfo.solution_sn;
    xtcp_tx_buf[TASK_SE_TASK_ID] = task_id;
    xtcp_tx_buf[TASK_SE_TASK_ID+1] = task_id>>8;
    
    memcpy(&xtcp_tx_buf[TASK_SE_TASK_NAME],task_allinfo_tmp->task_coninfo.task_name,DIV_NAME_NUM); 
    xtcp_tx_buf[TASK_SE_TASK_STATE] = task_allinfo_tmp->task_coninfo.task_state;
    xtcp_tx_buf[TASK_SE_TASK_PRIO] = task_allinfo_tmp->task_coninfo.task_prio;
    xtcp_tx_buf[TASK_SE_TASK_VOL] = task_allinfo_tmp->task_coninfo.task_vol;
    xtcp_tx_buf[TASK_SE_REPE_MODE] = task_allinfo_tmp->task_coninfo.task_repe_mode;
    xtcp_tx_buf[TASK_SE_REPE_WEEK] = task_allinfo_tmp->task_coninfo.week_repebit;
    memcpy(&xtcp_tx_buf[TASK_SE_REPE_DATE],task_allinfo_tmp->task_coninfo.dateinfo,3*MAX_TASK_DATE_NUM);
    memcpy(&xtcp_tx_buf[TASK_SE_BEG_TIME],&task_allinfo_tmp->task_coninfo.time_info,3);
    xtcp_tx_buf[TASK_SE_DURA_TIME] =   task_allinfo_tmp->task_coninfo.dura_time/3600;
    xtcp_tx_buf[TASK_SE_DURA_TIME+1] = (task_allinfo_tmp->task_coninfo.dura_time%3600)/60;
    xtcp_tx_buf[TASK_SE_DURA_TIME+2] = (task_allinfo_tmp->task_coninfo.dura_time%3600)%60;
    xtcp_tx_buf[TASK_SE_PLAY_MODE] = task_allinfo_tmp->task_coninfo.play_mode;
    xtcp_tx_buf[TASK_SE_PLAY_TOL] = task_allinfo_tmp->task_coninfo.music_tolnum;

    xtcp_tx_buf[TASK_SE_PLAY_STATE] = 0;
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if((timetask_now.ch_state[i]!=0xFF)&&(timetask_now.task_musicplay[i].task_id==task_allinfo_tmp->task_coninfo.task_id)){
            xtcp_tx_buf[TASK_SE_PLAY_STATE] = 1;
        }
    }    

    return build_endpage_decode(TASK_SE_LEN_END,TIMETASK_UPDATA_CMD,g_sys_val.con_id_tmp);
}

//==========================================================================================
// 即时任务更新通知
//==========================================================================================
uint16_t rttaskinfo_upgrade_build(uint16_t id){
    memcpy(&xtcp_tx_buf[RTTASK_CFG_CONTORL],&xtcp_rx_buf[RTTASK_CFG_CONTORL],RTTASK_CFG_DIVTOL - RTTASK_CFG_CONTORL);
    xtcp_tx_buf[RTTASK_CFG_TASKID] = id;
    xtcp_tx_buf[RTTASK_CFG_TASKID+1] = id>>8;
    xtcp_tx_buf[RTTASK_REFRESH_STATE] = 0;
    /*
    for(uint8_t i=0;i<150;i++){
        debug_printf("%x ",xtcp_tx_buf[i]);
    }
    debug_printf("\n");
    */
    return build_endpage_decode(RTTASK_REFRESH_DATEND,RTTASK_UPDATA_CMD,g_sys_val.con_id_tmp);
}

//==========================================================================================
// 账户更新通知
//==========================================================================================
uint16_t acinfo_upgrade_build(uint16_t id){
    uint16_t adr_base = A_CONFIG_AC_DIV_MAC_B;  
    xtcp_tx_buf[A_CONFIG_CONTORL_B] = xtcp_rx_buf[A_CONFIG_CONTORL_B];
    memcpy(&xtcp_tx_buf[A_CONFIG_CONTORL_B],&xtcp_rx_buf[A_CONFIG_CONTORL_B],A_CONFIG_AC_DIV_MAC_B - A_CONFIG_CONTORL_B);
    //----------------------------------------------------------------------------------------------------------------------------------------------
    // 加密登录名
    for(uint8_t i=0;i<DIV_NAME_NUM/2;i++){
        if(xtcp_tx_buf[A_CONFIG_NAME_B+i*2]==0 && xtcp_tx_buf[A_CONFIG_NAME_B+i*2+1]==0){
            break;
        }
        xtcp_tx_buf[A_CONFIG_NAME_B+i*2] = xtcp_tx_buf[A_CONFIG_NAME_B+i*2]^g_sys_val.sn_key[i*2];
        xtcp_tx_buf[A_CONFIG_NAME_B+i*2+1] = xtcp_tx_buf[A_CONFIG_NAME_B+i*2+1]^g_sys_val.sn_key[i*2+1];
    }
    // 加密密码
    for(uint8_t i=0;i<SYS_PASSWORD_NUM-2;i++){
        xtcp_tx_buf[A_CONFIG_AC_SN_B+i] = xtcp_tx_buf[A_CONFIG_AC_SN_B+i]^g_sys_val.sn_key[i];
    }    
    //------------------------------------------------------------------------------------------------------------------------
    xtcp_tx_buf[A_CONFIG_ACNUM_B] = id;
    for(uint8_t i=0;i<xtcp_rx_buf[A_CONFIG_AC_DIVTOL_B];i++){
        memcpy(&xtcp_tx_buf[adr_base],&xtcp_rx_buf[adr_base],6);
        adr_base += 6;
    }
    return build_endpage_decode(adr_base,ACCOUNT_UPDATA_CMD,g_sys_val.con_id_tmp);
}

//==========================================================================================
// 方案更新通知
//==========================================================================================
uint16_t sulo_upgrade_build(uint8_t id){
    xtcp_tx_buf[SOLU_CFG_SOLU_ID] = id;
    xtcp_tx_buf[SOLU_CFG_SOLU_CONTORL] = xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL];
    xtcp_tx_buf[SOLU_CFG_SOLU_CONFIGBIT] = xtcp_rx_buf[SOLU_CFG_SOLU_CONFIGBIT];
    xtcp_tx_buf[SOLU_CFG_SOLU_STATE] = solution_list.solu_info[id].en;
    memcpy(&xtcp_tx_buf[SOLU_CFG_SOLU_NAME],solution_list.solu_info[id].name,DIV_NAME_NUM);
    memcpy(&xtcp_tx_buf[SOLU_CFG_SOLU_BEGDATE],&solution_list.solu_info[id].begin_date,3);
    memcpy(&xtcp_tx_buf[SOLU_CFG_SOLU_ENDDATE],&solution_list.solu_info[id].end_date,3);
    xtcp_tx_buf[SOLU_CFG_SOLU_PRIO] = solution_list.solu_info[id].prio;
    
    return build_endpage_decode(SOLU_CFG_SOLU_LEN_END,SULO_UPDATA_CMD,g_sys_val.con_id_tmp);
}

//==========================================================================================
// 同步主机IP 协议  BF07
//==========================================================================================
uint16_t sync_hostip_build(uint8_t mac[],uint8_t *ipaddr){
    xtcp_tx_buf[POL_DAT_BASE] = 0;
    memcpy(&xtcp_tx_buf[POL_DAT_BASE+1],mac,6);
    memcpy(&xtcp_tx_buf[POL_DAT_BASE+7],ipaddr,4);
    return build_endpage_decode(POL_DAT_BASE+11,SYNC_HOSTIP_CMD,g_sys_val.con_id_tmp);
}

//==========================================================================================
// 备份控制 协议  B90A
//==========================================================================================
uint16_t backup_contorl_build(uint8_t state,uint8_t *data){
    xtcp_tx_buf[POL_DAT_BASE] = state;
    //memcpy(&xtcp_tx_buf[POL_DAT_BASE+1],data,64);
    memset(&xtcp_tx_buf[POL_DAT_BASE+1],00,64);
    return build_endpage_decode(POL_DAT_BASE+1+64,BACKUP_CONTORL_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

//==========================================================================================
// 备份消息推送 协议  B90B
//==========================================================================================
uint16_t backup_updata_build(uint8_t state,uint8_t bar){
    xtcp_tx_buf[POL_DAT_BASE] = state;
    xtcp_tx_buf[POL_DAT_BASE+1] = bar;
    return build_endpage_decode(POL_DAT_BASE+2,BACKUP_UPDATA_CMD,g_sys_val.con_id_tmp);
}

//==========================================================================================
// 搜索设备列表 协议  BF09
//==========================================================================================
uint16_t divsrc_list_build(){
    uint8_t i;
    uint16_t dat_base;
    conn_sending_s.divsrc_list.pack_tol = g_sys_val.search_div_tol/10;
    if((g_sys_val.search_div_tol%10)||(g_sys_val.search_div_tol==0))
        conn_sending_s.divsrc_list.pack_tol++;
    xtcp_tx_buf[DIVSRC_LIST_TOLPACK] = conn_sending_s.divsrc_list.pack_tol;
    xtcp_tx_buf[DIVSRC_LIST_PACKNUM] = conn_sending_s.divsrc_list.pack_inc;
    dat_base = DIVSRC_LIST_DAT_BASE;
    debug_printf("div tol %d\n",g_sys_val.search_div_tol);
    for(i=0;i<10&&conn_sending_s.divsrc_list.div_inc<g_sys_val.search_div_tol;i++){
        user_divsrv_read(conn_sending_s.divsrc_list.div_inc,tmp_union.buff);
        memcpy(&xtcp_tx_buf[dat_base+DIVSRC_MAC_B],&tmp_union.buff[DIVSRC_MAC_B],6);
        memcpy(&xtcp_tx_buf[dat_base+DIVSRC_NAME_B],&tmp_union.buff[DIVSRC_NAME_B],DIV_NAME_NUM);
        xtcp_tx_buf[dat_base+DIVSRC_STATE_B] = tmp_union.buff[DIVSRC_STATE_B];
        xtcp_tx_buf[dat_base+DIVSRC_VOL_B] = tmp_union.buff[DIVSRC_VOL_B];
        memcpy(&xtcp_tx_buf[dat_base+DIVSRC_PASSWORD_B],&tmp_union.buff[DIVSRC_PASSWORD_B],SYS_PASSWORD_NUM);
        memcpy(&xtcp_tx_buf[dat_base+DIVSRC_TYPE_B],&tmp_union.buff[DIVSRC_TYPE_B],DIV_NAME_NUM);
        xtcp_tx_buf[dat_base+DIVSRC_VERSION_B] = tmp_union.buff[DIVSRC_VERSION_B];
        xtcp_tx_buf[dat_base+DIVSRC_VERSION_B+1] = tmp_union.buff[DIVSRC_VERSION_B+1];
        memcpy(&xtcp_tx_buf[dat_base+DIVSRC_HOSTIP_B],&tmp_union.buff[DIVSRC_HOSTIP_B],4);     
        debug_printf("src div %d %d %d %d\n",xtcp_tx_buf[dat_base+DIVSRC_HOSTIP_B],xtcp_tx_buf[dat_base+DIVSRC_HOSTIP_B+1],xtcp_tx_buf[dat_base+DIVSRC_HOSTIP_B+2],xtcp_tx_buf[dat_base+DIVSRC_HOSTIP_B+3]);
        conn_sending_s.divsrc_list.div_inc++;
        dat_base += DIVSRC_DATEND_B;
    }
    xtcp_tx_buf[DIVSRC_LIST_DIVTOL] = i;
    //
    conn_sending_s.divsrc_list.pack_inc++;
    if(conn_sending_s.divsrc_list.pack_inc == (conn_sending_s.divsrc_list.pack_tol)){
        conn_sending_s.id = null;
        conn_sending_s.conn_state ^=DIVSRC_LIST_SENDING;
    }
    return build_endpage_decode(dat_base,SYSSET_DIVFOUNT_CMD,conn_sending_s.divsrc_list.id);
}


//==========================================================================================
// 云服务器心跳 协议  BE00
//==========================================================================================
uint16_t cld_heart_build(){
    memcpy(&xtcp_tx_buf[CLD_HEART_IP],&host_info.ipconfig.ipaddr[0],4);
    memcpy(&xtcp_tx_buf[CLD_HEART_MAC],host_info.mac,6);
    memcpy(&xtcp_tx_buf[CLD_HEART_MASCHCODE],g_sys_val.maschine_code,10);
    xtcp_tx_buf[CLD_HEART_VER] = VERSION_H;
    xtcp_tx_buf[CLD_HEART_VER+1] = VERSION_L;
    memcpy(&xtcp_tx_buf[CLD_HEART_MAC],host_info.mac,6);
    memcpy(&xtcp_tx_buf[CLD_HEART_DIVTYPE],host_info.div_type,DIV_TYPE_NUM);
    memcpy(&xtcp_tx_buf[CLD_SYSNAME_BASE],host_info.name,DIV_TYPE_NUM);
    //
    return build_endpage_decode(CLD_HEART_DATEND,CLD_HEART_CMD,g_sys_val.con_id_tmp);
}

//==========================================================================================
// 云服务器注册查询 协议  BE01
//==========================================================================================
uint16_t cld_resigerchk_build(){
    // MAC地址
    memcpy(&xtcp_tx_buf[POL_DAT_BASE],host_info.mac,6);
    // 机器码
    memcpy(&xtcp_tx_buf[POL_DAT_BASE+6],g_sys_val.maschine_code,10);
    //
    return build_endpage_decode(POL_DAT_BASE+16,CLD_REGISTER_CHK_CMD,g_sys_val.con_id_tmp);
}


//==========================================================================================
// 云注册申请 协议  BE08
//==========================================================================================
uint16_t cld_resiger_request_build(){
    memcpy(&xtcp_tx_buf[POL_DAT_BASE],g_sys_val.maschine_code,10);
    memcpy(&xtcp_tx_buf[POL_DAT_BASE+10],g_sys_val.register_code,10);

    return build_endpage_decode(POL_DAT_BASE+20,CLD_REGISTER_REQUEST_CMD,g_sys_val.con_id_tmp);
}

//===============================================================================
// 时间同步申请 BE03
//================================================================================
uint16_t cld_timesysnc_request_build(){
    memcpy(&xtcp_tx_buf[POL_DAT_BASE],host_info.mac,6);
    //
    return build_endpage_decode(POL_DAT_BASE+20,CLD_TIMESYNC_CMD,g_sys_val.con_id_tmp);
}

//===============================================================================
// 手机申请 B90D
//================================================================================
uint16_t cld_appregsied_request_build(){
    xtcp_tx_buf[POL_DAT_BASE] = g_sys_val.register_rec_s_tmp;
    xtcp_tx_buf[POL_DAT_BASE+1] = host_info.regiser_state;
    xtcp_tx_buf[POL_DAT_BASE+2] = host_info.regiser_days;
    xtcp_tx_buf[POL_DAT_BASE+3] = host_info.regiser_days>>8; 
    return build_endpage_decode(POL_DAT_BASE+4,APP_REGISTER_CONTORL,g_sys_val.register_could_id);
}


