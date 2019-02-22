#include "list_instance.h"
#include "account.h"
#include "task_decode.h"
#include "divlist_decode.h"
#include "protocol_adrbase.h"
#include "task_decode.h"
#include "user_file_contorl.h"
#include "list_contorl.h"

//================================================================================
// fun list decode 
//  命令处理函数
rec_fun_lis_t rec_fun_lis[]={{DIV_HEART_CMD,div_heart_recive},
                             {APP_CONNECTHAND_CMD,app_sysonline_recive},
                             {ONLINE_REQUEST_CMD,div_online_recive},
                             {DIVLIST_REQUEST_CMD,divlist_request_recive},
                             {DIV_EXTRA_INFO_CMD,div_extra_info_recive},
                             {DIV_INFO_SET_CMD,div_info_set_recive},
                             {AREA_GETREQUEST_CMD,arealist_request_rec},
                             {AREA_CONFIG_CMD,area_config_recive},
                             {SOLUTION_CHECK_CMD,solution_check_recive},
                             {TASK_CHECK_CMD,task_check_recive},
                             {TASK_DTINFO_CK_CMD,task_dtinfo_check_recive},
                             {SOLUTION_CONFIG_CMD,solution_config_recive},
                             //{TASK_CONFIG_CMD,task_config_recive},
                             {TASK_DIINFO_CONFIG_CMD,task_dtinfo_config_recive},
                             {TASK_PLAYTEXT_CMD,task_playtext_recive},
                             {TASK_TODAYWEEK_CK_CMD,today_week_check_recive},
                             {TASK_CONFIG_WEEK_CMD,today_week_config_recive},
                             {RTTASK_CHECK_CMD,rttask_list_check_recive},
                             {RTTASK_DTINFO_CHECK_CMD,rttask_dtinfo_check_recive},
                             {RTTASK_CONFIG_CMD,rttask_config_recive},
                             {RTTASK_CONTORL_CMD,rttask_contorl_recive},
                             {RTTASK_BUILD_CMD,rttask_build_recive},
                             {DIV_IPMAC_CHL_CMD,div_ip_mac_check_recive},
                             {MIC_USERLIST_CHK_CMD,mic_userlist_chk_recive},
                             {HOST_SEARCH_CMD,user_host_search_recive},
                             {MUSIC_PATCH_CHK_CMD,music_patch_list_chk_recive},
                             {MUSIC_LIB_CHK_CMD,music_music_list_chk_recive},
                             {MUSIC_PATCHNAME_CON_CMD,music_patchname_config_recive},
                             {MUSIC_BUSY_CHK_CMD,music_busy_chk_recive},
                             {MUSIC_FILE_CONTORL_CMD,music_file_config_recive},
                             {MUSIC_PROCESS_BAR_CMD,musicfile_bar_chk_recive},
                             {RTTASK_REBUILD_CMD,task_rttask_rebuild},
                             {MIC_AUX_REQUEST_CMD,mic_aux_request_recive},
                             {MIC_AUX_CLOSE_CMD,mic_aux_heart_recive},
                             {TASK_BAT_CONFIG_CMD,task_bat_config_recive},
                             {TASK_EN_CONFIG_CMD,task_en_recive},
                             {MUSIC_BAT_CONTORL_CMD,music_bat_contorl_recive},
                             {MUSIC_BAT_STATE_CMD,music_bat_info_recive},
                             {ACCOUNT_LOGIN_CMD,account_login_recive},
                             {ACCOUNT_USER_LIST_CMD,account_userlist_recive},
                             {ACCOUNT_DIV_LIST_CMD,account_div_list_recive},
                             {ACCOUNT_CONFIG_CMD,account_config_recive},
                             {ACCOUNT_SYSONLINE_CMD,account_sysonline_recive},
                             {USER_TIMER_SYNC_CMD,user_timer_sync_recive},
 							 {BACKUP_BUSY_CHK_CMD,backup_busy_chk},
                             {BACKUP_UPDATA_CMD,backup_mes_send_recive},
                             {BACKUP_CONTORL_CMD,backup_contorl_chk},
                             {TMP_IPSET_CMD,tmp_ipset_recive},
                             {TASK_BAT_DIVSET_CMD,bat_task_divset_recive},
                             {CLD_REGISTER_RECIVE_CMD,account_sys_register_recive},
                             {CLD_CLOULDLOGIN_CMD,cld_account_login_recive},
                             {APP_REGISTER_CONTORL,app_register_request},
                             {CLD_REGISTER_REQUEST_CMD,cld_register_recive},
                             {LAN_DIVRESEARCH_CMD,research_lan_revice},
                             {SYSSET_DIVFOUNT_CMD,sysset_divfound_recive},
                             {SYSSET_DIV_HOSTSET_CMD,divresearch_hostset_recive},
                             {CLD_TIMER_SYNC_CMD,cld_timer_sync_recive},
                             {SYSSET_IPSET_CMD,sysset_ipset_recive}
                            };

