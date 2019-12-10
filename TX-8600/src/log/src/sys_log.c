#include "sys_log.h" 
#include <debug_print.h>
#include <print.h>
#include <stdarg.h>
#include <syscall.h>
#include <limits.h>
#include <print.h>
#include <string.h>
#include <ctype.h>
#include "user_xccode.h"
#include "sys_config_dat.h"
#include "user_unti.h"

#define MAX_INT_STRING_SIZE 10

#ifndef DEBUG_PRINTF_BUFSIZE
#define DEBUG_PRINTF_BUFSIZE 1000
#endif

char g_user_debug_buf[DEBUG_PRINTF_BUFSIZE];

// 反转数组排列
void log_reverse_array(char buf[], unsigned size)
{
  int begin = 0;
  int end = size - 1;
  int tmp;
  for (;begin < end; begin++,end--) {
    tmp = buf[begin];
    buf[begin] = buf[end];
    buf[end] = tmp;
  }
}
// 字符转换
int log_itoa(unsigned n, char *buf, unsigned base, int fill)
{
  static const char digits[] = "0123456789ABCDEF";
  unsigned i = 0;

  if (n == 0&& fill==0)
    fill += 1;

  while (n > 0) {
    unsigned next = n / base;
    unsigned cur  = n % base;
    buf[i] = digits[cur];
    i += 1;
    fill--;
    n = next;
  }
  for (;fill > 0; fill--) {
    buf[i] = '0';
    i++;
  }
  log_reverse_array(buf, i);
  return i;
}
// utf16字符转换
int itoa_forutf16(unsigned n, char *buf, unsigned base, int fill)
{
  static const char digits[] = "0123456789ABCDEF";
  unsigned i = 0;

  if (n == 0&& fill==0)
    fill += 1;

  while (n > 0) {
    unsigned next = n / base;
    unsigned cur  = n % base;
  
    buf[i] = 0;  
    i++;
    
    buf[i] = digits[cur];
    i += 1;
    
    fill--;
    n = next;
  }
  for (;fill > 0; fill--) {
    //    
    buf[i] = 0;  
    i++;

    buf[i] = '0';
    i++;
    //
    }
  log_reverse_array(buf, i);
  return i;
}

#if 1
//-----------------------------------------------------------------------------------------
// DEBUG打印及网络打印
//-----------------------------------------------------------------------------------------
void xtcp_debug_printf(char * fmt, ...)
{
  char width = 0;
  char * marker;
  int intArg;
  unsigned int uintArg;
  char * strArg;
  char *end = &g_user_debug_buf[DEBUG_PRINTF_BUFSIZE - 1 - MAX_INT_STRING_SIZE];

  va_list args;

  va_start(args,fmt);
  marker = fmt;
  char *p = g_user_debug_buf;
  while (*fmt) {
    if (p > end) {
      // flush
      _write(FD_STDOUT, g_user_debug_buf, p - g_user_debug_buf);
      p = g_user_debug_buf;
    }
    switch (*fmt) {
    case '%':
      fmt++;
      if (*(fmt) == '-' || *(fmt) == '+' || *(fmt) == '#' || *(fmt) == ' ') {
        // Ignore flags
        fmt++;
      }
      while (*(fmt) && *(fmt) >= '0' && *(fmt) <= '9') {
        // Ignore width
        width = *(fmt)-'0';
        fmt++;
      }
      // Use 'tolower' to ensure both %x/%X do something sensible
      switch (tolower(*(fmt))) {
      case 'd':
        intArg = va_arg(args, int);
        if (intArg < 0) {
          *p++ = '-';
          intArg = -intArg;
        }
        p += log_itoa(intArg, p, 10, width);
        break;
      case 'u':
        uintArg = va_arg(args, int);
        p += log_itoa(uintArg, p, 10, width);
        break;
      case 'p':
      case 'x':
        uintArg = va_arg(args, int);
        p += log_itoa(uintArg, p, 16, width);
        break;
      case 'c':
        intArg = va_arg(args, int);
        *p++ = intArg;
        break;
      case 's':
        strArg = va_arg(args, char *);
        int len = strlen(strArg);
        if (len > (end - g_user_debug_buf)) {
                // flush
          _write(FD_STDOUT, g_user_debug_buf, p - g_user_debug_buf);
          p = g_user_debug_buf;
        }
        if (len > (end - g_user_debug_buf))
          len = end - g_user_debug_buf;
        memcpy(p, strArg, len);
        p += len;
        break;
      default:
        break;
      }
      width = 0;
      break;

    default:
      *p++ = *fmt;
    }
    fmt++;
  }
  _write(FD_STDOUT, g_user_debug_buf, p - g_user_debug_buf);
  va_end(args);
  if(g_sys_val.eth_debug_f){
      user_xtcp_debugudpsend((uint8_t *)g_user_debug_buf,p - g_user_debug_buf);
  }
  return;
}

