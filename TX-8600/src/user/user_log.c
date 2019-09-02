#include "user_log.h"
#include "account.h"
#include "debug_print.h"
#include "user_xccode.h"
#include "user_unti.h"
#include "task_decode.h"
#include "fl_buff_decode.h"

//=============================================================================================================
// 账号日志
//=============================================================================================================

// 局域网登录
void log_account_login(){
    g_sys_val.log_info_p = log_info_chang("account login:%l \r\r\n",&xtcp_rx_buf[A_LOGIN_NAME_B]);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr);
}
// 云登录
void log_account_couldlogin(uint8_t id){
    g_sys_val.log_info_p = log_info_chang("account login phone:%l \r\r\n",account_info[id].phone_num);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr);
}
// 账号编辑
void log_account_config(uint8_t id){
    g_sys_val.log_info_p = log_info_chang("account config config:%d 0=add 1=config 2=del\r\n    name:%l\r\n    phonenum:%l\r\n    sn:%l\r\n   limited:%d 0=admin 1=normal\r\r\n",
                                            xtcp_rx_buf[A_CONFIG_CONTORL_B],
                                            account_info[id].name,account_info[id].phone_num,account_info[id].sn,account_info[id].type);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr);
}

//=============================================================================================================
// 设备信息日志
//===============================================================================================================

//分区编辑
void log_divarea_config(){
    g_sys_val.log_info_p = log_info_chang("area config \r\r\n config:%d 0=add 1=del 2=config\r\n    name:%l divtol:%d \r\r\n",  
                                            xtcp_rx_buf[AREASET_CONFIG_BYE_B],
                                            &xtcp_rx_buf[AREASET_AREA_NAME_B],xtcp_rx_buf[AREASET_DIV_TOL_B]);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr);

}
// 设备上线
void log_divonline(div_node_t *div_info_p){
    g_sys_val.log_info_p = log_info_chang("div online name:%l\r\r\n",div_info_p->div_info.name);
    user_loginfo_add(div_info_p->div_info.mac,div_info_p->div_info.ip);
    
}
// 设备下线
void log_divoffline(div_node_t *div_info_p){
    g_sys_val.log_info_p = log_info_chang("div off type:%l name:%l\r\r\n",div_info_p->div_info.div_type,div_info_p->div_info.name);
    user_loginfo_add(div_info_p->div_info.mac,div_info_p->div_info.ip);
}
// 主机信息配置
void log_hostinfo_config(){
    g_sys_val.log_info_p = log_info_chang("hostinfo config  name:%l  dhcp:%d\r  mask:%d.%d.%d.%d\r  gate:%d.%d.%d.%d\r\r\n", 
                                            host_info.name,host_info.dhcp_en,
                                            host_info.ipconfig.netmask[0],host_info.ipconfig.netmask[1],host_info.ipconfig.netmask[2],host_info.ipconfig.netmask[3],
                                            host_info.ipconfig.gateway[0],host_info.ipconfig.gateway[1],host_info.ipconfig.gateway[2],host_info.ipconfig.gateway[3]);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr);
}

// 删除设备
void log_div_del(div_node_t *div_info_p){
    g_sys_val.log_info_p = log_info_chang("div del type:%l name:%l\r\r\n",div_info_p->div_info.div_type,div_info_p->div_info.name);
    user_loginfo_add(div_info_p->div_info.mac,div_info_p->div_info.ip);    
}

//=============================================================================================================
// 音乐库操作日志信息
//=============================================================================================================
// 音乐文件夹信息改变
void log_musicpatch_config(){
    g_sys_val.log_info_p = log_info_chang("music patch\r\r\n config:%d 0=add 1=del 2=config\r oldname:%l  newname:%l\r\r\n", 
                                        xtcp_rx_buf[MUS_PTHCON_CONFIG],&xtcp_rx_buf[MUS_PTHCON_SRCNAME],&xtcp_rx_buf[MUS_PTHCON_DESNAME]);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr);    
}

// 音乐文件信息改变
void log_musicfile_config(){
    g_sys_val.log_info_p = log_info_chang("music file bat conifg\r  config:%d 0=copy 1=move 2=del 3=stop\r\r\n", 
                                        g_sys_val.file_bat_contorl);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr);    

    /*
    g_sys_val.log_info_p = log_info_chang("music file conifg  mac:%x-%x-%x-%x-%x-%x  config:%d 0=copy 1=move 2=del\r patch:%l  name:%l\r\r\n",    
                                        g_sys_val.file_bat_id[0],g_sys_val.file_bat_id[1],g_sys_val.file_bat_id[2],
                                        g_sys_val.file_bat_id[3],g_sys_val.file_bat_id[4],g_sys_val.file_bat_id[5],
                                        xtcp_tx_buf[MUSIC_BATINFO_FILESTATE],&xtcp_tx_buf[MUSIC_BATINFO_PATCH],&xtcp_tx_buf[MUSIC_BATINFO_FILE]);
    user_loginfo_add();    
    */
}