// 多包列表发送函数
sending_fun_lis_t sending_fun_lis[]={div_sending_decode,         //设备列表
                                     arealist_sending_decode,    //分区列表
                                     tasklist_sending_decode,    //定时任务列表
                                     task_dtinfo_decode,         //定时任务详细信息列表
                                     rttask_list_sending_decode, //即时任务列表传送
                                     music_patch_list_send_decode,  //
                                     music_music_list_send_decode,  //
                                     ac_list_sending_decode,        //
                                     divsrc_sending_decode,         //
                                    };
// 多包发送状态
conn_sending_s_t conn_sending_s;
    
//---------------------------------------------------------------------------
uint16_t fun_list_len;
//
void init_funlist_len(){
    fun_list_len = (sizeof(rec_fun_lis))/(sizeof(rec_fun_lis_t));
}
//===============================================================================

//================================================================================
// list instance
//================================================================================
// xtcp conn list 
conn_list_t conn_list[MAX_UDP_CONNET];
//connect_list_t connect_list;
conn_long_list_t conn_long_list;
//-----------------------------------------
// div list
div_list_t div_list;
//------------------------------------------
// host info
host_info_t host_info = {0};
host_info_t host_info_tmp = {
                         {0x42,0x4C,0x45,0x00,0x00,0x00},//mac
                         {0x9A,0x5B,0xF6,0x65,0x68,0x56,0x3B,0x4E,0x3A,0x67,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}, //NAME
                         {0x31,0x00,0x32,0x00,0x33,0x00,0x34,0x00,0x35,0x00,0x36,0x00,00,00},  //sn  123456 
                         {VERSION_H,VERSION_L}, //version
                         {0x54,0x00,0x58,0x00,0x2D,0x00,0x38,0x00,0x36,0x00,0x30,0x00,0x30,0x00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}, //TX-8600
                         0x00,    //AUX_TYPE
                         0x00,    //host mode
                         0x00,    //slient en
                         0x00,    //slient lv
                         0x00,    //dhcp
                         0x00,    //regiser state
                         0x00,  //regiser day
                         {//172,16,13,119,//ip
                          172,16,13,112,//ip
                          255,255,255,0,//netmask
                          172,16,13,254}, //gateway 
                         {0x49,0x00,0x54,0x00,0x43,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,00,00,00,00,00,00,00,00,00,00}, //BRAND
                         //-----------------------------------------------------------------------------------------------------------------------                   
                        };
//----------------------------------------------------
//  area info
area_info_t area_info[MAX_AREA_NUM];
//----------------------------------------------------
// account list info
account_info_t account_info[MAX_ACCOUNT_NUM];
//----------------------------------------------------
// 任务方案
solution_list_t solution_list;
//--------------------------------------------------------------------------------
// 定时任务列表
timetask_list_t timetask_list;

// 音乐通道播放状态
timetask_now_t timetask_now;

// 即时任务链表
rttask_lsit_t rttask_lsit = {0};
// 即时任务启动连接建立状态
rttask_build_state_t rttask_build_state[MAX_RTTASK_CONTORL_NUM];

//================================================================================
