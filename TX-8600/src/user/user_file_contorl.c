#include "user_file_contorl.h"
#include "list_instance.h"
#include "ack_build.h"
#include "user_xccode.h"
#include "list_contorl.h"
#include "user_unti.h"
#include "file_list.h"
#include "string.h"
#include "user_messend.h"
#include "task_decode.h"
#include "debug_print.h"

extern uint8_t f_name[];

//====================================================================================================
// 音乐库 文件夹名称列表获取                MUSIC_PATCH_CHK_CMD     0xB800
//====================================================================================================
void music_patch_list_chk_recive(){
    //是否有列表正在发送
    if(conn_sending_s.id!=null)
        return;
    conn_sending_s.conn_sending_tim = 0;
    // 连发状态初始化
    conn_sending_s.conn_state |= PATCH_LIST_SENDING;
    conn_sending_s.patchlist.patch_inc =0;
    conn_sending_s.patchlist.pack_inc=0;
    memcpy(conn_sending_s.patchlist.id,&xtcp_rx_buf[POL_ID_BASE],6);
    //
    conn_sending_s.could_s = xtcp_rx_buf[POL_COULD_S_BASE];
    could_list_init();
    //
    user_sending_len = music_patchlist_chk_build();
    user_xtcp_send(conn,conn_sending_s.could_s);
    
}

void music_patch_list_send_decode(){
    user_sending_len = music_patchlist_chk_build();
    user_xtcp_send(conn,conn_sending_s.could_s);
}

