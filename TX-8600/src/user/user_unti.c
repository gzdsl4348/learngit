#include "user_unti.h"
#include "debug_print.h"

//-----------------------------------------------------------------------------------------------------
// 系统全局变量
g_sys_val_t g_sys_val;

//================================================================================
// 临时变量
tmp_union_t tmp_union;

//-----------------------------------------------------------------------------------------------------
uint8_t mac_cmp(uint8_t *a, uint8_t *b)
{
    return ((a[0]==b[0])&&(a[1]==b[1])&&(a[2]==b[2])&&(a[3]==b[3])&&(a[4]==b[4])&&(a[5]==b[5]));
}

uint8_t ip_cmp(uint8_t *a, uint8_t *b){
    //debug_printf("ip1 %d,%d,%d,%d\n",a[0],a[1],a[2],a[3]);
    //debug_printf("ip2 %d,%d,%d,%d\n",b[0],b[1],b[2],b[3]);
    return ((a[0]==b[0])&&(a[1]==b[1])&&(a[2]==b[2])&&(a[3]==b[3]));
}

uint8_t charncmp(uint8_t *c1,uint8_t *c2,unsigned len){
    for(unsigned i=0;i<len;i++){
        if(c1[i]!=c2[i])
            return 0;
    }
    return 1;
}


