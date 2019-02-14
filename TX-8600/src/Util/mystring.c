#include "mystring.h"

wchar *wstrcat(wchar *wdst, const wchar*wsrc)
{
    wchar *p=wdst;  
    while(*wdst!=0)wdst++;
    while(*wsrc!=0)*wdst++=*wsrc++;
    *wdst=0;
    return p; 
}
int wstrcmp(const wchar *wdst, const wchar*wsrc)
{
    if((0 == wdst) || (0 == wsrc)) return 1;
    
    while (*wdst && *wsrc && (*wdst == *wsrc))
    {
        wdst++;
        wsrc++;
    }
    return *wdst - *wsrc;
}

int wstrlen(const wchar * wstr) 
{
    const wchar *cp =  wstr;
    while (*cp++);
    return (cp - wstr - 1);
	
}


wchar * wstrcpy(wchar *wdst, const wchar *wsrc)  
{
    if(wdst == 0 || wsrc == 0) return 0;   
    wchar *ret = wdst; 
    while(*wsrc != 0)
    {
        *ret++ = *wsrc++;
    }
    *ret = 0;
    return ret;
}


void ascii_to_unicode(wchar *wdst, const char *wsrc)
{
    while(*wsrc != 0)
    {
        *wdst++ = *wsrc++;
    }
    *wdst = 0;
}


/*************************************************************************************************
* 将UTF8编码转换成Unicode（UCS-2LE）编码  低地址存低位字节
* 参数：
*    char* p_in     输入字符串
*    char*p_out   输出字符串
* 返回值：转换后的Unicode字符串的字节数，如果出错则返回-1
**************************************************************************************************/
//utf8转unicode
int utf8_to_unicode(const char* p_in, char* p_out)
{
    char high;
    char low;
    char middle;
	int output_size = 0; //记录转换后的Unicode字符串的字节数
 
	while (*p_in)
	{
		if (*p_in > 0x00 && *p_in <= 0x7F) //处理单字节UTF8字符（英文字母、数字）
		{
			*p_out = *p_in;
			 p_out++;
			*p_out = 0; //小端法表示，在高地址填补0
		}
		else if (((*p_in) & 0xE0) == 0xC0) //处理双字节UTF8字符
		{
			high = *p_in;
			p_in++;
			low = *p_in;
			if ((low & 0xC0) != 0x80)  //检查是否为合法的UTF8字符表示
			{
                output_size = -1;
			    goto end;//如果不是则报错
			}
 
			*p_out = (high << 6) + (low & 0x3F);
			p_out++;
			*p_out = (high >> 2) & 0x07;
		}
		else if (((*p_in) & 0xF0) == 0xE0) //处理三字节UTF8字符
		{
			high = *p_in;
			p_in++;
			middle = *p_in;
			p_in++;
			low = *p_in;
			if (((middle & 0xC0) != 0x80) || ((low & 0xC0) != 0x80))
			{
                output_size = -1;
                goto end;
			}
			*p_out = (middle << 6) + (low & 0x3F);//取出middle的低两位与low的低6位，组合成unicode字符的低8位
			p_out++;
			*p_out = (high << 4) + ((middle >> 2) & 0x0F); //取出high的低四位与middle的中间四位，组合成unicode字符的高8位
		}
		else //对于其他字节数的UTF8字符不进行处理
		{
            output_size = -1;
			goto end;
		}
		p_in ++;//处理下一个utf8字符
		p_out ++;
		output_size += 2;
	}
end:    
	//unicode字符串后面，有两个\0
	*p_out = 0;
	 p_out++;
	*p_out = 0;
	return output_size;
}
#include "debug_print.h"
void string_test()
{
    const char utf8[] = "123涓枃123";
    char output[50] = {0};
    int size = utf8_to_unicode(utf8, output);
    debug_printf("string_test %d:\n", wstrlen((const wchar*)output));
    for(int i=0; i<size; i++)
    {
        debug_printf("%x ", output[i]);
    }
    debug_printf("\n\n");
}