//====================================================================================================
// 音乐库 详细音乐名称列表获取   MUSIC_LIB_CHK_CMD                 0xB801
//====================================================================================================
void music_music_list_chk_recive(){
    //是否有列表正在发送
    if(conn_sending_s.id!=null)
        return;
    #if 0
    for(uint8_t i=0;i<100;i++){
        debug_printf("%2x ",xtcp_rx_buf[i]);
        if(i%20==0){
            debug_printf("\n");
        }
    }
    #endif
    //获取文件夹列表
    user_fl_get_patchlist(tmp_union.buff);
    uint32_t *patch_tol = &tmp_union.buff[0];
    dir_info_t *dir_info = &tmp_union.buff[4];
    //比较文件夹

    //debug_printf("src floa:");
    //for(uint8_t j=0;j<PATCH_NAME_NUM;j++){
    //        debug_printf("%x,",xtcp_rx_buf[(MUS_LIBHCK_CHKPATCH_NAME)+j]);
    //    }
    //debug_printf("\n");
    
    for(uint8_t i=0;i<*patch_tol;i++){
        //debug_printf("des floa:");
        //for(uint8_t j=0;j<PATCH_NAME_NUM/2;j++){
        //    debug_printf("%x,",dir_info[i].name[j]);
        //}
        //debug_printf("\n");
        #if 0
        for(uint8_t j=0;j<PATCH_NAME_NUM;j++){
            debug_printf("%x ",xtcp_rx_buf[MUS_LIBHCK_CHKPATCH_NAME+j]);
        }
        debug_printf("\n");
        for(uint8_t j=0;j<PATCH_NAME_NUM/2;j++){
            debug_printf("%x ",dir_info[i].name[j]);
        } 
        debug_printf("\n");
        #endif
        
        if(charncmp(&xtcp_rx_buf[MUS_LIBHCK_CHKPATCH_NAME],dir_info[i].name,PATCH_NAME_NUM)==1){
            conn_sending_s.musiclist.sector_index = dir_info[i].sector;
            conn_sending_s.musiclist.music_inc = 0;
            conn_sending_s.musiclist.pack_inc = 0;
            memcpy(conn_sending_s.musiclist.id,&xtcp_rx_buf[POL_ID_BASE],6);
            //
            conn_sending_s.conn_sending_tim = 0;
            conn_sending_s.conn_state |= MUSICNAME_LIST_SENDING;
            conn_sending_s.musiclist.sector_index;
            //
            conn_sending_s.could_s = xtcp_rx_buf[POL_COULD_S_BASE];
            could_list_init();
            //debug_printf("conn id %d recid %d\n",g_sys_val.could_conn.id,conn.id);
            //
            user_sending_len = music_namelist_chk_build(1);
            user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
            return;
        }
    }
    user_sending_len = music_namelist_chk_build(0);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//音乐名称连发
void music_music_list_send_decode(){
    debug_printf("music send\n");
    user_sending_len = music_namelist_chk_build(1);
    user_xtcp_send(conn,conn_sending_s.could_s);
}

//====================================================================================================
// 音乐库 音乐库名称修改                   MUSIC_PATCHNAME_CON_CMD   0xB802
//====================================================================================================
void music_patchname_config_recive(){
    uint8_t state=0;
    if(g_sys_val.file_bat_contorl_s)
        goto file_contorl_end;
    //-----------------------------------------------------------------
    user_fl_get_patchlist(tmp_union.buff);
    uint32_t *patch_tol = &tmp_union.buff[0];
    dir_info_t *dir_info = &tmp_union.buff[4];
    
    // 创建文件夹
    if(xtcp_rx_buf[MUS_PTHCON_CONFIG]==0){
        //for(uint8_t i=0;i<30;i++){
        //    debug_printf("%d,",xtcp_rx_buf[MUS_PTHCON_SRCNAME+i]);
        //}
        //debug_printf("\n");
        if(user_file_mir(&xtcp_rx_buf[MUS_PTHCON_SRCNAME])==0){
             debug_printf("floder add\n");
             state =1;
        }
    }
    //删除文件夹
    if(xtcp_rx_buf[MUS_PTHCON_CONFIG]==1){
        task_music_stop_all();
        if(user_filefolder_del(&xtcp_rx_buf[MUS_PTHCON_SRCNAME])==0)
            state = 1;
        debug_printf("floder del\n");
    }
    // 重命名
    if(xtcp_rx_buf[MUS_PTHCON_CONFIG]==2){
        //比较文件夹
        for(uint8_t i=0;i<*patch_tol;i++){
            if(charncmp(&xtcp_rx_buf[MUS_PTHCON_SRCNAME],dir_info[i].name,PATCH_NAME_NUM)==1){
                if(user_file_rename(&xtcp_rx_buf[MUS_PTHCON_SRCNAME],&xtcp_rx_buf[MUS_PTHCON_DESNAME])==0){
                    debug_printf("floder rename\n");
                    state = 1;
                }
            }
        }
    }
    file_contorl_end:
    // 完成操作
    debug_printf("file contorl busy %d\n",state);
    if(state){
        g_sys_val.file_conn_tmp = conn;
        g_sys_val.file_ack_cmd = MUSIC_PATCHNAME_CON_CMD;
        g_sys_val.file_contorl_couldf = xtcp_rx_buf[POL_COULD_S_BASE];
        user_sending_len = onebyte_ack_build(02,MUSIC_PATCHNAME_CON_CMD,&xtcp_rx_buf[POL_ID_BASE]);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    }
    else{
        user_sending_len = onebyte_ack_build(state,MUSIC_PATCHNAME_CON_CMD,&xtcp_rx_buf[POL_ID_BASE]);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    }
}

//====================================================================================================
// 音乐库 上传状态查询                   MUSIC_BUSY_CHK_CMD   0xB803
//====================================================================================================
void music_busy_chk_recive(){
    debug_printf("music chk\n");   
    user_sending_len = onebyte_ack_build(0,MUSIC_BUSY_CHK_CMD,&xtcp_rx_buf[POL_ID_BASE]);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}

//====================================================================================================
// 音乐库 文件操作                  
//====================================================================================================
#define FILE_CONTORL_COPY 0
#define FILE_CONTORL_MOVE 1
#define FILE_CONTORL_DEL 2

void file_patch_get(uint8_t *patch,uint8_t *file,uint16_t *file_patch){
    uint8_t i,j; 
    //获得文件夹名称
    memcpy(tmp_union.buff,patch,PATCH_NAME_NUM);
    for(i=0;i<(PATCH_NAME_NUM/2);i++){
        if(((uint16_t *)tmp_union.buff)[i]==0)
            break;
        file_patch[i] = ((uint16_t *)tmp_union.buff)[i];
    }
    file_patch[i] = 0x002F;
    i++;
    //获得文件名称
    memcpy(tmp_union.buff,file,MUSIC_NAME_NUM);
    for(j=0; j<(MUSIC_NAME_NUM/2); i++,j++){
        if(((uint16_t *)tmp_union.buff)[j]==0)
            break;
        file_patch[i] = ((uint16_t *)tmp_union.buff)[j];
    }
    file_patch[i] = 0x00;
}


// 取消   MUSIC_FILE_CONTORL_CMD   0xB804  
#if 1
void music_file_config_recive(){
    uint8_t state = 0;
    uint32_t *patch_tol;
    dir_info_t *dir_info;
    
    task_music_stop_all();
    if(g_sys_val.file_bat_contorl_s)
        goto file_config_end;
//    debug_printf("file contorl %d\n",xtcp_rx_buf[FILECON_CONTORL_B]);
    // 取源文件路径
    file_patch_get(&xtcp_rx_buf[FILECON_SRC_PATCHNAME],&xtcp_rx_buf[FILECON_SRC_MUSICNAME],g_sys_val.fsrc);
    // 去目标文件路径
    file_patch_get(&xtcp_rx_buf[FILECON_DES_PATCHNAME],&xtcp_rx_buf[FILECON_SRC_MUSICNAME],g_sys_val.fdes);

    //-------------------------------------------------------------------------------
    for(uint8_t i=0;i<(PATCH_NAME_NUM+MUSIC_NAME_NUM)/2;i++){
        debug_printf("%x ",g_sys_val.fsrc[i]);
        if(g_sys_val.fsrc[i]==0)
            break;
    }
    #if 1
    debug_printf("\n");
    for(uint8_t i=0;i<(PATCH_NAME_NUM+MUSIC_NAME_NUM)/2;i++){
        debug_printf("%x ",g_sys_val.fdes[i]);
        if(g_sys_val.fdes[i]==0)
            break;
    }
    debug_printf("\n");
    #endif
    //
    // 移动文件
    if(xtcp_rx_buf[FILECON_CONTORL_B] == FILE_CONTORL_MOVE){
        user_fl_get_patchlist(tmp_union.buff);
        patch_tol = &tmp_union.buff[0];
        dir_info = &tmp_union.buff[4]; 
    
        for(uint8_t i=0;i<*patch_tol;i++){
            if(charncmp(dir_info[i].name,&xtcp_rx_buf[FILECON_DES_PATCHNAME],PATCH_NAME_NUM)==1){
                if((dir_info[i].music_num + 1)>MAX_SDCARD_MUSIC_NUM){
                    state = 4;
                    goto file_config_end;
                }
            }
        }
        if(user_file_move(g_sys_val.fsrc,PATCH_NAME_NUM+MUSIC_NAME_NUM,g_sys_val.fdes,PATCH_NAME_NUM+MUSIC_NAME_NUM)==0){
            state = 1;
        }
    }
    // 复制文件
    else if(xtcp_rx_buf[FILECON_CONTORL_B] == FILE_CONTORL_COPY){
        user_fl_get_patchlist(tmp_union.buff);
        patch_tol = &tmp_union.buff[0];
        dir_info = &tmp_union.buff[4]; 
    
        for(uint8_t i=0;i<*patch_tol;i++){
            if(charncmp(dir_info[i].name,&xtcp_rx_buf[FILECON_DES_PATCHNAME],PATCH_NAME_NUM)==1){
                if((dir_info[i].music_num + 1)>MAX_SDCARD_MUSIC_NUM){
                    state = 4;
                    debug_printf("\n\nfull\n");
                    goto file_config_end;
                }
            }
        }

        if(user_file_copy(g_sys_val.fsrc,PATCH_NAME_NUM+MUSIC_NAME_NUM,g_sys_val.fdes,PATCH_NAME_NUM+MUSIC_NAME_NUM)==0){
            state = 1;
        }
    }
    // 删除文件
    else if(xtcp_rx_buf[FILECON_CONTORL_B] == FILE_CONTORL_DEL){
        if(user_file_delete(g_sys_val.fsrc,PATCH_NAME_NUM+MUSIC_NAME_NUM)==0){
            state = 1;
        }
    }
    file_config_end:
    debug_printf("\n\nfile contorl ack:%d\n",state);
    if(state==1){
        g_sys_val.file_conn_tmp = conn;
        g_sys_val.file_contorl_couldf = xtcp_rx_buf[POL_COULD_S_BASE];
        g_sys_val.file_ack_cmd = MUSIC_FILE_CONTORL_CMD;
        memcpy(g_sys_val.file_contorl_id,xtcp_rx_buf[POL_ID_BASE],6);
        user_sending_len = onebyte_ack_build(02,MUSIC_FILE_CONTORL_CMD,&xtcp_rx_buf[POL_ID_BASE]);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    }
    else{
        user_sending_len = onebyte_ack_build(state,MUSIC_FILE_CONTORL_CMD,&xtcp_rx_buf[POL_ID_BASE]);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
    
    }
}
#endif

//====================================================================================================
// 音乐库 进度查询                    MUSIC_PROCESS_BAR_CMD    0xB805
//====================================================================================================
void musicfile_bar_chk_recive(){
    uint8_t file_process;
    uint8_t state=0;
    if(user_file_progress(&file_process))
        state = 1;
    user_sending_len = file_progress_build(state,file_process,&xtcp_rx_buf[POL_ID_BASE],
                                            g_sys_val.file_bat_nametmp,g_sys_val.file_bat_srcpatch);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);   
}