//=============================================================================================================
// 即时任务日志信息
//=============================================================================================================

// 即时任务控制 开启/停止
void log_rttask_runingstate(uint16_t id){
    fl_rttask_read(&g_tmp_union.rttask_dtinfo,id);
    g_sys_val.log_info_p = log_info_chang("rttask contorl:%d 0=stop 1=start name:%l duratime:%d-second\r\r\n", 
                                        xtcp_rx_buf[RTTASK_PLAY_CONTORL],g_tmp_union.rttask_dtinfo.name,g_tmp_union.rttask_dtinfo.dura_time);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr); 
}
// 即时任务时间结束
void log_rttask_timeover(){
    g_sys_val.log_info_p = log_info_chang("rttask timeover name:l duratime:%d-second\r\r\n",g_tmp_union.rttask_dtinfo.name,g_tmp_union.rttask_dtinfo.dura_time);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr);   
}

// 即时任务编辑
void log_rttask_config(){
    g_sys_val.log_info_p = log_info_chang("rttask config\r config:%d 0=add 1=del 2=config\r name:%l duratime:%d-second prio:%d 00=normal 10=impoartant 20=emerngecy\r source div mac:%x-%x-%x-%x-%x-%x,divtol:%d\r\r\n",    
                                    xtcp_rx_buf[RTTASK_CFG_CONTORL],
                                    g_tmp_union.rttask_dtinfo.name,g_tmp_union.rttask_dtinfo.dura_time,
                                    g_tmp_union.rttask_dtinfo.src_mas[0],g_tmp_union.rttask_dtinfo.src_mas[1],g_tmp_union.rttask_dtinfo.src_mas[2],
                                    g_tmp_union.rttask_dtinfo.src_mas[3],g_tmp_union.rttask_dtinfo.src_mas[4],g_tmp_union.rttask_dtinfo.src_mas[5],g_tmp_union.rttask_dtinfo.div_tol);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr); 
}

//=============================================================================================================
// 定时打铃任务日志信息
//=============================================================================================================
// 定时打铃任务控制 开启/停止
void log_timetask_contorl(){
    g_sys_val.log_info_p = log_info_chang("timetask contorl:%d 0=start 1=stop 2=config  name:%l\r\r\n", 
                                    xtcp_rx_buf[TASK_PLAY_CONTORL],g_tmp_union.task_allinfo_tmp.task_coninfo.task_name);
    xtcp_debug_printf("timetask contorl:%d 0=start 1=stop 2=config  name:%l\r\r\n", 
                                    xtcp_rx_buf[TASK_PLAY_CONTORL],g_tmp_union.task_allinfo_tmp.task_coninfo.task_name);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr); 
}

// 定时打铃任务编辑
void log_timetask_config(uint8_t state){
    g_sys_val.log_info_p = log_info_chang("timetask config\r config:%d 0=add 2=del 3=config 5=bat name:%l\r\r\n", 
                                state,g_tmp_union.task_allinfo_tmp.task_coninfo.task_name);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr); 
}

// 今日任务配置
void log_todaytask_config(){
    g_sys_val.log_info_p = log_info_chang("todaytask config day:%d\r\r\n",
                                g_sys_val.today_date);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr); 
}

// 方案配置
void log_solu_config(){
    g_sys_val.log_info_p = log_info_chang("solution config config:%d 0=add 1=del 2=config 3=copy name:%l prio:%d en%d 0=disable 1=enable\r\r\n", 
                            xtcp_rx_buf[SOLU_CFG_SOLU_CONTORL],&xtcp_rx_buf[SOLU_CFG_SOLU_NAME],xtcp_rx_buf[SOLU_CFG_SOLU_PRIO],xtcp_rx_buf[SOLU_CFG_SOLU_STATE]);
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr); 
}

//云连接
void log_could_online(){
    g_sys_val.log_info_p = log_info_chang("colud connect online\r\r\n");
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr); 
}

//云断开
void log_could_offline(){
    g_sys_val.log_info_p = log_info_chang("colud connect offline\r\r\n");
    user_loginfo_add(&xtcp_rx_buf[POL_ID_BASE],conn.remote_addr); 
}


