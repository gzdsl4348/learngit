#include "could_serve.h"
#include "ack_build.h"
#include "list_instance.h"
#include "user_xccode.h"
#include "user_unti.h"
#include "account.h"
#include "debug_print.h"

#define CLD_HEART_TIME_CNT  10 //10√Î–ƒÃ¯

void could_heart_send_timer(){
    static uint8_t heeart_timer_cnt=0;
    g_sys_val.could_heart_timcnt++;
    if(g_sys_val.could_heart_timcnt>CLD_HEART_TIME_CNT){
        g_sys_val.could_heart_timcnt = 0;
        debug_printf("cld id %d\n",g_sys_val.could_conn.id);
        if(g_sys_val.could_conn.id!=0){
            user_sending_len = cld_heart_build();
            user_could_send(1);  
            //text 
            #if 1
            user_could_send(1);  
            user_could_send(1);  
            #endif
            debug_printf("send cld\n");
            #if 0
            if(tmp1>=2){
                debug_printf("send colud\n");
                xtcp_rx_buf[POL_ID_BASE] = 1;
                account_list_updat();
                tmp1=0;
            }
            #endif
            g_sys_val.could_send_cnt++;
            if(g_sys_val.could_send_cnt>3){    //30√Î÷ÿ¡¨
                g_sys_val.could_conn.id=0;
                debug_printf("could send time over\n");
                user_xtcp_close(g_sys_val.could_conn);
                //user_xtcp_unlisten(g_sys_val.colud_port);
                g_sys_val.colud_connect_f=0;
            }
        }
        else if(g_sys_val.colud_connect_f==0){
            g_sys_val.colud_connect_f=1;
            g_sys_val.could_send_cnt = 0;
            debug_printf("colud connect\n");
            //user_xtcp_unlisten(g_sys_val.colud_port);
            user_xtcp_connect_tcp(g_sys_val.could_ip);
        }
    }
}



