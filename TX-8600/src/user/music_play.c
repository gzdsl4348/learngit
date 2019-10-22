#include "music_play.h"
#include "user_unti.h"
#include "user_xccode.h"
#include "sys_config_dat.h"
#include "fl_buff_decode.h"
#include "list_contorl.h"
#include "eth_audio.h"
#include "eth_audio_config.h"
#include "debug_print.h"
#include "sys_log.h"
#include "task_decode.h"
#include "user_lcd.h"

audio_txlist_t t_audio_txlist;

extern uint8_t f_name[];

void task_music_send(uint8_t ch,taskmac_info_t *p_taskmac_info,uint8_t div_tol,uint8_t task_prio,uint8_t task_vol){
    div_node_t *div_tmp_p;
    t_audio_txlist.num_info=0;
    // 配置发送目标
    for(uint8_t i=0;i<div_tol;i++){
        /*
        //查找目标设备与IP
        xtcp_debug_printf("des mca: %x,%x,%x,%x,%x,%x\n",p_taskmac_info[i].mac[0],
                                                p_taskmac_info[i].mac[1],
                                                p_taskmac_info[i].mac[2],
                                                p_taskmac_info[i].mac[3],
                                                p_taskmac_info[i].mac[4],
                                                p_taskmac_info[i].mac[5]);
        */
        div_tmp_p = get_div_info_p(p_taskmac_info[i].mac);
        if(div_tmp_p==null){
            //i++;
            continue;
        }
        //if(div_tmp_p->div_info.div_state == 00)
        //    continue;
        //xtcp_debug_printf("div ok\n");
        //
        //获得mac
        memcpy(t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac,p_taskmac_info[i].mac,6);
        //获得IP
        memcpy(t_audio_txlist.t_des_info[t_audio_txlist.num_info].ip,div_tmp_p->div_info.ip,4);
        //获得分区控制位
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].area_contorl = p_taskmac_info[i].zone_control;
        t_audio_txlist.num_info++;
        //
    }
    
    // text    
    #if 0
    t_audio_txlist.num_info = 2; 
    for(uint8_t i=0;i<1;i++){
        if(i==0){
            t_audio_txlist.t_des_info[0].mac[0] = 0x42;
            t_audio_txlist.t_des_info[0].mac[1] = 0x4C;
            t_audio_txlist.t_des_info[0].mac[2] = 0x45;
            t_audio_txlist.t_des_info[0].mac[3] = 0x00;
            t_audio_txlist.t_des_info[0].mac[4] = 0x72;
            t_audio_txlist.t_des_info[0].mac[5] = 0x45;

            t_audio_txlist.t_des_info[0].ip[0] = 172;
            t_audio_txlist.t_des_info[0].ip[1] = 16;
            t_audio_txlist.t_des_info[0].ip[2] = 23;
            t_audio_txlist.t_des_info[0].ip[3] = 112;
        }
        else{
            t_audio_txlist.t_des_info[i].mac[0] = 0x10;
            t_audio_txlist.t_des_info[i].mac[1] = 0x7B;
            t_audio_txlist.t_des_info[i].mac[2] = 0x44;
            t_audio_txlist.t_des_info[i].mac[3] = 0x51;
            t_audio_txlist.t_des_info[i].mac[4] = 0x2C;
            t_audio_txlist.t_des_info[i].mac[5] = 0x30;

            t_audio_txlist.t_des_info[i].ip[0] = 172;
            t_audio_txlist.t_des_info[i].ip[1] = 16;
            t_audio_txlist.t_des_info[i].ip[2] = 23;
            t_audio_txlist.t_des_info[i].ip[3] = 243;

        }
    }
    #endif
    /*
    if(ch==1){
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[0] = 0x42;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[1] = 0x4C;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[2] = 0x45;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[3] = 0x00;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[4] = 0x0A;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[5] = 0x07;

        t_audio_txlist.t_des_info[t_audio_txlist.num_info].ip[0] = 172;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].ip[1] = 16;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].ip[2] = 13;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].ip[3] = 107;
        
        t_audio_txlist.num_info++;
    }
    */
    // 配置优先级
    g_sys_val.audio_type[ch] = task_prio;
    //g_sys_val.audio_type[ch] = 0x11;
    //i_ethaud_cfg->set_audio_type(g_sys_val.audio_type);
    set_audio_type(g_sys_val.audio_type);
    // 配置目标
    user_audio_desip_set(ch,task_prio);
    // 配置音量
    set_audio_vol(ch,task_vol);
    // 发送使能
    user_audio_senden(ch);
}

void task_music_stop(uint8_t ch){
    user_music_stop(ch);
    // 发送禁止
    user_audio_send_dis(ch);
    //xtcp_debug_printf("music stop\n");
}


