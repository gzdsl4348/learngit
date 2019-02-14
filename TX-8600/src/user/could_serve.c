#include "could_serve.h"
#include "ack_build.h"
#include "list_instance.h"
#include "user_xccode.h"
#include "user_unti.h"
#include "account.h"
#include "debug_print.h"

#define CLD_HEART_TIME_CNT  2

void could_heart_send_timer(){
    static uint8_t heeart_timer_cnt=0;
    heeart_timer_cnt++;
    if(heeart_timer_cnt>CLD_HEART_TIME_CNT){
        heeart_timer_cnt = 0;
        //debug_printf("could id %d\n",g_sys_val.could_conn.id);
        if(g_sys_val.could_conn.id!=0){
            static uint8_t tmp=0;
            tmp++;
            if(tmp>=5){
                user_sending_len = cld_heart_build();
                user_could_send(1);  
                tmp=0;
            }
            g_sys_val.could_send_cnt++;
            if(g_sys_val.could_send_cnt>30){
                g_sys_val.could_conn.id=0;
                debug_printf("could send time over\n");
                user_xtcp_close(g_sys_val.could_conn);
                user_xtcp_unlisten(g_sys_val.colud_port);
                g_sys_val.colud_connect_f=0;
            }
        }
        else if(g_sys_val.colud_connect_f==0){
            g_sys_val.colud_connect_f=1;
            g_sys_val.could_send_cnt = 0;
            debug_printf("colud connect\n");
            user_xtcp_connect_tcp(g_sys_val.could_ip);
        }
    }
}