//====================================================================================================
// 音乐文件批量操作                    MUSIC_BAT_CONTORL_CMD    0xB806
//====================================================================================================
void music_bat_contorl_recive(){
    if(xtcp_rx_buf[MUSIC_BAT_TOL_NUM]>25)
        return;
    task_music_stop_all();
    // 停止操作
    if(xtcp_rx_buf[MUSIC_BAT_CONTORL]==3){
        debug_printf("bat contorl stop\n");
        g_sys_val.file_bat_contorl_s = 0;
        user_file_stop();
        user_sending_len = twobyte_ack_build(xtcp_rx_buf[MUSIC_BAT_CONTORL],0,MUSIC_BAT_CONTORL_CMD);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);  
        return;
    }
    // 单次配一个批量任务
    if((g_sys_val.file_bat_contorl_s)||((g_sys_val.file_bat_conn.id!=null)&&(conn.id!=g_sys_val.file_bat_conn.id))){
        user_sending_len = twobyte_ack_build(xtcp_rx_buf[MUSIC_BAT_CONTORL],01,MUSIC_BAT_CONTORL_CMD);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);  
        return;
    }
    if(xtcp_rx_buf[MUSIC_BAT_PACKINC]==0){
        g_sys_val.file_bat_conn = conn;
        g_sys_val.file_batpack_inc = 0;
        g_sys_val.file_bat_musicinc = 0;
        g_sys_val.file_bat_tolnum = 0;
        g_sys_val.file_bat_contorl = xtcp_rx_buf[MUSIC_BAT_CONTORL];
        memcpy(g_sys_val.file_bat_id,&xtcp_rx_buf[POL_ID_BASE],6);
        memcpy(g_sys_val.file_bat_srcpatch,&xtcp_rx_buf[MUSIC_BAT_SRC_PATCH],PATCH_NAME_NUM);
        memcpy(g_sys_val.file_bat_despatch,&xtcp_rx_buf[MUSIC_BAT_DES_PATCH],PATCH_NAME_NUM);
        g_sys_val.file_bat_could_f = xtcp_rx_buf[POL_COULD_S_BASE];
    }
    //---------------------------------------------------------------------------------------
    // 保存文件名称
    uint16_t data_base = MUSIC_BAT_NAME_BASE;
    for(uint8_t i=0; i<xtcp_rx_buf[MUSIC_BAT_TOL_NUM] ;i++){
        user_file_bat_write(g_sys_val.file_bat_tolnum,&xtcp_rx_buf[data_base]);
        #if 1
        debug_printf("get file:");
        for(uint8_t j=0;j<(MUSIC_NAME_NUM);j++){
            debug_printf("%x ",xtcp_rx_buf[data_base+j]);
        }
        debug_printf("\n");
        #endif
        data_base += MUSIC_NAME_NUM;
        g_sys_val.file_bat_tolnum++;
    }
    //----------------------------------------------------------------------------------------
    g_sys_val.file_batpack_inc++;
    g_sys_val.file_bat_overtime = 0;
    //完成分包接收
    if(((xtcp_rx_buf[MUSIC_BAT_PACKINC]+1)==xtcp_rx_buf[MUSIC_BAT_PACKTOL])&&(g_sys_val.file_batpack_inc == xtcp_rx_buf[MUSIC_BAT_PACKTOL])){
        //======================================================================================================================    
        user_fl_get_patchlist(tmp_union.buff);
        uint32_t *patch_tol;
        dir_info_t *dir_info;
        patch_tol = &tmp_union.buff[0];
        dir_info = &tmp_union.buff[4]; 
        uint8_t *music_tmp = &tmp_union.buff[1024]; 
        
        for(uint8_t i=0;i<*patch_tol;i++){
            if(charncmp(g_sys_val.file_bat_despatch,dir_info[i].name,PATCH_NAME_NUM)==1){
                debug_printf("\n\n bat music chk\n");
                debug_printf("file:%d,muc:%d\n",dir_info[i].music_num,g_sys_val.file_bat_tolnum);
                if(dir_info[i].music_num+g_sys_val.file_bat_tolnum > MAX_SDCARD_MUSIC_NUM){
                    user_sending_len = twobyte_ack_build(xtcp_rx_buf[MUSIC_BAT_CONTORL],0,MUSIC_BAT_CONTORL_CMD);    
                    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]); 

                    //user_file_bat_read(g_sys_val.file_bat_musicinc-1,music_tmp);
                    memset(music_tmp,0,64);
                    user_sending_len = file_batinfo_build(g_sys_val.file_bat_despatch,music_tmp,2,2,g_sys_val.file_bat_contorl);
                    user_xtcp_send(g_sys_val.file_bat_conn,xtcp_rx_buf[POL_COULD_S_BASE]);
                    return;
                }
                break;
                
            }
        }
        //======================================================================================================================    
        g_sys_val.file_bat_contorl_s = 1;
        g_sys_val.file_bat_tim = 0;
        user_sending_len = twobyte_ack_build(xtcp_rx_buf[MUSIC_BAT_CONTORL],0,MUSIC_BAT_CONTORL_CMD);
        user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);  
        file_bat_contorl_event(0);
        debug_printf("bat music tol %d\n",g_sys_val.file_bat_tolnum);
        debug_printf("bat contorl recive over\n");
    
    }
}

