#ifndef _MYSTRING_H_
#define _MYSTRING_H_

//typedef unsigned short wchar;
#define wchar   unsigned short

#ifdef __XC__
unsigned wstrcpy(wchar *wdst, const wchar *wsrc);
unsigned wstrcat(wchar *wdst, const wchar*wsrc);
#else
wchar * wstrcpy(wchar *wdst, const wchar *wsrc);
wchar *wstrcat(wchar *wdst, const wchar*wsrc);
#endif
int wstrlen(const wchar * wstr);
int wstrcmp(const wchar *wdst, const wchar*wsrc);
void ascii_to_unicode(wchar *wdst, const char *wsrc);
int utf8_to_unicode(const char* p_in, char* p_out);
void string_test();

#endif
