#include "bsp_ds1302.h"
#include "user_unti.h"
#include "user_xccode.h"
#include "flash_user.h"
#include "fl_buff_decode.h"

#include "debug_print.h"
#include "sys_log.h"

#include <xs1.h>
#include <platform.h>

on tile[1]: out port ds1302_CE = XS1_PORT_1H;
on tile[1]: out port ds1302_Clk = XS1_PORT_1I;
on tile[1]: port ds1302_IO = XS1_PORT_1J;

#define one_us 100 //100MHZ /100


static uint8_t tmp;
void ds1302_get_date(){
	//
	tmp = ds1302_contorl(ds1302_minute_read,0x00);
	g_sys_val.time_info.minute = ((tmp>>4)&0x07)*10 + (tmp&0x0F);	
	//
	tmp = ds1302_contorl(ds1302_hour_read,0x00);
	g_sys_val.time_info.hour = ((tmp&0x3F)>>4)*10+(tmp&0x0F);
	//
	tmp = ds1302_contorl(ds1302_date_read,0x00);
	g_sys_val.date_info.date = ((tmp>>4)&0x07)*10 + (tmp&0x0F);
	//
	tmp = ds1302_contorl(ds1302_month_read,0x00);
	g_sys_val.date_info.month = ((tmp>>4)&0x07)*10 + (tmp&0x0F);
	//
	tmp = ds1302_contorl(ds1302_year_read,0x00);
	g_sys_val.date_info.year = ((tmp>>4)&0x0F)*10 + (tmp&0x0F);
	//
	tmp = ds1302_contorl(ds1302_week_read,0x00);
	g_sys_val.date_info.week = (tmp&0x0F);
	//
	tmp = ds1302_contorl(ds1302_second_read,0x00);
	g_sys_val.time_info.second = ((tmp>>4)&0x07)*10 + (tmp&0x0F);	
	
}

void ds1302_time_set(){
	tmp = ((g_sys_val.time_info.second/10)<<4)|(g_sys_val.time_info.second%10);
	ds1302_contorl(ds1302_second_write,tmp);
	tmp = ((g_sys_val.time_info.minute/10)<<4)|(g_sys_val.time_info.minute%10);
	ds1302_contorl(ds1302_minute_write,tmp);	
	tmp = ((g_sys_val.time_info.hour/10)<<4)|(g_sys_val.time_info.hour%10);
	ds1302_contorl(ds1302_hour_write,tmp);	
}

void ds1302_date_set(){
	tmp = ((g_sys_val.date_info.date/10)<<4)|(g_sys_val.date_info.date%10);
	ds1302_contorl(ds1302_date_write,tmp);
	tmp = ((g_sys_val.date_info.month/10)<<4)|(g_sys_val.date_info.month%10);
	ds1302_contorl(ds1302_month_write,tmp);
	tmp = ((g_sys_val.date_info.year/10)<<4)|(g_sys_val.date_info.year%10);
	ds1302_contorl(ds1302_year_write,tmp); 
    tmp = (g_sys_val.date_info.week);
    ds1302_contorl(ds1302_week_write,tmp); 
}

void ds1302_init(){
	ds1302_contorl(0x8E,0x00);	// WP DISABLE
	//tmp = 0; //TIMER RESET
    //
	tmp = ds1302_contorl(ds1302_second_read,0x00);	// CH read
	//ds1302_contorl(0x90,0xA0);	
	if((tmp&0x80)!=0){
		ds1302_contorl(ds1302_minute_write,0x00);	
		ds1302_contorl(ds1302_hour_write,0x00);	
		ds1302_contorl(ds1302_date_write,0x01);
		ds1302_contorl(ds1302_month_write,0x01);
		ds1302_contorl(ds1302_year_write,0x01);
		ds1302_contorl(ds1302_week_write,0x01);
		ds1302_contorl(ds1302_second_write,0x00);	// SECOND RESET
		debug_printf("timer_reset");
	}
	ds1302_get_date();
    g_sys_val.today_date = g_sys_val.date_info;
    host_info.online_date_info = g_sys_val.date_info;
    // 打开日志记录
    g_sys_val.log_waitmk_f = user_file_mklog();
    debug_printf("mk flag %d\n",g_sys_val.log_waitmk_f);
}

uint8_t ds1302_contorl(uint8_t cmd,uint8_t data){
	timer	 tim_p;
	unsigned tim_tmp;
	uint8_t dat_tmp;
	tmp=cmd;
	ds1302_CE <: 0x01;
	//cmd
	for(uint8_t i=0;i<8;i++){
		ds1302_Clk <: 0x00;
		ds1302_IO <: >> cmd;
		tim_p :> tim_tmp;
		tim_p when timerafter(tim_tmp+one_us):> void;
		ds1302_Clk <: 0x01;
		tim_p :> tim_tmp;
		tim_p when timerafter(tim_tmp+one_us):> void;
	}		
	// read
	if((tmp &0x01)!=0){	
	    ds1302_IO <:0x01;
		set_port_drive_low(ds1302_IO); //上拉设置
		for(uint8_t i=0;i<8;i++){
			ds1302_Clk <: 0x01;
			tim_p :> tim_tmp;
			tim_p when timerafter(tim_tmp+one_us):> void;
			ds1302_Clk <: 0x00;
			tim_p :> tim_tmp;
			tim_p when timerafter(tim_tmp+one_us):> void;
			ds1302_IO :> >>dat_tmp;
		}		
	}
	// write
	else{
		for(uint8_t i=0;i<8;i++){
			ds1302_Clk <: 0x00;
			ds1302_IO <: >> data;
			tim_p :> tim_tmp;
			tim_p when timerafter(tim_tmp+one_us):> void;
			ds1302_Clk <: 0x01;
			tim_p :> tim_tmp;
			tim_p when timerafter(tim_tmp+one_us):> void;
		}		
	}
	ds1302_Clk <: 0x00;
	ds1302_IO <: 0x00;
	set_port_drive(ds1302_IO);
	ds1302_CE <: 0x00;
	return dat_tmp;
}