#endif

//-----------------------------------------------------------------------------------------
// 日志信息转换
//-----------------------------------------------------------------------------------------
log_info_t g_log_info;

log_info_t* log_info_chang(char * fmt, ...){
    char width = 0;
    char * marker;
    int intArg;
    unsigned int uintArg;
    char * strArg;
    char *end = &g_user_debug_buf[DEBUG_PRINTF_BUFSIZE - 1 - MAX_INT_STRING_SIZE];
    
    va_list args;
    
    va_start(args,fmt);
    marker = fmt;
    char *p = g_user_debug_buf;
    while (*fmt) {
      if (p > end) {
        // flush
        //_write(FD_STDOUT, g_user_debug_buf, p - g_user_debug_buf);
        p = g_user_debug_buf;
      }
      switch (*fmt) {
      case '%':
        fmt++;
        if (*(fmt) == '-' || *(fmt) == '+' || *(fmt) == '#' || *(fmt) == ' ') {
          // Ignore flags
          fmt++;
          *(fmt)=0;
          fmt++;
        }
        while (*(fmt) && *(fmt) >= '0' && *(fmt) <= '9') {
          // Ignore width
          width = *(fmt)-'0';
          fmt++;
          *(fmt)=0;
          fmt++;  
        }
        // Use 'tolower' to ensure both %x/%X do something sensible
        switch (tolower(*(fmt))) {
        case 'd':
          intArg = va_arg(args, int);
          if (intArg < 0) {
            *p++ = '-';
            intArg = -intArg;
          }
          p += itoa_forutf16(intArg, p, 10, width);
          break;
        case 'u':
          uintArg = va_arg(args, int);
          p += itoa_forutf16(uintArg, p, 10, width);
          break;
        case 'p':
        case 'x':
          uintArg = va_arg(args, int);
          p += itoa_forutf16(uintArg, p, 16, width);
          break;
        case 'c':
          intArg = va_arg(args, int);
          *p++ = intArg;
          *p++ = 0;
          break;
        case 's':
          strArg = va_arg(args, char *);
          int len = strlen(strArg);
          if (len*2 > (end - g_user_debug_buf)) {
                  // flush
            //_write(FD_STDOUT, g_user_debug_buf, p - g_user_debug_buf);
            p = g_user_debug_buf;
          }
          if (len*2 > (end - g_user_debug_buf))
            len = end - g_user_debug_buf;
          for(unsigned i=0;i<len;i++){
              *p++ = strArg[i];
              *p++ = 0;
          }
          break;
        case 'l':
          strArg = va_arg(args, char *);
          if ( p+64 > end) 
            break;
          for(unsigned i=0;i<64/2;i++){
              if(strArg[i*2]==0 && strArg[i*2+1]==0)
                break;
              *p++ = strArg[i*2];
              *p++ = strArg[i*2+1];
          }
          break;
        default:
          break;
        }
        width = 0;
        break;
    
      default:
        *p++ = *fmt;
        *p++ = 0;
      }
      fmt++;
    }
    //_write(FD_STDOUT, g_user_debug_buf, p - g_user_debug_buf);
    va_end(args);
    
    g_log_info.len = p - g_user_debug_buf;
    memcpy(g_log_info.buff,g_user_debug_buf,p - g_user_debug_buf);
    #if 0
    for(uint8_t i=0;i<g_log_info.len;i++){
        debug_printf("%x ",g_log_info.buff[i]);
    }
    debug_printf("\n");
    #endif
    return &g_log_info; 
}