//====================================================================================================
// 音乐文件批量操作信息回复                    MUSIC_BAT_STATE_CMD    0xB807
//===================================================================================================
void music_bat_info_recive(){
    g_sys_val.file_bat_resendf=0;
    // 操作完成
    if(g_sys_val.file_bat_resend_tmp[0]==0){
        g_sys_val.file_bat_conn.id=null;
    }
    debug_printf("recive bat info\n");
}

//==================================================================================================
// 文件操作回复 及接收超时
//==================================================================================================
void file_contorl_ack_decode(uint8_t error_code){
    if(g_sys_val.file_ack_cmd!=0){ 
        debug_printf("folar error %d\n",error_code);
        if(error_code!=0)
        {
            user_sending_len = onebyte_ack_build(0,g_sys_val.file_ack_cmd,g_sys_val.file_contorl_id);
            user_xtcp_send(g_sys_val.file_conn_tmp,g_sys_val.file_contorl_couldf);    
        }
        else{
            mes_send_listinfo(MUSICLIS_INFO_REFRESH,0);
            user_sending_len = onebyte_ack_build(1,g_sys_val.file_ack_cmd,g_sys_val.file_contorl_id);
            user_xtcp_send(g_sys_val.file_conn_tmp,g_sys_val.file_contorl_couldf);
        }
        g_sys_val.file_ack_cmd = 0;
    }
}

