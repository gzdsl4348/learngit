#include "user_wifi_contorl.h"
#include "user_xccode.h"
#include "debug_print.h"
#include "fl_buff_decode.h"
#include "sys_log.h"

void dhcp_dis(){
    char disdhcp[]={0x61,0x74,0x2B,0x44,0x68,0x63,0x70,0x64,0x3D,0x30,0x0D,0x0A}; 
    user_lan_uart0_tx(disdhcp,12,0);
}

void dhcp_en(){
    char endhcp[]={0x61,0x74,0x2B,0x44,0x68,0x63,0x70,0x64,0x3D,0x31,0x0D,0x0A}; 
    user_lan_uart0_tx(endhcp,12,0);
}

void wifi_save(){
    char wifisave[]={0x61,0x74,0x2B,0x53,0x61,0x76,0x65,0x3D,0x31,0x0D,0x0A}; 
    user_lan_uart0_tx(wifisave,11,0);
}

void wifi_apply(){
    char wifiapply[]={0x61,0x74,0x2B,0x41,0x70,0x70,0x6C,0x79,0x3D,0x31,0x0D,0x0A}; //12
    user_lan_uart0_tx(wifiapply,12,0);
}


// mode=0:设置ip     mode=1:设置掩码
void wifi_ipset(uint8_t ip[],uint8_t mode){
    uint8_t ip_tmp[32];
    #define IPSET_LEN 9
    char wifi_ipset[]={0x61,0x74,0x2B,0x4C,0x41,0x4E,0x49,0x70,0x3D}; 
    #define MASKSET_LEN 13
    char wifi_maskset[]={0x61,0x74,0x2B,0x4C,0x41,0x4E,0x49,0x70,0x4D,0x61,0x73,0x6B,0x3D};
    //
    uint8_t ip_adrbase;
    if(mode){
        memcpy(&ip_tmp[0],wifi_maskset,MASKSET_LEN);
        ip_adrbase = MASKSET_LEN;
    }
    else{
        memcpy(&ip_tmp[0],wifi_ipset,IPSET_LEN);
        ip_adrbase = IPSET_LEN;
    }
    uint8_t tmp_f;
    for(uint8_t i=0;i<4;i++){
        tmp_f=0;
        if(ip[i]/100 != 0){
            ip_tmp[ip_adrbase] = ip[i]/100+0x30;
            ip_adrbase++;
            tmp_f = 1;
        }
        if(((ip[i]%100)/10!=0)||tmp_f){
            ip_tmp[ip_adrbase] = (ip[i]%100)/10+0x30;
            ip_adrbase++;
        } 
        ip_tmp[ip_adrbase] = ip[i]%10 + 0x30;
        ip_adrbase++;
        if(i!=3){
            ip_tmp[ip_adrbase] = 0x2E;
            ip_adrbase++;
        }
    }
    //增加结束符
    ip_tmp[ip_adrbase] = 0x0D;
    ip_tmp[ip_adrbase+1] = 0x0A;
    ip_adrbase+=2;
    /*
    for(uint8_t i=0;i<ip_adrbase;i++){
        xtcp_debug_printf("%x ",ip_tmp[i]);
    }
    */
    user_lan_uart0_tx(ip_tmp,ip_adrbase,0);
}