void task_music_play(uint8_t ch,uint8_t num,task_music_info_t *p_music_info){
    xtcp_debug_printf("\n\n\nmusic play \n\n");
    
    timetask_now.task_musicplay[ch].play_state = 1;
    user_rttask_musname_put(p_music_info,ch);
    //获取歌曲路径名
    uint8_t i,j,ch_tmp;
    uint8_t task_id_right=0;
    // 判断是否有空位显示任务
    for(uint8_t i=0;i<MAX_DISP_TASK;i++){
        if(g_sys_val.disp_ch[i]==ch){
            task_id_right=1; // 有空位显示任务
            ch_tmp = i;
            break;
        }
    }
    /*
    for(i=0;i<(PATCH_NAME_NUM/2);i++){
        if(((uint16_t *)p_music_info->music_path)[i]==0)
            break;
        ((uint16_t *)f_name)[i] = ((uint16_t *)p_music_info->music_path)[i];
    }
    ((uint16_t *)f_name)[i] = 0x002F;
    i++;s
    // 更新曲目名
    for(j=0; j<(MUSIC_NAME_NUM/2); i++,j++){
        if(((uint16_t *)p_music_info->music_name)[j]==0)
            break;
        ((uint16_t *)f_name)[i] = ((uint16_t *)p_music_info->music_name)[j];
    }
    ((uint16_t *)f_name)[i] = 0x00;
    */
    //
    if(task_id_right){     
        uint8_t playing_char[]={0x6B,0x63,0x57,0x28,0x64,0xAD,0x65,0x3E,0x00,0x3A};
        memcpy(g_sys_val.disinfo2buf[ch_tmp],playing_char,10);
        uint8_t data_base=10;
        for(j=0; j<(MUSIC_NAME_NUM/2);j++){
            g_sys_val.disinfo2buf[ch_tmp][data_base+j*2] = p_music_info->music_name[j*2+1];
            g_sys_val.disinfo2buf[ch_tmp][data_base+j*2+1] = p_music_info->music_name[j*2];
            if(g_sys_val.disinfo2buf[ch_tmp][data_base+j*2+1]==0 && g_sys_val.disinfo2buf[ch_tmp][data_base+j*2]==0)
                goto music_dispend;
            if(j>14)
                break;
        }   
        data_base+=(12*2);
        g_sys_val.disinfo2buf[ch_tmp][data_base] = 00;
        g_sys_val.disinfo2buf[ch_tmp][data_base+1] = 0x2E;
        g_sys_val.disinfo2buf[ch_tmp][data_base+2] = 00;
        g_sys_val.disinfo2buf[ch_tmp][data_base+3] = 0x2E;  
        g_sys_val.disinfo2buf[ch_tmp][data_base+4] = 00;
        g_sys_val.disinfo2buf[ch_tmp][data_base+5] = 0x2E;
        g_sys_val.disinfo2buf[ch_tmp][data_base+6] =0x00;
        g_sys_val.disinfo2buf[ch_tmp][data_base+7] =0x00;
    }
    music_dispend:
    #if 0
    for(i=0;i<(MUSIC_NAME_NUM+PATCH_NAME_NUM)/2;i++){
        if(((uint16_t *)f_name)[i]==0){
            xtcp_debug_printf("\n");  
            break;
        }
        xtcp_debug_printf("%c",f_name[i]);        
    }
    #endif
    // 播放通道音乐
    user_music_play(ch,p_music_info);
    // 获得播放时间
    if(timetask_now.task_musicplay[ch].rttask_f){
        memcpy(&g_sys_val.rttask_musinfo,p_music_info,sizeof(task_music_info_t));
        timetask_now.task_musicplay[ch].music_tolsec = get_music_tolsec(&g_sys_val.rttask_musinfo);
        timetask_now.task_musicplay[ch].music_sec=0;
    }
    //xtcp_debug_printf("music play out\n");
}

void rttask_music_play(uint16_t id){
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        // 关闭音乐播放
        if(timetask_now.ch_state[i]!=0xFF && timetask_now.task_musicplay[i].task_id==id && timetask_now.task_musicplay[i].rttask_f){
            user_audio_senden(i);
        }
    }
}

void rttask_music_totimer(uint16_t id,uint16_t music_sec){
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if(timetask_now.ch_state[i]!=0xFF && timetask_now.task_musicplay[i].task_id==id && timetask_now.task_musicplay[i].rttask_f){
            timetask_now.task_musicplay[i].music_sec=music_sec;
        }
    }
}

uint16_t get_music_tolsec(task_music_info_t *p_music_info){
    uint16_t music_sec=0;
    user_fl_get_patchlist(g_tmp_union.buff);
    uint32_t *patch_tol = &g_tmp_union.buff[0];
    dir_info_t *dir_info = &g_tmp_union.buff[4];

    for(uint16_t i=0;i<*patch_tol;i++){
        // 比较文件夹
        if(charncmp(p_music_info->music_path,dir_info[i].name,PATCH_NAME_NUM)==1){
            user_fl_get_musiclist(dir_info[i].sector,g_tmp_union.buff);
            uint32_t *music_tol = &g_tmp_union.buff[0];
            music_info_t *music_info = &g_tmp_union.buff[4];
            // 比较文件夹
            for(uint16_t j=0;j<*music_tol;j++){
                if(charncmp(p_music_info->music_name,music_info[j].name,MUSIC_NAME_NUM)==1){
                    //xtcp_debug_printf("get music sec %d\n",music_info[j].totsec);
                    return music_info[j].totsec;
                }
            }
        }
    }
    return music_sec;
}

void close_rttask_musicplay(uint16_t id){
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if(timetask_now.ch_state[i]!=0xFF && timetask_now.task_musicplay[i].task_id==id && timetask_now.task_musicplay[i].rttask_f){
            task_music_config_stop(i);
        }
    }
}