//==================================================================================================
// 文件批量操作处理
//==================================================================================================
#define FILE_BAT_CONTORL_SUCCEED    00
#define FILE_BAT_CONTORL_BUSY       01
#define FILE_BAT_CONTORL_FAIL       02

void file_bat_contorl_event(uint8_t error_code){
    uint8_t file_state=0;
    uint8_t bat_state=FILE_BAT_CONTORL_BUSY;
    uint8_t contorl;
    uint8_t *music_tmp = &tmp_union.buff[1024]; 

    uint32_t *patch_tol;
    dir_info_t *dir_info;

    if(g_sys_val.file_bat_contorl_s){ 
        //-----------------------------------------------------------------------------------------------------
        // 文件操作错误
        debug_printf("folar error %d\n",error_code);
        if(error_code!=0){
            file_state=1;
        }
        //-----------------------------------------------------------------------------------------------------
        bat_next_music:
        g_sys_val.file_bat_tim = 0;
        //---------------------------------------------------------------------------------------------------
        contorl = g_sys_val.file_bat_contorl;
        //批量操作完成
        if(g_sys_val.file_bat_musicinc >= g_sys_val.file_bat_tolnum){
            g_sys_val.file_bat_contorl_s = 0;
            bat_state = FILE_BAT_CONTORL_SUCCEED;
            //信息更新
            mes_send_listinfo(MUSICLIS_INFO_REFRESH,0);
        }
        if(g_sys_val.file_bat_musicinc!=0){
            g_sys_val.file_bat_resend_inc = 0;
            g_sys_val.file_bat_resendtim = 0;
            g_sys_val.file_bat_resendf = 1;
            user_file_bat_read(g_sys_val.file_bat_musicinc-1,music_tmp);
            g_sys_val.file_bat_resend_tmp[0] = bat_state;
            g_sys_val.file_bat_resend_tmp[1] = file_state;
            g_sys_val.file_bat_resend_tmp[2] = contorl;
            user_sending_len = file_batinfo_build(g_sys_val.file_bat_srcpatch,music_tmp,bat_state,file_state,contorl);
            user_xtcp_send(g_sys_val.file_bat_conn,g_sys_val.file_bat_could_f);   
            //g_sys_val.file_bat_conn.id = null;
        }
        if(bat_state==FILE_BAT_CONTORL_SUCCEED)
            return;
        //-----------------------------------------------------------------------------------------------------
        // 操作下一首音乐
        uint8_t busy_state=0;
        file_state = 0;
        user_file_bat_read(g_sys_val.file_bat_musicinc,music_tmp); //
        memcpy(g_sys_val.file_bat_nametmp,music_tmp,MUSIC_NAME_NUM);
        // 取源文件路径
        file_patch_get(g_sys_val.file_bat_srcpatch,music_tmp,g_sys_val.fsrc);
        // 取目标文件路径
        file_patch_get(g_sys_val.file_bat_despatch,music_tmp,g_sys_val.fdes);
        //--------------------------------------------------------------------------------
        // DEBUG 打印
        debug_printf("src file :");
        for(uint8_t i=0;i<(PATCH_NAME_NUM+MUSIC_NAME_NUM)/2;i++){
            debug_printf("%x ",g_sys_val.fsrc[i]);
            if(g_sys_val.fsrc[i]==0)
                break;
        }
        debug_printf("\ndes file :");
        for(uint8_t i=0;i<(PATCH_NAME_NUM+MUSIC_NAME_NUM)/2;i++){
            debug_printf("%x ",g_sys_val.fdes[i]);
            if(g_sys_val.fdes[i]==0)
                break;
        }
        debug_printf("\n");
        //----------------------------------------------------------------------------
        // 复制
        if(g_sys_val.file_bat_contorl==0){
            user_fl_get_patchlist(tmp_union.buff);
            patch_tol = &tmp_union.buff[0];
            dir_info = &tmp_union.buff[4]; 
            for(uint8_t i=0;i<*patch_tol;i++){
                if(charncmp(&xtcp_rx_buf[MUS_LIBHCK_CHKPATCH_NAME],g_sys_val.file_bat_despatch,PATCH_NAME_NUM)==1){
                    if(dir_info[i].music_num_full){
                        file_state = 2;
                        goto next_bat_music;
                    }
                }
            }
            
            if(user_file_copy(g_sys_val.fsrc,PATCH_NAME_NUM+MUSIC_NAME_NUM,g_sys_val.fdes,PATCH_NAME_NUM+MUSIC_NAME_NUM)){
                file_state = 3;
            }
        }
        // 移动
        else if(g_sys_val.file_bat_contorl==1){
            //
            user_fl_get_patchlist(tmp_union.buff);
            patch_tol = &tmp_union.buff[0];
            dir_info = &tmp_union.buff[4]; 
            for(uint8_t i=0;i<*patch_tol;i++){
                if(charncmp(&xtcp_rx_buf[MUS_LIBHCK_CHKPATCH_NAME],g_sys_val.file_bat_despatch,PATCH_NAME_NUM)==1){
                    if(dir_info[i].music_num_full){
                        file_state = 2;
                        goto next_bat_music;
                    }
                }
            }
            
            if(user_file_move(g_sys_val.fsrc,PATCH_NAME_NUM+MUSIC_NAME_NUM,g_sys_val.fdes,PATCH_NAME_NUM+MUSIC_NAME_NUM)){
                file_state = 3;
            } 
        }
        // 删除
        else if(g_sys_val.file_bat_contorl==2){
            if(user_file_delete(g_sys_val.fsrc,PATCH_NAME_NUM+MUSIC_NAME_NUM)){
                file_state = 3;
            }
        }
        next_bat_music:
        g_sys_val.file_bat_musicinc++;
        if(file_state == 3)
            goto bat_next_music;
        if(file_state == 2)
            goto bat_next_music;
        //----------------------------------------------------------------------------------------------------- 
    }
}

