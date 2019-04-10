#include "music_play.h"
#include "user_unti.h"
#include "user_xccode.h"
#include "list_instance.h"
#include "fl_buff_decode.h"
#include "list_contorl.h"
#include "eth_audio.h"
#include "eth_audio_config.h"
#include "debug_print.h"

audio_txlist_t t_audio_txlist;

extern uint8_t f_name[];

void task_music_send(uint8_t ch){
    div_node_t *div_tmp_p;
    t_audio_txlist.num_info=0;
    // 配置发送目标
    for(uint8_t i=0;i<tmp_union.task_allinfo_tmp.task_coninfo.div_tolnum;i++){
        //查找目标设备与IP
        debug_printf("des mca: %x,%x,%x,%x,%x,%x\n",tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac[0],
                                                tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac[1],
                                                tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac[2],
                                                tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac[3],
                                                tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac[4],
                                                tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac[5]);
        div_tmp_p = get_div_info_p(tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac);
        if(div_tmp_p==null){
            //i++;
            continue;
        }
        //if(div_tmp_p->div_info.div_state == 00)
        //    continue;
        //debug_printf("div ok\n");
        //
        //获得mac
        memcpy(t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac,tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].mac,6);
        //获得IP
        memcpy(t_audio_txlist.t_des_info[t_audio_txlist.num_info].ip,div_tmp_p->div_info.ip,4);
        //获得分区控制位
        
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].area_contorl = tmp_union.task_allinfo_tmp.task_maclist.taskmac_info[i].zone_control;
        t_audio_txlist.num_info++;
        //
    }
    
    // text
    /*
    for(;t_audio_txlist.num_info<2;t_audio_txlist.num_info++){
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[0] = 0x10;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[1] = 0x7B;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[2] = 0x44;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[3] = 0x51;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[4] = 0x2C;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].mac[5] = 0x30;

        t_audio_txlist.t_des_info[t_audio_txlist.num_info].ip[0] = 172;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].ip[1] = 16;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].ip[2] = 13;
        t_audio_txlist.t_des_info[t_audio_txlist.num_info].ip[3] = 224;
    }
    */
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
    g_sys_val.audio_type[ch] = tmp_union.task_allinfo_tmp.task_coninfo.task_prio;
    //g_sys_val.audio_type[ch] = 0x11;
    //i_ethaud_cfg->set_audio_type(g_sys_val.audio_type);
    set_audio_type(g_sys_val.audio_type);
    // 配置目标
    user_audio_desip_set(ch,tmp_union.task_allinfo_tmp.task_coninfo.task_prio);
    // 配置音量
    set_audio_vol(ch,tmp_union.task_allinfo_tmp.task_coninfo.task_vol);
    // 发送使能
    user_audio_senden(ch);
}

void task_music_stop(uint8_t ch){
    user_music_stop(ch);
    // 发送禁止
    user_audio_send_dis(ch);
    //debug_printf("music stop\n");
}

void task_music_play(uint8_t ch,uint8_t num){
    //获取歌曲路径名
    uint8_t i,j,ch_tmp;
    for(uint8_t i=0;i<MAX_MUSIC_CH;i++){
        if(g_sys_val.disp_ch[i]==ch){
            ch_tmp = i;
            break;
        }
    }
    for(i=0;i<(PATCH_NAME_NUM/2);i++){
        if(((uint16_t *)tmp_union.task_allinfo_tmp.task_musiclist.music_info[num].music_path)[i]==0)
            break;
        ((uint16_t *)f_name)[i] = ((uint16_t *)tmp_union.task_allinfo_tmp.task_musiclist.music_info[num].music_path)[i];
    }
    ((uint16_t *)f_name)[i] = 0x002F;
    i++;
    for(j=0; j<(MUSIC_NAME_NUM/2); i++,j++){
        if(((uint16_t *)tmp_union.task_allinfo_tmp.task_musiclist.music_info[num].music_name)[j]==0)
            break;
        ((uint16_t *)f_name)[i] = ((uint16_t *)tmp_union.task_allinfo_tmp.task_musiclist.music_info[num].music_name)[j];
        g_sys_val.dispmusic_buff[ch_tmp][j*2] = tmp_union.task_allinfo_tmp.task_musiclist.music_info[num].music_name[j*2+1];
        g_sys_val.dispmusic_buff[ch_tmp][j*2+1] = tmp_union.task_allinfo_tmp.task_musiclist.music_info[num].music_name[j*2];
    }
    ((uint16_t *)f_name)[i] = 0x00;
    g_sys_val.dispmusic_buff[ch_tmp][j*2] =0x00;
    g_sys_val.dispmusic_buff[ch_tmp][j*2+1]=0x00; 
    #if 0
    for(i=0;i<(MUSIC_NAME_NUM+PATCH_NAME_NUM)/2;i++){
        if(((uint16_t *)f_name)[i]==0){
            debug_printf("\n");  
            break;
        }
        debug_printf("%c",f_name[i]);        
    }
    #endif
    // 播放通道音乐
    user_music_play(ch);
    //debug_printf("music play out\n");
}


