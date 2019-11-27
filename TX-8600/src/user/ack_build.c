#include "ack_build.h"
#include "checksum.h"
#include "sys_config_dat.h"
#include "list_contorl.h"
#include "protocol_adrbase.h"
#include "user_unti.h"
#include "fl_buff_decode.h"
#include "debug_print.h" 
#include "file_list.h"
#include "account.h"
#include "user_xccode.h"
#include "sys_log.h"
#include "user_log.h"

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

void build_endpage_forid(uint16_t len,uint8_t id[]){
	memcpy(&xtcp_tx_buf[POL_ID_BASE],id,6);
    uint16_t sum;
    sum = chksum_8bit(0,&xtcp_tx_buf[POL_LEN_BASE],(len-6));
    xtcp_tx_buf[len-4] = sum;
    xtcp_tx_buf[len-3] = sum>>8;
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

#if 0
void could_list_init(){
    //xtcp_debug_printf("cf %d\n",xtcp_rx_buf[POL_COULD_S_BASE]);
    if(xtcp_rx_buf[POL_COULD_S_BASE]){
        conn_sending_s.id = g_sys_val.could_conn.id;
    }
    else{
        conn_sending_s.id = conn.id;
    }
}
#endif

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
uint16_t div_list_resend_build(uint16_t cmd,div_node_t **div_list_p,uint8_t div_send_num,uint8_t list_num){
    uint8_t div_inc=0;
    //-----------------------------------------------------
    //dat begin
    xtcp_tx_buf[DIVLISTRE_TOTALPACK_B] = t_list_connsend[list_num].pack_tol;
    xtcp_tx_buf[DIVLISTRE_CURRENTPACK_B] = t_list_connsend[list_num].pack_inc;
    //
    uint16_t div_info_base = DIVLISTRE_INFO_B;
  
    for(uint8_t i=0; i<div_send_num; i++){
        if(*div_list_p == null)
            break;
        memcpy(&xtcp_tx_buf[div_info_base],&(*div_list_p)->div_info,DIVLISTRE_AREA_B);

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
    t_list_connsend[list_num].pack_inc++;
    #if LIST_TEXT_DEBUG
    xtcp_debug_printf("tol %d inc %d\n",t_list_connsend[list_num].pack_tol,t_list_connsend[list_num].pack_inc);
    #endif
    //----------------------------------------------------------------------------
    if(t_list_connsend[list_num].pack_inc >= t_list_connsend[list_num].pack_tol){
        t_list_connsend[list_num].conn_state = LIST_SEND_END;
    }
    return build_endpage_decode(div_info_base,cmd,t_list_connsend[list_num].could_id);
}

//============================================================================================
//分区列表协议
//============================================================================================
uint16_t area_list_send_build(uint16_t cmd,uint8_t list_num){
    uint8_t area_inc=0;
    uint16_t area_dat_base = AREAGET_DAT_BASE_B; 
    //-----------------------------------------------------
    // 获取总包数包序号
    xtcp_tx_buf[AREAGET_PACKTOTAL_B] = t_list_connsend[list_num].pack_tol;
    xtcp_tx_buf[AREAGET_CURRENTPACK_B] = t_list_connsend[list_num].pack_inc;
    //
    for(;t_list_connsend[list_num].list_info.arealist.area_inc<MAX_AREA_NUM;t_list_connsend[list_num].list_info.arealist.area_inc++){        
        if(area_info[t_list_connsend[list_num].list_info.arealist.area_inc].area_sn != 0xFFFF){
            // 获取到分区列表 信息
            uint8_t area_num=t_list_connsend[list_num].list_info.arealist.area_inc;
            xtcp_tx_buf[area_dat_base+AREAGET_AREA_SN] = area_info[area_num].area_sn;
            xtcp_tx_buf[area_dat_base+AREAGET_AREA_SN+1] = area_info[area_num].area_sn>>8;
            xtcp_tx_buf[area_dat_base+AREAGET_ACCOUNT_ID] = area_info[area_num].account_id;
            memcpy(&xtcp_tx_buf[area_dat_base+AREAGET_AREA_NAME],area_info[area_num].area_name,DIV_NAME_NUM);
            area_dat_base += AREAGET_DAT_END;
            area_inc++;
        }
        if(area_inc>AREA_SEND_NUM){
            break;
        }
    }
    xtcp_tx_buf[AREAGET_TOTALAREA_B] = area_inc;
    //
    t_list_connsend[list_num].pack_inc++;
    #if LIST_TEXT_DEBUG
    xtcp_debug_printf("tol %d inc %d\n",t_list_connsend[list_num].pack_tol,t_list_connsend[list_num].pack_inc);
    #endif
    if(t_list_connsend[list_num].pack_inc >= t_list_connsend[list_num].pack_tol){
        t_list_connsend[list_num].conn_state = LIST_SEND_END;
    }
    return build_endpage_decode(area_dat_base,cmd,t_list_connsend[list_num].could_id);
}

uint16_t area_config_ack_build(uint16_t area_sn,uint8_t state,uint8_t contorl,uint8_t fail_div_cnt){
    uint16_t dat_len=POL_DAT_BASE;
    //-----------------------------------------------------
    //dat begin
    xtcp_tx_buf[POL_DAT_BASE] =  contorl;
    xtcp_tx_buf[POL_DAT_BASE+1] =  area_sn;
    xtcp_tx_buf[POL_DAT_BASE+2] = area_sn>>8;
    xtcp_tx_buf[POL_DAT_BASE+3] = state;
    xtcp_tx_buf[POL_DAT_BASE+4] = fail_div_cnt;
    dat_len +=5;
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
    //
    xtcp_tx_buf[AC_LOGIN_DHCP_EN_B] = host_info.dhcp_en;
    //
    memcpy(&xtcp_tx_buf[AC_LOGIN_IPMASK_B],host_info.ipconfig.netmask,4);
    //
    memcpy(&xtcp_tx_buf[AC_LOGIN_IPGATE_B],host_info.ipconfig.gateway,4);
    //
    #if NO_NEED_REGISTER
    xtcp_tx_buf[AC_LOGIN_RES_STATE_B] = 2;
    #else
    xtcp_tx_buf[AC_LOGIN_RES_STATE_B] = host_info.regiser_state;
    //
    
    xtcp_tx_buf[AC_LOGIN_RES_DAY_B] = host_info.regiser_days;
    xtcp_tx_buf[AC_LOGIN_RES_DAY_B+1] = host_info.regiser_days>>8;

    /*
    if(user_id==1){
        xtcp_tx_buf[AC_LOGIN_RES_STATE_B] = 0;
    }
    else if(user_id==2){
        xtcp_tx_buf[AC_LOGIN_RES_STATE_B] = 1;
    }
    else if(user_id==3){
        xtcp_tx_buf[AC_LOGIN_RES_STATE_B] = 2;
    }
    else if(user_id==4){
        xtcp_tx_buf[AC_LOGIN_RES_STATE_B] = 3;
    }
    else if(user_id==5){
        xtcp_tx_buf[AC_LOGIN_RES_STATE_B] = 4;
    }
    else if(user_id==6){
        xtcp_tx_buf[AC_LOGIN_RES_STATE_B] = 3;
        xtcp_tx_buf[AC_LOGIN_RES_DAY_B] = 1;
        xtcp_tx_buf[AC_LOGIN_RES_DAY_B+1] = 0;

    }
    else if(user_id==7){
        xtcp_tx_buf[AC_LOGIN_RES_STATE_B] = 1;
        xtcp_tx_buf[AC_LOGIN_RES_DAY_B] = 1;
        xtcp_tx_buf[AC_LOGIN_RES_DAY_B+1] = 0;
    }

    xtcp_debug_printf("uid %d\n",user_id);
    */
    #endif

    memcpy(&xtcp_tx_buf[AC_LOGIN_SYS_MACHCODE_B],g_sys_val.maschine_code,10);

    xtcp_tx_buf[AC_LOGIN_BRAND_B]=host_info.div_brand_f;

    xtcp_tx_buf[AC_LOGIN_CLDSTATE_B]=1;
    if(g_sys_val.could_conn.id==0 && host_info.offline_day==0)
        xtcp_tx_buf[AC_LOGIN_CLDSTATE_B]=0;
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
uint16_t account_list_ack_build(uint8_t list_num){
    //
    //-----------------------------------------------------
    //dat begin
    uint16_t dat_base = AC_LISTCK_DAT_BASE;
    uint8_t tmp_inc;
    tmp_inc = t_list_connsend[list_num].list_info.ac_list.account_inc;
    //-------------------------------------------------
    // 
    uint8_t total_user = 0;
    for(; tmp_inc<MAX_ACCOUNT_NUM && total_user<MAX_SEND_ACCOUNT_NUM_FORPACK; tmp_inc++){
        if(account_info[tmp_inc].id!=0xFF){
            //xtcp_debug_printf("ac %d,%d\n",account_info[tmp_inc].id,tmp_inc);
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

            //xtcp_debug_printf("year %d mon %d day %d\n",xtcp_tx_buf[dat_base+AC_LISTCK_BUILD_DATE_B],
            //    xtcp_tx_buf[dat_base+AC_LISTCK_BUILD_DATE_B+1],xtcp_tx_buf[dat_base+AC_LISTCK_BUILD_DATE_B+2]);

            xtcp_tx_buf[dat_base+AC_LISTCK_DIV_TOL_B] = account_info[tmp_inc].div_tol;
                
            dat_base +=AC_LISTCK_DATLEN_B;
            total_user++;
        }//if 
    }//for
    
    t_list_connsend[list_num].list_info.ac_list.account_inc = tmp_inc;
    xtcp_tx_buf[AC_LISTCK_PAGENUM_B] = t_list_connsend[list_num].pack_inc;
    xtcp_tx_buf[AC_LISTCK_TOLNUM_B] = total_user;
    xtcp_tx_buf[AC_LISTCK_TOLPAGE_B] = t_list_connsend[list_num].pack_tol;

    t_list_connsend[list_num].pack_inc++;
    #if LIST_TEXT_DEBUG
    xtcp_debug_printf("tol %d inc %d\n",t_list_connsend[list_num].pack_tol,t_list_connsend[list_num].pack_inc);
    #endif
    if(t_list_connsend[list_num].pack_inc >= t_list_connsend[list_num].pack_tol){
        t_list_connsend[list_num].conn_state = LIST_SEND_END;
    }
    
    return build_endpage_decode(dat_base,ACCOUNT_USER_LIST_CMD,t_list_connsend[list_num].could_id);
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
uint16_t solution_list_ack_build(uint16_t cmd,uint8_t task_num_en){
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
        //xtcp_debug_printf("id %d %d\n", xtcp_tx_buf[data_base+SOLU_CK_ID],xtcp_tx_buf[data_base+SOLU_CK_STATE] );
        //
        xtcp_tx_buf[SOLU_CK_TOLNUM]++;
        //
        if(task_num_en){
            uint16_t tmp_num=0;
            timetask_t *timetask_p = timetask_list.all_timetask_head;
            while(timetask_p!=null){
                if(timetask_p->solu_id==solution_list.solu_info[i].id){
                    tmp_num++;
                }
                timetask_p = timetask_p->all_next_p;
            }
            xtcp_tx_buf[data_base+SOLU_CK_TASKNUM] =tmp_num;
            data_base += (SOLU_CK_LEN_END+1); 
        }
        else{
            data_base += SOLU_CK_LEN_END;
        }
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
uint16_t task_list_ack_build(uint16_t cmd,uint8_t sulo_en,uint8_t sulo_num,uint8_t list_num){
    uint8_t i;
    task_coninfo_t * tmp_p;
    //-----------------------------------------------------
    xtcp_tx_buf[TASK_CK_TOLPACK] = t_list_connsend[list_num].list_info.tasklist.task_tol;
    
    xtcp_tx_buf[TASK_CK_PACK_NUM] = t_list_connsend[list_num].pack_inc;
    //
    uint16_t data_base = TASK_CK_DAT_BASE;
    //
    i=0;
    while(t_list_connsend[list_num].list_info.tasklist.task_p!=null){
        // 是否查找指定方案
        if(sulo_en && sulo_num!=t_list_connsend[list_num].list_info.tasklist.task_p->solu_id){
            t_list_connsend[list_num].list_info.tasklist.task_p = t_list_connsend[list_num].list_info.tasklist.task_p->all_next_p;
            continue;
        }
        if(i>=MAX_TASK_ONCESEND){
            break;
        }
        i++;
        // 取flash数据
        fl_timertask_read(&g_tmp_union.task_allinfo_tmp,t_list_connsend[list_num].list_info.tasklist.task_p->id);
        tmp_p = &g_tmp_union.task_allinfo_tmp.task_coninfo;
        //
        xtcp_tx_buf[data_base+TASK_CK_SOLU_ID] = tmp_p->solution_sn; 
        xtcp_tx_buf[data_base+TASK_CK_TASK_ID] = t_list_connsend[list_num].list_info.tasklist.task_p->id;
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
        for(uint8_t j=0;j<MAX_MUSIC_CH;j++){
            if((timetask_now.ch_state[j]!=0xFF)&&(timetask_now.task_musicplay[j].task_id==tmp_p->task_id) && timetask_now.task_musicplay[j].rttask_f==0){
                xtcp_tx_buf[data_base+TASK_CK_TEXTPLAY_S] = 1;
            }
        }
        //xtcp_debug_printf("\n task s: %d \n",xtcp_tx_buf[data_base+TASK_CK_TEXTPLAY_S]);
        //
        t_list_connsend[list_num].list_info.tasklist.task_p = t_list_connsend[list_num].list_info.tasklist.task_p->all_next_p;
        data_base +=TASK_CK_LEN_END;
    }
    xtcp_tx_buf[TASK_CK_TASK_TOL] = i;
    t_list_connsend[list_num].pack_inc++;
    #if LIST_TEXT_DEBUG
    xtcp_debug_printf("tol %d inc %d\n",xtcp_tx_buf[TASK_CK_TOLPACK],t_list_connsend[list_num].pack_inc);
    #endif
    if(t_list_connsend[list_num].pack_inc>=xtcp_tx_buf[TASK_CK_TOLPACK]){
        t_list_connsend[list_num].conn_state = LIST_SEND_END;
    }
    //
    return build_endpage_decode(data_base,cmd,t_list_connsend[list_num].could_id);
}

//==========================================================================================
// 任务详细信息查看
//==========================================================================================
#define MAX_SEND_MUSIC  10

uint16_t task_dtinfo_chk_build(uint8_t list_num){
    uint16_t data_base;
    //-----------------------------------------------------
    xtcp_tx_buf[TASK_DTG_ACK_ID] = g_tmp_union.task_allinfo_tmp.task_coninfo.task_id;
    xtcp_tx_buf[TASK_DTG_ACK_ID+1] = g_tmp_union.task_allinfo_tmp.task_coninfo.task_id>>8;
    xtcp_tx_buf[TASK_DTG_TOLPACK] = g_tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum/MAX_SEND_MUSIC+1; //增加一个设备包数
    if(g_tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum%MAX_SEND_MUSIC || xtcp_tx_buf[TASK_DTG_TOLPACK]==0){
        xtcp_tx_buf[TASK_DTG_TOLPACK]++;
    }
    xtcp_tx_buf[TASK_DTG_PACK_NUM] = t_list_connsend[list_num].pack_inc; 
    //设备总数包
    if(xtcp_tx_buf[TASK_DTG_PACK_NUM]==0){
        xtcp_tx_buf[TASK_DTG_PACK_TYPE]=0;
        xtcp_tx_buf[TASK_DTG_DIV_TOL] = g_tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum;
        data_base = TASK_DTG_DIV_BASE;
        //
        for(uint8_t i=0;i<xtcp_tx_buf[TASK_DTG_DIV_TOL];i++){
            xtcp_tx_buf[data_base+TASK_DTG_DIV_AREACFG] = g_tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].zone_control;
            xtcp_tx_buf[data_base+TASK_DTG_DIV_AREACFG+1] = g_tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].zone_control>>8;
            memcpy(&xtcp_tx_buf[data_base+TASK_DTG_DIV_MAC],g_tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac,6); 
            data_base += TASK_DTG_DIV_LEN;
        }
    }
    //音乐信息包
    else{
        xtcp_tx_buf[TASK_DTG_PACK_TYPE]=1;
        data_base = TASK_DTG_DIV_BASE;
        uint8_t i;
        for(i=0; (i<MAX_SEND_MUSIC)&&(t_list_connsend[list_num].list_info.task_dtinfo.music_inc<g_tmp_union.task_allinfo_tmp.task_coninfo.music_tolnum); i++){
            //获得音乐路径名
            memcpy(&xtcp_tx_buf[data_base+TASK_DTG_MUSIC_PATH],
                    g_tmp_union.task_allinfo_tmp.task_musiclist.music_info[t_list_connsend[list_num].list_info.task_dtinfo.music_inc].music_path,PATCH_NAME_NUM);
            //获得音乐名
            memcpy(&xtcp_tx_buf[data_base+TASK_DTG_MUSIC_NAME],
                    g_tmp_union.task_allinfo_tmp.task_musiclist.music_info[t_list_connsend[list_num].list_info.task_dtinfo.music_inc].music_name,MUSIC_NAME_NUM);
            //
            t_list_connsend[list_num].list_info.task_dtinfo.music_inc++;
            data_base += TASK_DTG_MUSIC_LEN;
        }        
        xtcp_tx_buf[TASK_DTG_MUSIC_TOL] = i;
    }
    t_list_connsend[list_num].pack_inc++;
    if(t_list_connsend[list_num].pack_inc >= xtcp_tx_buf[TASK_DTG_TOLPACK]){
        t_list_connsend[list_num].conn_state = LIST_SEND_END;
    }
    return build_endpage_decode(data_base,TASK_DTINFO_CK_CMD,t_list_connsend[list_num].could_id);
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
    //xtcp_debug_printf("chk week %d\n",xtcp_tx_buf[POL_DAT_BASE]);
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
uint16_t rttask_list_chk_build(uint8_t list_num){
    uint8_t i;
    uint16_t data_base;
    //-----------------------------------------------------
    // 获得总包数
    xtcp_tx_buf[RTTASK_CK_TOLPACK] = rttask_lsit.rttask_tol/MAX_RTTASK_SEND;
    if(rttask_lsit.rttask_tol%MAX_RTTASK_SEND || xtcp_tx_buf[RTTASK_CK_TOLPACK]==0){
        xtcp_tx_buf[RTTASK_CK_TOLPACK]++;
    }
    // 当前包序号
    xtcp_tx_buf[RTTASK_CK_PACKNUM] = t_list_connsend[list_num].pack_inc;
    // 任务列表
    data_base = RTTASK_CK_BASE;
    //
    for(i=0;(i<MAX_RTTASK_SEND);i++){
        if(t_list_connsend[list_num].list_info.rttasklist.rttask_p==null)
            break;
        // 获得即时任务详细信息
        fl_rttask_read(&g_tmp_union.rttask_dtinfo,t_list_connsend[list_num].list_info.rttasklist.rttask_p->rttask_id);
        // 获得账户ID
        xtcp_tx_buf[data_base+RTTASK_CK_ACID] = g_tmp_union.rttask_dtinfo.account_id;
        // 获得即时任务ID
        xtcp_tx_buf[data_base+RTTASK_CK_TASKID] = g_tmp_union.rttask_dtinfo.rttask_id;
        xtcp_tx_buf[data_base+RTTASK_CK_TASKID+1] = g_tmp_union.rttask_dtinfo.rttask_id>>8;
        // 获得任务名称
        memcpy(&xtcp_tx_buf[data_base+RTTASK_CK_TASKNAME],g_tmp_union.rttask_dtinfo.name,DIV_NAME_NUM);
        // 播放源MAC
        memcpy(&xtcp_tx_buf[data_base+RTTASK_CK_SRCMAC],g_tmp_union.rttask_dtinfo.src_mas,6);
        // 保留 播放优先级
        xtcp_tx_buf[data_base+RTTASK_CK_TASKPRIO]=g_tmp_union.rttask_dtinfo.prio;
        // 任务音量
        xtcp_tx_buf[data_base+RTTASK_CK_TASKVOL] = g_tmp_union.rttask_dtinfo.task_vol;
        // 持续时间
        if(g_tmp_union.rttask_dtinfo.dura_time==0xFFFFFFFF){
            xtcp_tx_buf[data_base+RTTASK_CK_DURATIME] = 0xFF;
            xtcp_tx_buf[data_base+RTTASK_CK_DURATIME+1] = 0xFF;
            xtcp_tx_buf[data_base+RTTASK_CK_DURATIME+2] = 0xFF;
        }
        else{
            xtcp_tx_buf[data_base+RTTASK_CK_DURATIME] = g_tmp_union.rttask_dtinfo.dura_time/3600;
            xtcp_tx_buf[data_base+RTTASK_CK_DURATIME+1] = (g_tmp_union.rttask_dtinfo.dura_time%3600)/60;
            xtcp_tx_buf[data_base+RTTASK_CK_DURATIME+2] = (g_tmp_union.rttask_dtinfo.dura_time%3600)%60;
        }
        // 遥控按键
        xtcp_tx_buf[data_base+RTTASK_CK_KEYINFO] = g_tmp_union.rttask_dtinfo.task_key;
        // 任务状态
        //-----------------------------------------------------------------
        rttask_info_t *tmp_p = rttask_lsit.run_head_p;
        xtcp_tx_buf[data_base+RTTASK_CK_STATE] = 0;
        while(tmp_p!=null){
            if(tmp_p->rttask_id == g_tmp_union.rttask_dtinfo.rttask_id){
                xtcp_tx_buf[data_base+RTTASK_CK_STATE] = tmp_p->run_state;
                break;
            }
            tmp_p = tmp_p->run_next_p;
        }
        //-----------------------------------------------------------------
        t_list_connsend[list_num].list_info.rttasklist.rttask_p = t_list_connsend[list_num].list_info.rttasklist.rttask_p->all_next_p;
        data_base += RTTASK_CK_LEN;
    }
    xtcp_tx_buf[RTTASK_CK_TASKTOL] = i;
    t_list_connsend[list_num].pack_inc++;
    #if LIST_TEXT_DEBUG
	xtcp_debug_printf("tol %d inc %d\n", xtcp_tx_buf[RTTASK_CK_TOLPACK],t_list_connsend[list_num].pack_inc);
    #endif
    if(t_list_connsend[list_num].pack_inc >= xtcp_tx_buf[RTTASK_CK_TOLPACK]){
        t_list_connsend[list_num].conn_state = LIST_SEND_END;
    }
    return build_endpage_decode(data_base,RTTASK_CHECK_CMD,t_list_connsend[list_num].could_id);
}

//==========================================================================================
// 即时任务详细信息查询
//==========================================================================================
uint16_t rttask_dtinfo_chk_build(uint16_t id){
    uint16_t data_base;
    //-----------------------------------------------------
    fl_rttask_read(&g_tmp_union.rttask_dtinfo,id);
    //
    xtcp_tx_buf[RTTASK_DTCK_ACKID] = id;
    xtcp_tx_buf[RTTASK_DTCK_ACKID+1] = id>>8;
    if(g_tmp_union.rttask_dtinfo.div_tol>100)
        g_tmp_union.rttask_dtinfo.div_tol = 100;
    xtcp_tx_buf[RTTASK_DTCK_DIVTOL] = g_tmp_union.rttask_dtinfo.div_tol;
    data_base = RTTASK_DTCK_DATBASE;
    for(uint8_t i=0;i<xtcp_tx_buf[RTTASK_DTCK_DIVTOL];i++){
        //设备MAC
        memcpy(&xtcp_tx_buf[data_base+RTTASK_DTCK_DIVMAC],g_tmp_union.rttask_dtinfo.des_info[i].mac,6);
        //分区控制标志
        xtcp_tx_buf[data_base+RTTASK_DTCK_AREACONTORL] = g_tmp_union.rttask_dtinfo.des_info[i].zone_control;
        xtcp_tx_buf[data_base+RTTASK_DTCK_AREACONTORL+1] = g_tmp_union.rttask_dtinfo.des_info[i].zone_control>>8;
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
        memset(&g_tmp_union.rttask_dtinfo,0x00,sizeof(rttask_dtinfo_t));
    }
    // 获取任务详细信息
    else{
        fl_rttask_read(&g_tmp_union.rttask_dtinfo,id);
    }
    // 任务建立与关闭
    xtcp_tx_buf[RTTASK_BUILD_CONTORL] = contorl;
    //
    xtcp_tx_buf[RTTASK_BUILD_ID] = g_tmp_union.rttask_dtinfo.rttask_id;
    xtcp_tx_buf[RTTASK_BUILD_ID+1] = g_tmp_union.rttask_dtinfo.rttask_id>>8;
    // 随机ID
    xtcp_tx_buf[RTTASK_BUILD_CONID] = ran_id;
    xtcp_tx_buf[RTTASK_BUILD_CONID+1] = ran_id>>8;
    // 任务优先级 保留
    xtcp_tx_buf[RTTASK_BUILD_PRIO] = g_tmp_union.rttask_dtinfo.prio;
    // 任务音量
    xtcp_tx_buf[RTTASK_BUILD_VOL] = g_tmp_union.rttask_dtinfo.task_vol;
    // 持续时间
    xtcp_tx_buf[RTTASK_BUILD_DURATIME] = g_tmp_union.rttask_dtinfo.dura_time/3600;
    xtcp_tx_buf[RTTASK_BUILD_DURATIME+1] = (g_tmp_union.rttask_dtinfo.dura_time%3600)/60;
    xtcp_tx_buf[RTTASK_BUILD_DURATIME+2] = (g_tmp_union.rttask_dtinfo.dura_time%3600)%60;
    // 设备总量
    xtcp_tx_buf[RTTASK_BUILD_DIVTOL] = g_tmp_union.rttask_dtinfo.div_tol;
    //
    data_base = RTTASK_BUILD_BASE;

    for(uint8_t i=0; i<g_tmp_union.rttask_dtinfo.div_tol; i++){
        //取得设备指针
        div_tmp_p = get_div_info_p(g_tmp_union.rttask_dtinfo.des_info[i].mac);
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
        xtcp_tx_buf[data_base+RTTASK_BUILD_AREACONTORL] = g_tmp_union.rttask_dtinfo.des_info[i].zone_control;
        xtcp_tx_buf[data_base+RTTASK_BUILD_AREACONTORL+1] = g_tmp_union.rttask_dtinfo.des_info[i].zone_control>>8;
        //设备MAC
        memcpy(&xtcp_tx_buf[data_base+RTTASK_BUILD_MAC],g_tmp_union.rttask_dtinfo.des_info[i].mac,6);
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
    fl_rttask_read(&g_tmp_union.rttask_dtinfo,id);
    // 设备总量
    xtcp_tx_buf[RTTASK_LISTUP_DIVTOL] = g_tmp_union.rttask_dtinfo.div_tol;
    //
    data_base = RTTASK_LISTUP_LIST_BASE;
    *needsend = 0;
    for(uint8_t i=0; i<g_tmp_union.rttask_dtinfo.div_tol; i++){
        //取得设备指针
        div_tmp_p = get_div_info_p(g_tmp_union.rttask_dtinfo.des_info[i].mac);
        if(div_tmp_p==null){
            xtcp_tx_buf[RTTASK_LISTUP_DIVTOL]--;
            continue;
        }
        
        //获得设备IP
        memcpy(&xtcp_tx_buf[data_base+RTTASK_LISTUP_IP],div_tmp_p->div_info.ip,4);
        //分区控制位
        xtcp_tx_buf[data_base+RTTASK_LISTUP_AREACONTORL] = g_tmp_union.rttask_dtinfo.des_info[i].zone_control;
        xtcp_tx_buf[data_base+RTTASK_LISTUP_AREACONTORL+1] = g_tmp_union.rttask_dtinfo.des_info[i].zone_control>>8;
        //设备MAC
        memcpy(&xtcp_tx_buf[data_base+RTTASK_LISTUP_MAC],g_tmp_union.rttask_dtinfo.des_info[i].mac,6);
        //
        //判断是否需要更新
        #if 0
        xtcp_debug_printf("%x,%x,%x,%x,%x,%x\n%x,%x,%x,%x,%x,%x\n",tmp_union.rttask_dtinfo.des_info[i].mac[0],tmp_union.rttask_dtinfo.des_info[i].mac[1],
                                                              tmp_union.rttask_dtinfo.des_info[i].mac[2],tmp_union.rttask_dtinfo.des_info[i].mac[3],
                                                              tmp_union.rttask_dtinfo.des_info[i].mac[4],tmp_union.rttask_dtinfo.des_info[i].mac[5],
                                                              rttask_div_p->div_info.mac[0],rttask_div_p->div_info.mac[1],rttask_div_p->div_info.mac[2],
                                                              rttask_div_p->div_info.mac[3],rttask_div_p->div_info.mac[4],rttask_div_p->div_info.mac[5]);
        #endif
        if(charncmp(g_tmp_union.rttask_dtinfo.des_info[i].mac,rttask_div_p->div_info.mac,6)){
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
uint16_t music_patchlist_chk_build(uint8_t list_num){
    uint16_t data_base;
    uint8_t i;
    uint32_t *patch_tol = &g_tmp_union.buff[0];
    dir_info_t *dir_info = &g_tmp_union.buff[4];
    //
    if(g_sys_val.sd_state){
        *patch_tol = 0;
    }else{
        user_fl_get_patchlist(g_tmp_union.buff);
    }
    //xtcp_debug_printf("patch tol %d\n",*patch_tol);
    //======================================================
    if(*patch_tol == -1){
       *patch_tol=0; 
    }
    //-----------------------------------------------------
    xtcp_tx_buf[MUS_PTHCHK_PACKTOL] = *patch_tol/MAX_PATCHNUM_SEND;
    if(*patch_tol%MAX_PATCHNUM_SEND || xtcp_tx_buf[MUS_PTHCHK_PACKTOL]==0)
        xtcp_tx_buf[MUS_PTHCHK_PACKTOL]++;
	//
    xtcp_tx_buf[MUS_PTHCHK_CURRENTPACK] = t_list_connsend[list_num].pack_inc;
    //
    data_base = MUS_PTHCHK_DATBASE;
    for(i=0;(i<MAX_PATCHNUM_SEND)&&(t_list_connsend[list_num].list_info.patchlist.patch_inc < *patch_tol);i++){
        memcpy(&xtcp_tx_buf[data_base+MUS_PTHCHK_PATCHNAME],dir_info[t_list_connsend[list_num].list_info.patchlist.patch_inc].name,PATCH_NAME_NUM);
        xtcp_tx_buf[data_base+MUS_PTHCHK_PATCHMUSICTOL] = dir_info[t_list_connsend[list_num].list_info.patchlist.patch_inc].music_num;
        xtcp_tx_buf[data_base+MUS_PTHCHK_MUSICOVER_F] = dir_info[t_list_connsend[list_num].list_info.patchlist.patch_inc].music_num_full;
        /*
        if(dir_info[conn_sending_s.patchlist.patch_inc].music_num>MAX_SDCARD_MUSIC_NUM)
            xtcp_tx_buf[data_base+MUS_PTHCHK_MUSICOVER_F] = 1;
        else
            xtcp_tx_buf[data_base+MUS_PTHCHK_MUSICOVER_F] = 0;
        */
        t_list_connsend[list_num].list_info.patchlist.patch_inc++;
        data_base += MUS_PTHCHK_DAT_LEN;
    }
    xtcp_tx_buf[MUS_PTHCHK_PATCHTOL]=i;

    t_list_connsend[list_num].pack_inc++;
    if(t_list_connsend[list_num].pack_inc >= xtcp_tx_buf[MUS_PTHCHK_PACKTOL]){
        t_list_connsend[list_num].conn_state = LIST_SEND_END;
    }
    return build_endpage_decode(data_base,MUSIC_PATCH_CHK_CMD,t_list_connsend[list_num].could_id);
}

//==========================================================================================
// 音乐文名称列表 查询回复
//==========================================================================================
#define MAX_MUSICNUM_SEND   10
uint16_t music_namelist_chk_build(uint8_t state,uint8_t list_num){
    uint16_t data_base;
    uint8_t i;
    uint32_t *music_tol;
    music_info_t *music_info;
    //---------------------------------------------------------------
    // 取音乐列表flash 信息
    if(state){
        user_fl_get_musiclist(t_list_connsend[list_num].list_info.musiclist.sector_index,g_tmp_union.buff);
        music_tol = &g_tmp_union.buff[0];
        music_info = &g_tmp_union.buff[4];
        //-------------------------------------------------------------------------
        xtcp_tx_buf[MUS_LIBCHK_PACKTOL] = *music_tol/MAX_MUSICNUM_SEND;
        if(*music_tol%MAX_MUSICNUM_SEND || xtcp_tx_buf[MUS_LIBCHK_PACKTOL]==0)
            xtcp_tx_buf[MUS_LIBCHK_PACKTOL]++;
        xtcp_tx_buf[MUS_PTHCHK_CURRENTPACK] = t_list_connsend[list_num].pack_inc;
    }
    //-----------------------
    else{
        *music_tol = 0;
        xtcp_tx_buf[MUS_PTHCHK_CURRENTPACK] = 0;
        xtcp_tx_buf[MUS_LIBCHK_PACKTOL]=1;
    }
    //----------------------------------------------------------------
    memcpy(&xtcp_tx_buf[MUS_LIBCHK_PATCHNAME],t_list_connsend[list_num].list_info.musiclist.music_patch_name,PATCH_NAME_NUM);
    //
    data_base = MUS_LIBCHK_DATBASE;
    for(i=0;(i<MAX_MUSICNUM_SEND)&&(t_list_connsend[list_num].list_info.musiclist.music_inc<*music_tol);){
        uint8_t getname_flag=1;
        if(host_info.wav_mode==0){
            for(uint8_t i=0;i<MUSIC_NAME_NUM-4;i++){
                if(music_info[t_list_connsend[list_num].list_info.musiclist.music_inc].name[i]==0x2E &&
                   music_info[t_list_connsend[list_num].list_info.musiclist.music_inc].name[i+1]==0x77 &&
                   music_info[t_list_connsend[list_num].list_info.musiclist.music_inc].name[i+2]==0x61 &&
                   music_info[t_list_connsend[list_num].list_info.musiclist.music_inc].name[i+3]==0x76){
                       getname_flag=0;
                       break;
                   }
            }
        }
        if(getname_flag){
            memcpy(&xtcp_tx_buf[data_base+MUS_LIBCHK_MUSICNAME],music_info[t_list_connsend[list_num].list_info.musiclist.music_inc].name,MUSIC_NAME_NUM);
            xtcp_tx_buf[data_base+MUS_LIBCHK_DURATIME] = music_info[t_list_connsend[list_num].list_info.musiclist.music_inc].totsec;
            xtcp_tx_buf[data_base+MUS_LIBCHK_DURATIME+1] = music_info[t_list_connsend[list_num].list_info.musiclist.music_inc].totsec>>8;
            data_base += MUS_LIBCHK_DAT_LEN;
            i++;
        }
        t_list_connsend[list_num].list_info.musiclist.music_inc++;
    }
    xtcp_tx_buf[MUS_LIBCHK_MUSICTOL] = i;
    //
    t_list_connsend[list_num].pack_inc++;
    #if LIST_TEXT_DEBUG
    xtcp_debug_printf("tol %d inc %d\n",xtcp_tx_buf[MUS_LIBCHK_PACKTOL],t_list_connsend[list_num].pack_inc);
    #endif
    if(t_list_connsend[list_num].pack_inc >= xtcp_tx_buf[MUS_LIBCHK_PACKTOL]){
        t_list_connsend[list_num].conn_state = LIST_SEND_END;
    }
    return build_endpage_decode(data_base,MUSIC_LIB_CHK_CMD,t_list_connsend[list_num].could_id);
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
    // 日志更新
    //log_musicfile_config();
    return build_endpage_decode(MUSIC_BATINFO_LEN_END,MUSIC_BAT_STATE_CMD,g_sys_val.file_bat_id);
}

//==========================================================================================
// sd卡容量查询 B809
//==========================================================================================
uint16_t sdcard_sizechk_build(){
    unsigned tol_mb,free_mb;
    user_getsdcard_state(&tol_mb,&free_mb);
    xtcp_tx_buf[SDCARD_CHK_STATE] = g_sys_val.sd_state;
    memcpy(&xtcp_tx_buf[SDCARD_CHK_TOLMB],&tol_mb,4);
    memcpy(&xtcp_tx_buf[SDCARD_CHK_FREEMB],&free_mb,4);
    //
    return build_endpage_decode(SDCARD_CHK_LEN,SDCARD_SIZECHK_B809_CMD,&xtcp_rx_buf[POL_ID_BASE]);
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

    uint8_t tmp = 1;
    if(g_sys_val.could_conn.id==0 && host_info.offline_day==0)
        tmp=0;

    xtcp_tx_buf[SYS_ONLINE_CHK_SD_B] = (g_sys_val.sd_state&0x01) | ((tmp&0x01)<<1);
    //
    xtcp_tx_buf[SYS_ONLINE_CHK_DIVSTATE_B] = (state);
    //
    uint16_t data_base = SYS_ONLINE_CHK_TASKID_B;
    uint16_t tol_task = 0;
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if((timetask_now.ch_state[i]!=0xFF)&&(timetask_now.task_musicplay[i].rttask_f==0)){
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
    //xtcp_debug_printf("send updata\n");
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
        if((timetask_now.ch_state[i]!=0xFF)&&(timetask_now.task_musicplay[i].task_id==task_allinfo_tmp->task_coninfo.task_id)&&(timetask_now.task_musicplay[i].rttask_f==0)){
            xtcp_tx_buf[TASK_SE_PLAY_STATE] = 1;
        }
    }    

    return build_endpage_decode(TASK_SE_LEN_END,TIMETASK_UPDATA_CMD,g_sys_val.con_id_tmp);
}

//==========================================================================================
// 即时任务更新通知
//==========================================================================================
uint16_t rttaskinfo_upgrade_build(uint16_t id,uint16_t contorl){
    uint8_t task_state=0;
    
    fl_rttask_read(&g_tmp_union.rttask_dtinfo,id);
    //
    // 查找任务状态
    rttask_info_t *runtmp_p = rttask_lsit.run_head_p;
    while(runtmp_p!=null){
        //比较任务id
        if(runtmp_p->rttask_id == id){ 
            task_state = runtmp_p->run_state;
            break;
        }
        runtmp_p = runtmp_p->run_next_p;
    }
    xtcp_tx_buf[RTTASK_CFG_CONTORL] = contorl;

    xtcp_tx_buf[RTTASK_CFG_ACID] = g_tmp_union.rttask_dtinfo.account_id;
    //
    xtcp_tx_buf[RTTASK_CFG_TASKID] = id;
    xtcp_tx_buf[RTTASK_CFG_TASKID+1] = id>>8;
    memcpy(&xtcp_tx_buf[RTTASK_CFG_TASKNAME],g_tmp_union.rttask_dtinfo.name,DIV_NAME_NUM);
    memcpy(&xtcp_tx_buf[RTTASK_CFG_SRCMAC],g_tmp_union.rttask_dtinfo.src_mas,6);
    xtcp_tx_buf[RTTASK_CFG_TASKPRIO] = g_tmp_union.rttask_dtinfo.prio;
    xtcp_tx_buf[RTTASK_CFG_TASKVOL] = g_tmp_union.rttask_dtinfo.task_vol;
    // 持续时间
    // 任务持续时间
    if(g_tmp_union.rttask_dtinfo.dura_time==0xFFFFFFFF){
        xtcp_tx_buf[RTTASK_CFG_DURATIME] = 0xFF;
        xtcp_tx_buf[RTTASK_CFG_DURATIME+1] = 0xFF;
        xtcp_tx_buf[RTTASK_CFG_DURATIME+2] = 0xFF;
    }
    else{
        xtcp_tx_buf[RTTASK_CFG_DURATIME] = g_tmp_union.rttask_dtinfo.dura_time/3600;
        xtcp_tx_buf[RTTASK_CFG_DURATIME+1] = (g_tmp_union.rttask_dtinfo.dura_time%3600)/60;
        xtcp_tx_buf[RTTASK_CFG_DURATIME+2] = (g_tmp_union.rttask_dtinfo.dura_time%3600)%60;
    }
    xtcp_tx_buf[RTTASK_CFG_KETINFO] = g_tmp_union.rttask_dtinfo.task_key;
    xtcp_tx_buf[RTTASK_CFG_DIVTOL] = g_tmp_union.rttask_dtinfo.div_tol;
    //
    #if 0 // 不需更新播放列表
    uint16_t data_base = RTTASK_CFG_DIV_BASE;
    for(uint8_t i=0;i<xtcp_tx_buf[RTTASK_CFG_DIVTOL];i++){
        memcpy(&xtcp_tx_buf[data_base + RTTASK_CFG_MAC],tmp_union.rttask_dtinfo.des_info[i].mac,6); 
        xtcp_tx_buf[data_base + RTTASK_CFG_AREACONTORL] = tmp_union.rttask_dtinfo.des_info[i].zone_control;
        xtcp_tx_buf[data_base + RTTASK_CFG_AREACONTORL+1] = tmp_union.rttask_dtinfo.des_info[i].zone_control>>8;
        data_base+=RTTASK_CFG_LEN;
    }
    #endif
    
    xtcp_tx_buf[RTTASK_REFRESH_STATE] = task_state;
    /*
    for(uint8_t i=0;i<150;i++){
        xtcp_debug_printf("%x ",xtcp_tx_buf[i]);
    }
    xtcp_debug_printf("\n");
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
// 同步主机IP 协议  BF0B
//==========================================================================================
uint16_t sync_ipinfo_build(uint8_t mac[]){
    xtcp_tx_buf[SYSIP_STATE] = 0;
    memcpy(&xtcp_tx_buf[SYSIP_DESMAC],mac,6);
    xtcp_tx_buf[SYSIP_SETFLAG]=3;
    memcpy(&xtcp_tx_buf[SYSIP_HOSTIP],host_info.ipconfig.ipaddr,4);
    xtcp_tx_buf[SYSIP_DHCPEN]=1;
    
    return build_endpage_decode(SYSIP_DATLEN,SYSSET_IPSET_CMD,g_sys_val.con_id_tmp);
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
uint16_t divsrc_list_build(uint8_t list_num){
    uint8_t i;
    uint16_t dat_base;
    t_list_connsend[list_num].pack_tol = g_sys_val.search_div_tol/10;
    if(g_sys_val.search_div_tol%10 || t_list_connsend[list_num].pack_tol==0)
        t_list_connsend[list_num].pack_tol++;
    //
    xtcp_tx_buf[DIVSRC_LIST_TOLPACK] = t_list_connsend[list_num].pack_tol;
    xtcp_tx_buf[DIVSRC_LIST_PACKNUM] = t_list_connsend[list_num].pack_inc;
    dat_base = DIVSRC_LIST_DAT_BASE;
    //xtcp_debug_printf("div tol %d\n",g_sys_val.search_div_tol);
    for(i=0;i<10&&t_list_connsend[list_num].list_info.divsrc_list.div_inc<g_sys_val.search_div_tol;i++){
        user_divsrv_read(t_list_connsend[list_num].list_info.divsrc_list.div_inc,g_tmp_union.buff);
        memcpy(&xtcp_tx_buf[dat_base+DIVSRC_MAC_B],&g_tmp_union.buff[DIVSRC_MAC_B],6);
        memcpy(&xtcp_tx_buf[dat_base+DIVSRC_NAME_B],&g_tmp_union.buff[DIVSRC_NAME_B],DIV_NAME_NUM);
        xtcp_tx_buf[dat_base+DIVSRC_STATE_B] = g_tmp_union.buff[DIVSRC_STATE_B];
        xtcp_tx_buf[dat_base+DIVSRC_VOL_B] = g_tmp_union.buff[DIVSRC_VOL_B];
        memcpy(&xtcp_tx_buf[dat_base+DIVSRC_PASSWORD_B],&g_tmp_union.buff[DIVSRC_PASSWORD_B],SYS_PASSWORD_NUM);
        memcpy(&xtcp_tx_buf[dat_base+DIVSRC_TYPE_B],&g_tmp_union.buff[DIVSRC_TYPE_B],DIV_NAME_NUM);
        xtcp_tx_buf[dat_base+DIVSRC_VERSION_B] = g_tmp_union.buff[DIVSRC_VERSION_B];
        xtcp_tx_buf[dat_base+DIVSRC_VERSION_B+1] = g_tmp_union.buff[DIVSRC_VERSION_B+1];
        memcpy(&xtcp_tx_buf[dat_base+DIVSRC_HOSTIP_B],&g_tmp_union.buff[DIVSRC_HOSTIP_B],4);     
        //xtcp_debug_printf("src div %d %d %d %d\n",xtcp_tx_buf[dat_base+DIVSRC_HOSTIP_B],xtcp_tx_buf[dat_base+DIVSRC_HOSTIP_B+1],xtcp_tx_buf[dat_base+DIVSRC_HOSTIP_B+2],xtcp_tx_buf[dat_base+DIVSRC_HOSTIP_B+3]);
        t_list_connsend[list_num].list_info.divsrc_list.div_inc++;
        dat_base += DIVSRC_DATEND_B;
    }
    xtcp_tx_buf[DIVSRC_LIST_DIVTOL] = i;
    //
    t_list_connsend[list_num].pack_inc++;
    if(t_list_connsend[list_num].pack_inc >= t_list_connsend[list_num].pack_tol){
        t_list_connsend[list_num].conn_state = LIST_SEND_END;
    }
    return build_endpage_decode(dat_base,SYSSET_DIVFOUNT_CMD,t_list_connsend[list_num].could_id);
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

//===============================================================================
// 页面查看 B312 
//================================================================================
uint16_t taskview_page_build(uint16_t cmd){
    uint16_t tmp_num;
    div_node_t *div_info_p = div_list.div_head_p;
    rttask_info_t *rttask_info_p = rttask_lsit.all_head_p;
    timetask_t *timetask_p = timetask_list.all_timetask_head;
    char c_tx_8623[]={'T',0,'X',0,'-',0,'8',0,'6',0,'2',0,'3',0};
    //----------------------------------------------------------------------
    tmp_num = 0;
    while(rttask_info_p!=null){
        tmp_num++;
        rttask_info_p = rttask_info_p->all_next_p;
    }
    xtcp_tx_buf[POL_DAT_BASE] = tmp_num; //即时任务总数
    //----------------------------------------------------------------------
    tmp_num = 0;
    while(timetask_p!=null){
        if(timetask_p->solu_id==0xFF){
            tmp_num++;
        }
        timetask_p = timetask_p->all_next_p;
    }
    xtcp_tx_buf[POL_DAT_BASE+1] = tmp_num; //定时任务总数
    //xtcp_debug_printf("timetask num %d\n",tmp_num);
    //----------------------------------------------------------------------
    tmp_num = 0;
    for(uint8_t i=0;i<MAX_TASK_SOULTION;i++){
        if(solution_list.solu_info[i].state!=0xFF){
            tmp_num++;
        }
    }
    xtcp_tx_buf[POL_DAT_BASE+2] = tmp_num; //方案总数
    //----------------------------------------------------------------------
    tmp_num=0;
    while(div_info_p!=null){
		#if 0
		for(uint8_t i=0;i<14;i++){
			xtcp_debug_printf("%x ",c_tx_8623[i]);
		}
		xtcp_debug_printf("\n");

		for(uint8_t i=0;i<14;i++){
			xtcp_debug_printf("%x ",div_info_p->div_info.div_type[i]);
		}
		xtcp_debug_printf("\n");
		#endif
        if(charncmp(c_tx_8623,div_info_p->div_info.div_type,14)){
            tmp_num++;
        }
        div_info_p = div_info_p->next_p;
    }
    xtcp_tx_buf[POL_DAT_BASE+3] = tmp_num; //报警设备总数
    //xtcp_debug_printf("firediv num %d\n",tmp_num);
    //---------------------------------------------------------------------
    return build_endpage_decode(POL_DAT_BASE+4,cmd,&xtcp_rx_buf[POL_ID_BASE]);
}

//===============================================================================
// 查看收发包数量 C001
//===============================================================================
/*
uint16_t chk_txpage_cnt_build(){
	unsigned tmp_cnt;
	user_get_txpage_cnt(&tmp_cnt);
	xtcp_tx_buf[TEXT_TXCNT_STATE] = 1; //应答
	memcpy(&xtcp_tx_buf[TEXT_TXCNT_MAC],host_info.mac,6);
	memcpy(&xtcp_tx_buf[TEXT_TXCNT_FORTX_CNT],&tmp_cnt,4);
	memset(&xtcp_tx_buf[TEXT_TXCNT_FORRX_CNT],0x00,24);
	
	return build_endpage_decode(TEXT_TXCNT_DATEND,TEXT_TXPAGE_GET_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}
*/
//===============================================================================
// DNS 查询
//===============================================================================
uint16_t dns_couldip_chk_build(){
    uint16_t data_base_len;
    char could_domain[] = {0x05,0x79,0x75,0x6E,0x62,0x6F,0x06,0x69,0x74,0x63,0x2D,0x70,0x61,0x02,0x63,0x6E,0x00};
    
    xtcp_tx_buf[DNS_IDENTIFICATION] = 0;
    xtcp_tx_buf[DNS_IDENTIFICATION+1] = 0;
    xtcp_tx_buf[DNS_CODE_FLAG] = 01;
    xtcp_tx_buf[DNS_CODE_FLAG+1] = 0;
    xtcp_tx_buf[DNS_QUESTION] = 0;
    xtcp_tx_buf[DNS_QUESTION+1] = 01;

    xtcp_tx_buf[DNS_ANSWERS] = 0;
    xtcp_tx_buf[DNS_ANSWERS+1] = 0;
    xtcp_tx_buf[DNS_AUTHORITY] = 0;
    xtcp_tx_buf[DNS_AUTHORITY+1] = 0;
    xtcp_tx_buf[DNS_ADDITIONAL] = 0;
    xtcp_tx_buf[DNS_ADDITIONAL+1] = 0;

    data_base_len = DNS_DAT_BASE;
    // 获取域名
    memcpy(&xtcp_tx_buf[data_base_len],could_domain,sizeof(could_domain));
    //
    data_base_len += sizeof(could_domain);
    // TYPE
    xtcp_tx_buf[data_base_len]=0;
    data_base_len++;
    xtcp_tx_buf[data_base_len]=1;
    data_base_len++;
    //Class
    xtcp_tx_buf[data_base_len]=0;
    data_base_len++;
    xtcp_tx_buf[data_base_len]=1;
    data_base_len++;
    //
    return  data_base_len;
}


uint16_t music_batrechk_build(){
    memset(&xtcp_tx_buf[POL_DAT_BASE],0,4);
    for(uint8_t i=0;i<MAX_BATCONTORL_OBJ_NUM;i++){
        if(charncmp(g_sys_val.bat_contorlobj[i].bat_id,&xtcp_rx_buf[POL_MAC_BASE],6)){
            xtcp_tx_buf[POL_DAT_BASE] = g_sys_val.bat_contorlobj[i].bat_state;
            xtcp_tx_buf[POL_DAT_BASE+1] = g_sys_val.bat_contorlobj[i].succeed_num;
            xtcp_tx_buf[POL_DAT_BASE+2] = g_sys_val.bat_contorlobj[i].fail_num;
            xtcp_tx_buf[POL_DAT_BASE+3] = g_sys_val.bat_contorlobj[i].remain_num;
            // 进度恢复
            if(g_sys_val.bat_contorlobj[i].bat_state){
                g_sys_val.file_bat_conn = conn;
            }
        }
    }
    return build_endpage_decode(POL_DAT_BASE+4,MUSIC_B807_BATRECHK_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

// 获取指定IP列表
uint16_t divlist_ipchk_ack_build(){
    uint8_t tol_num;
    div_node_t *div_tmp_p;
    tol_num = xtcp_rx_buf[POL_DAT_BASE];
    xtcp_tx_buf[POL_DAT_BASE] = tol_num;
    uint16_t dat_base=POL_DAT_BASE+1;
    for(uint8_t i=0;i<tol_num;i++){
        memcpy(&xtcp_tx_buf[dat_base],&xtcp_rx_buf[dat_base],6);
        //找设备
        div_tmp_p = get_div_info_p(&xtcp_rx_buf[dat_base]);
        dat_base+=6;
        if(div_tmp_p==null){
            xtcp_tx_buf[dat_base] = 0;
            dat_base++;
            memset(&xtcp_tx_buf[dat_base],0x00,4);
            dat_base +=4;
        }
        else{
            xtcp_tx_buf[dat_base]= div_tmp_p->div_info.div_state;
            dat_base++;
            memcpy(&xtcp_tx_buf[dat_base],div_tmp_p->div_info.ip,4);
            dat_base +=4;
        }
    }
    return build_endpage_decode(dat_base,DIVLIST_IPCHK_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

uint16_t udp_trainsmit_headrt_build(){
    // MAC
    memcpy(&xtcp_tx_buf[POL_DAT_BASE],host_info.mac,6);
    memset(&xtcp_tx_buf[POL_DAT_BASE+6],0x00,64);
	return build_endpage_decode(POL_DAT_BASE+6+64,APP_AUDTRAINSMIT_UDP_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

uint16_t rttask_muslist_chk_build(uint8_t list_num){
    uint8_t i=0;
    fl_rttask_read(&g_tmp_union.rttask_dtinfo,t_list_connsend[list_num].list_info.rttaskmusic_ilst.task_id);

    xtcp_tx_buf[RTTASK_MUCLISTCHK_ID] = t_list_connsend[list_num].list_info.rttaskmusic_ilst.task_id;
    xtcp_tx_buf[RTTASK_MUCLISTCHK_ID+1] = t_list_connsend[list_num].list_info.rttaskmusic_ilst.task_id>>8;
    xtcp_tx_buf[RTTASK_MUCLISTCHK_PACKTOL] = g_tmp_union.rttask_dtinfo.music_tol/MAX_SEND_MUSIC;
    if(g_tmp_union.rttask_dtinfo.music_tol%MAX_SEND_MUSIC || xtcp_tx_buf[RTTASK_MUCLISTCHK_PACKTOL]==0){
        xtcp_tx_buf[RTTASK_MUCLISTCHK_PACKTOL]++;
    }
    xtcp_tx_buf[RTTASK_MUCLISTCHK_PACKINC]=t_list_connsend[list_num].pack_inc;
    uint16_t dat_base=RTTASK_MUCLISTCHK_DATBASE;
    
    //xtcp_debug_printf("get mus %d\n",g_tmp_union.rttask_dtinfo.music_tol);
    
    for(;t_list_connsend[list_num].list_info.rttaskmusic_ilst.music_inc<g_tmp_union.rttask_dtinfo.music_tol && i<MAX_SEND_MUSIC ;
        t_list_connsend[list_num].list_info.rttaskmusic_ilst.music_inc++,i++){
        //序号
        xtcp_tx_buf[dat_base+RTTASK_MUCLISTCHK_NUM] = t_list_connsend[list_num].list_info.rttaskmusic_ilst.music_inc+1;
        // 路径名
        memcpy(&xtcp_tx_buf[dat_base+RTTASK_MUCLISTCHK_PATCH],
        g_tmp_union.rttask_dtinfo.music_info[t_list_connsend[list_num].list_info.rttaskmusic_ilst.music_inc].music_path,PATCH_NAME_NUM);
        // 歌曲名
        memcpy(&xtcp_tx_buf[dat_base+RTTASK_MUCLISTCHK_MUSNAME],
        g_tmp_union.rttask_dtinfo.music_info[t_list_connsend[list_num].list_info.rttaskmusic_ilst.music_inc].music_name,MUSIC_NAME_NUM);
        //         
        dat_base += RTTASK_MUCLISTCHK_DATLEN;
    }
    xtcp_tx_buf[RTTASK_MUCLISTCHK_MUSTOL] = i;

    //xtcp_debug_printf("sends mus %d\n",t_list_connsend[list_num].list_info.rttaskmusic_ilst.music_inc);    
    //
    t_list_connsend[list_num].pack_inc++;

    if(t_list_connsend[list_num].pack_inc >= xtcp_tx_buf[RTTASK_MUCLISTCHK_PACKTOL]){
        t_list_connsend[list_num].conn_state = LIST_SEND_END;
    }
    return build_endpage_decode(dat_base,RTTASK_MUSICLIST_CHKCMD,t_list_connsend[list_num].could_id);
    
}

uint16_t rttask_infosend_build(uint8_t list_num,uint8_t ch){
    fl_rttask_read(&g_tmp_union.rttask_dtinfo,timetask_now.task_musicplay[ch].task_id);

    xtcp_tx_buf[RTTASK_INFO_TASKID] = rttask_info_list[list_num].task_id;
    xtcp_tx_buf[RTTASK_INFO_TASKID+1] = rttask_info_list[list_num].task_id>>8;
    
    xtcp_debug_printf("\n\n info send id %d\n\n",rttask_info_list[list_num].task_id);

    
    xtcp_debug_printf("\n\n info send mac %x %x %x %x %x %x \n\n",host_info.mac[0],host_info.mac[1],host_info.mac[2],host_info.mac[3],host_info.mac[4],host_info.mac[5]);

    xtcp_tx_buf[RTTASK_INFO_USERID] = rttask_info_list[list_num].user_id;
    xtcp_tx_buf[RTTASK_INFO_USERID+1] = rttask_info_list[list_num].user_id>>8;

    memcpy(&xtcp_tx_buf[RTTASK_INFO_MAC],host_info.mac,6);

    memcpy(&xtcp_tx_buf[RTTASK_INFO_DIVTYPE],host_info.div_type,DIV_TYPE_NUM);

    xtcp_tx_buf[RTTASK_INFO_VOL] = timetask_now.task_musicplay[ch].task_vol;

    xtcp_tx_buf[RTTASK_INFO_MUSICTOL] = g_tmp_union.rttask_dtinfo.music_tol;
    xtcp_tx_buf[RTTASK_INFO_MUSICTOL+1] = 0;

    xtcp_tx_buf[RTTASK_INFO_MUSICNUM] = timetask_now.task_musicplay[ch].music_inc+1;
    xtcp_tx_buf[RTTASK_INFO_MUSICNUM+1] = 0; 

    xtcp_tx_buf[RTTASK_INFO_PACKINFO] = 0;

    xtcp_tx_buf[RTTASK_INFO_MUSICTOLSEC]= timetask_now.task_musicplay[ch].music_tolsec/60/60;//时
    xtcp_tx_buf[RTTASK_INFO_MUSICTOLSEC+1]= (timetask_now.task_musicplay[ch].music_tolsec/60)%60;//分
    xtcp_tx_buf[RTTASK_INFO_MUSICTOLSEC+2]= timetask_now.task_musicplay[ch].music_tolsec%60;//秒

    xtcp_tx_buf[RTTASK_INFO_MUSPLAYSEC] = timetask_now.task_musicplay[ch].music_sec/60/60;
    xtcp_tx_buf[RTTASK_INFO_MUSPLAYSEC+1] = (timetask_now.task_musicplay[ch].music_sec/60)%60;//分
    xtcp_tx_buf[RTTASK_INFO_MUSPLAYSEC+2] = timetask_now.task_musicplay[ch].music_sec%60;//秒

    xtcp_tx_buf[RTTASK_INFO_SAVETYPE]=0;

    xtcp_tx_buf[RTTASK_INFO_PLAYSTATE] = 2;
    if(timetask_now.task_musicplay[ch].play_state)
        xtcp_tx_buf[RTTASK_INFO_PLAYSTATE] = 1;

    xtcp_tx_buf[RTTASK_INFO_CDSTATE] = 6;
    if(g_sys_val.rttask_musicset_f[ch]){
        xtcp_tx_buf[RTTASK_INFO_CDSTATE]=7;
    }

    xtcp_tx_buf[RTTASK_INFO_MUSTYPE] = 4;

    switch(timetask_now.task_musicplay[ch].play_mode){
        case ORDER_PLAY_M:
            xtcp_tx_buf[RTTASK_INFO_PLAYMODE] = 0;
            break;
        case LOOP_PLAY_M:
            xtcp_tx_buf[RTTASK_INFO_PLAYMODE] = 2;
            break;
        case RANDOM_PLAY_M:
            xtcp_tx_buf[RTTASK_INFO_PLAYMODE] = 5;
            break;
        case ONCE_PLAY_M:
            xtcp_tx_buf[RTTASK_INFO_PLAYMODE] = 4;
            break;
        case ONCE_LOOPPLAY_M:
            xtcp_tx_buf[RTTASK_INFO_PLAYMODE] = 1;
            break;
    }
    
    xtcp_tx_buf[RTTASK_INFO_NAMETYPE]=0;

    xtcp_tx_buf[RTTASK_INFO_NAMELEN] = MUSIC_NAME_NUM;

    for(uint8_t i=0;i<MUSIC_NAME_NUM/2;i++){
        xtcp_tx_buf[RTTASK_INFO_NAMEDAT+i*2] = g_tmp_union.rttask_dtinfo.music_info[timetask_now.task_musicplay[ch].music_inc].music_name[i*2+1];
        xtcp_tx_buf[RTTASK_INFO_NAMEDAT+i*2+1]= g_tmp_union.rttask_dtinfo.music_info[timetask_now.task_musicplay[ch].music_inc].music_name[i*2];
    }

    return build_endpage_decode(RTTASK_INFO_DATLEN,RTTASK_INFOSEND_CMD,rttask_info_list[list_num].could_id);
}

uint16_t divtext_send_build(){
    xtcp_tx_buf[TEXTDIV_SEND_STATE] = 0;
    memcpy(&xtcp_tx_buf[TEXTDIV_SEND_MAC],&xtcp_rx_buf[TEXTDIV_REC_DIVMAC],6);
    return build_endpage_decode(TEXTDIV_SEND_MAC_DATLEN,BE0E_DIV_TEXTCOMMOND_CMD,&xtcp_rx_buf[POL_ID_BASE]);
}

