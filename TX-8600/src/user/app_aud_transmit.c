#include "app_aud_transmit.h"
#include "sys_config_dat.h"
#include "ack_build.h"
#include "user_xccode.h"
#include "user_unti.h"

void set_audtrainsmit_divlist_recive(){
    uint16_t dat_base=AUD_TRAINS_DATBASE;
    uint8_t succse_s = 1;
    uint8_t state = app_trainsmit_ch_chk();
    if(state!=0xFF){
        succse_s=0;
        //获得音频优先级
        g_tmp_union.audts_divlist.prio = (xtcp_rx_buf[AUD_TRAINS_PRIO]<<4)+1;
        // 获得播放列表
        g_tmp_union.audts_divlist.num = xtcp_rx_buf[AUD_TRAINS_DIVNUM];
        g_tmp_union.audts_divlist.id = state;
        for(uint8_t i=0;i<xtcp_rx_buf[AUD_TRAINS_DIVNUM];i++){
            memcpy(g_tmp_union.audts_divlist.divinfo[i].mac,&xtcp_rx_buf[dat_base+AUD_TRAINS_DIVMAC],6);
            memcpy(g_tmp_union.audts_divlist.divinfo[i].ip,&xtcp_rx_buf[dat_base+AUD_TRAINS_DIVIP],4);
            g_tmp_union.audts_divlist.divinfo[i].area_info = xtcp_rx_buf[dat_base+AUD_TRAINS_AREA];
            dat_base+=AUD_TRAINS_DIVINFO_LEN;
        }
        // 保存播放列表
        app_trainsmit_divlist_set();
    }
    user_sending_len = twobyte_ack_build(succse_s,state ,BE0E_AUDTRAINSMIT_DIVLIST_CMD);
    user_xtcp_send(conn,xtcp_rx_buf[POL_COULD_S_BASE]);
}