//==================================================================================================
// 文件批量操作处理信息回报重发 400ms
//==================================================================================================
void bat_filecontorl_resend_tim(){
    if((g_sys_val.file_bat_conn.id!=null)&&(g_sys_val.file_bat_contorl_s==0)){
        g_sys_val.file_bat_overtime++;
        if(g_sys_val.file_bat_overtime>8){
            g_sys_val.file_bat_conn.id=null;
        }
    }    
    // 批量操作回复 进度回复
    static uint8_t tim=0;
    uint8_t file_process;
    tim++;
    if((tim>5)&&(g_sys_val.file_bat_contorl_s)){
        tim=0;
        if(user_file_progress(&file_process)==0){
            user_sending_len = file_progress_build(0,file_process,g_sys_val.file_bat_id,
                               g_sys_val.file_bat_nametmp,g_sys_val.file_bat_srcpatch);
            user_xtcp_send(g_sys_val.file_bat_conn,g_sys_val.file_bat_could_f);
            debug_printf("send %d\n",file_process);
        }
    }
    //---------------------------------------------------------------
    // 重发处理
    if(g_sys_val.file_bat_resendf){
        g_sys_val.file_bat_resendtim++;
        //---------------------------------------------------------
        // 重发数据
        if(g_sys_val.file_bat_resendtim>4){
            g_sys_val.file_bat_resendtim=0;
            user_file_bat_read(g_sys_val.file_bat_musicinc-1,tmp_union.buff);
            user_sending_len = file_batinfo_build(g_sys_val.file_bat_srcpatch,tmp_union.buff,
                                                  g_sys_val.file_bat_resend_tmp[0],g_sys_val.file_bat_resend_tmp[1],g_sys_val.file_bat_resend_tmp[2]);
            user_xtcp_send(g_sys_val.file_bat_conn,g_sys_val.file_bat_could_f);   
            // 停止重发
            g_sys_val.file_bat_resend_inc++;
            if(g_sys_val.file_bat_resend_inc>2){
                g_sys_val.file_bat_resendf=0;
                // 操作完成
                if(g_sys_val.file_bat_resend_tmp[0]==0){
                    g_sys_val.file_bat_conn.id=null;
                }
            }
        }
    }
}