void wifi_contorl_mode(){
    if(g_sys_val.wifi_contorl_state==0){
        return;
    }
    g_sys_val.wifi_timer++;
    switch(g_sys_val.wifi_contorl_state){
        case WIFI_WAIT_POWERON:
            if(g_sys_val.wifi_timer>120){ // 等10秒wifi开启
                g_sys_val.wifi_timer=0;
                // 进入AT模式
                g_sys_val.wifi_contorl_state = WIFI_AT_ENTER;
                g_sys_val.wifi_io_tmp ^= D_IO_WIFI_CONTORL;
                wifi_ioset(g_sys_val.wifi_io_tmp);
                //xtcp_debug_printf("enter at\n");
            }
            break;
        case WIFI_AT_ENTER:
            if(g_sys_val.wifi_timer>5){ // 等0.5秒 进入AT模式 
                g_sys_val.wifi_timer=0;
                // 进入DCHP控制模式
                g_sys_val.wifi_contorl_state = WIFI_AT_COM_DHCP;
                g_sys_val.wifi_io_tmp |= D_IO_WIFI_CONTORL;
                wifi_ioset(g_sys_val.wifi_io_tmp);
                //xtcp_debug_printf("at ouit\n");
            }
            break;
        case WIFI_AT_COM_DHCP:
            if(g_sys_val.wifi_timer>5){ // 等0.5秒 配置DHCP 
                g_sys_val.wifi_timer=0;
                if(g_sys_val.wifi_mode==WIFI_DHCPDIS_MODE){
                    // 配置DHCP
                    dhcp_dis();
                    dhcp_disp_dis();
                    g_sys_val.sys_dhcp_state_tmp=0;  
                    //xtcp_debug_printf("dhcp dis\n");
                }
                else{
                    // 配置DHCP
                    dhcp_en();
                    g_sys_val.sys_dhcp_state_tmp=1;  
                    //xtcp_debug_printf("dhcp en\n");
                }
                // 进入配置wifi模式
                g_sys_val.wifi_contorl_state = WIFI_LANIP_SET;
            }
            break;
        case WIFI_LANIP_SET:
            if(g_sys_val.wifi_timer>5){ // 等0.5秒 配置IP
                uint8_t ip_tmp[4];
                g_sys_val.wifi_timer = 0;
                memcpy(ip_tmp,host_info.ipconfig.ipaddr,4);
                if(g_sys_val.wifi_mode==WIFI_DHCPDIS_MODE){
                    ip_tmp[2]++;
                }
                else{
                    ip_tmp[3]++;
                }   
                wifi_ipset(ip_tmp,0);
                //xtcp_debug_printf("set lanip %d,%d,%d,%d\n",ip_tmp[0],ip_tmp[1],ip_tmp[2],ip_tmp[3]);
                g_sys_val.wifi_contorl_state = WIFI_AT_SAVE;
            }
            break;
        case WIFI_AT_SAVE:
            if(g_sys_val.wifi_timer>5){ // 等0.5秒 保存设置
                g_sys_val.wifi_timer = 0;
                wifi_save();
                if(host_info.sys_dhcp_state != g_sys_val.sys_dhcp_state_tmp){
                    host_info.sys_dhcp_state = g_sys_val.sys_dhcp_state_tmp;
                    g_sys_val.wifi_contorl_state = WIFI_AT_APPLY;
                    // flash info   
                    fl_hostinfo_write();    //烧写主机信息
                }
                else{
                    g_sys_val.wifi_contorl_state=0;
                }
                xtcp_debug_printf("wifi save\n");
            }
            break;
        case WIFI_AT_APPLY:
            if(g_sys_val.wifi_timer>5){
                g_sys_val.wifi_timer = 0;
                wifi_apply();
                //xtcp_debug_printf("wifi apply\n");
                g_sys_val.wifi_contorl_state = 0;
            }
            break;
    }
}

void wifi_open(){
    // 开启wifi模块
    g_sys_val.key_wait_release = KEY_WIFI_RELEASE;
    g_sys_val.wifi_contorl_state = WIFI_WAIT_POWERON;
    g_sys_val.wifi_timer = 0;
    g_sys_val.wifi_mode = WIFI_DHCPDIS_MODE;
    g_sys_val.wifi_io_tmp = D_IO_WIFI_POWER|D_IO_WIFI_CONTORL;
    user_lan_uart0_tx(&g_sys_val.wifi_io_tmp,0,1);
    wifi_ioset(g_sys_val.wifi_io_tmp);      
}

