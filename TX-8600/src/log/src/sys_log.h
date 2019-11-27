/*
*  File Name:remotel_business_base.h
*  Created on: 2019年5月26日
*  Author: caiws
*  description :后台日志记录功能
*  Modify date: 
* 	Modifier Author:
*  description :
*/
#ifndef __SYSLOG_H_
#define __SYSLOG_H_

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

/*---------------------------------------------------------------------------------------
*@description: debug打印与网络打印
*@Author: ljh
*@param[out]: 无
*@param[in]: 文本信息
*@return: 无
---------------------------------------------------------------------------------------*/
void xtcp_debug_printf(char fmt[], ...);

#define xtcp_debug_printf(...)  xtcp_debug_printf(__VA_ARGS__)
//=====================================================================
typedef struct log_info_t{
    char buff[512];
    unsigned len;
}log_info_t;

/*
*@description: 日志信息转换函数
*@Author: ljh
*@param[out]: 无
*@param[in]: 文本信息
*@return: log_info_t指针=》buff：本信息/len：文本长度
*/
#ifdef __XC__ 
log_info_t *unsafe log_info_chang(char * fmt, ...);
int itoa_forutf16(unsigned n, char *unsafe buf, unsigned base, int fill);
// 字符转换
int log_itoa(unsigned n, char *unsafe buf, unsigned base, int fill);

void log_reverse_array(char buf[], unsigned size);
#else
log_info_t* log_info_chang(char * fmt, ...);
int itoa_forutf16(unsigned n, char *buf, unsigned base, int fill);
// 字符转换
int log_itoa(unsigned n, char * buf, unsigned base, int fill);

void log_reverse_array(char buf[], unsigned size);
#endif

//=======================================================================

#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__SYSLOG_H_

