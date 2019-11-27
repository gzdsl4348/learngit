#include "could_serve.h"
#include "ack_build.h"
#include "sys_config_dat.h"
#include "user_xccode.h"
#include "user_unti.h"
#include "account.h"
#include "debug_print.h"
#include "conn_process.h"
#include "user_lcd.h"
#include "sys_log.h"
#include "user_log.h"

#define CLD_HEART_TIME_CNT  5 //5秒心跳

void could_heart_send_timer(){
    #if COULD_TCP_EN
    g_sys_val.could_heart_timcnt++;
    if(g_sys_val.could_heart_timcnt>CLD_HEART_TIME_CNT){
        g_sys_val.could_heart_timcnt = 0;
        //xtcp_debug_printf("cld id %d\n",g_sys_val.could_conn.id);
        if(g_sys_val.could_conn.id!=0){
            user_sending_len = cld_heart_build();
            user_could_send(1);  
            //text 
            #if 0
            user_sending_len = cld_heart_build();
            user_could_send(1);  
            user_sending_len = cld_heart_build();
            user_could_send(1);  
            #endif
            //xtcp_debug_printf("send cld\n");
            #if 0
            if(tmp1>=2){
                xtcp_debug_printf("send colud\n");
                xtcp_rx_buf[POL_ID_BASE] = 1;
                account_list_updat();
                tmp1=0;
            }
            #endif
            g_sys_val.could_send_cnt++;
            if(g_sys_val.could_send_cnt>2){    //20秒重连
                g_sys_val.could_conn.id=0;
                //xtcp_debug_printf("could send time over\n");
                user_xtcp_close(g_sys_val.could_conn);
                //user_xtcp_unlisten(g_sys_val.colud_port);
                g_sys_val.colud_connect_f=0;
                // 日志更新
                log_could_offline();
            }
        }
        else if(g_sys_val.colud_connect_f==0){
            g_sys_val.colud_connect_f=1;
            g_sys_val.could_send_cnt = 0;
            user_xtcp_sendfifo_init();
            xtcp_debug_printf("colud connect\n");
            //user_xtcp_unlisten(g_sys_val.colud_port);
            disp_couldstate(0);
            user_xtcp_connect_tcp(g_sys_val.could_ip);
        }
    }
    #endif
}

void dns_domain_recive_decode(){
    #if 0
    for(uint8_t i=0;i<49;i++){
        xtcp_debug_printf("%2x ",xtcp_rx_buf[i]);
    }
    xtcp_debug_printf("\n");
    #endif
    if(xtcp_rx_buf[DNS_CODE_FLAG]!=0x81 || xtcp_rx_buf[DNS_CODE_FLAG+1]!=0x80)
        return;
    if(xtcp_rx_buf[DNS_ANSWERS+1]!=1)
        return;
    if(xtcp_rx_buf[44]!=4)
        return;
    memcpy(g_sys_val.could_ip,&xtcp_rx_buf[45],4);
    //xtcp_debug_printf("\nrec dns could ip %d %d %d %d \n\n",g_sys_val.could_ip[0],g_sys_val.could_ip[1],g_sys_val.could_ip[2],g_sys_val.could_ip[3]);
    g_sys_val.dns_resend_cnt =0;
}

void dns_couldip_chk_send(){
    //xtcp_debug_printf("send dns\n");
    user_sending_len = dns_couldip_chk_build();
    #if 0
    for(uint8_t i=0;i<user_sending_len;i++){
        xtcp_debug_printf("%2x ",xtcp_tx_buf[i]);
    }
    xtcp_debug_printf("\n");
    #endif
    user_xtcp_send(g_sys_val.dns_conn  ,0);    
}

void dns_twominute_chk(){
    g_sys_val.dns_timecnt++;
    // 重发机制
    if(g_sys_val.dns_resend_cnt!=0){
        g_sys_val.dns_resend_cnt--;
        dns_couldip_chk_send();
    }
    //
    if(g_sys_val.dns_timecnt>120){   // 2分钟更新一次DNS服务
         g_sys_val.dns_timecnt=0;
         g_sys_val.dns_resend_cnt=3;
         dns_couldip_chk_send();
    }
}



