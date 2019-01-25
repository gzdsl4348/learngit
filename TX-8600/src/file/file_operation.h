#ifndef _FILE_OPERATION_T_
#define _FILE_OPERATION_T_

#include <stdint.h>

uint8_t mf_copy(uint8_t *psrc,uint8_t *pdst, uint8_t *pcurpct, uint8_t *pexit, uint32_t totsize,uint32_t cpdsize,uint8_t fwmode);

uint8_t mf_mkdir(uint8_t*pname);

uint8_t mf_unlink(uint8_t *pname);

uint8_t mf_rename(uint8_t *oldname,uint8_t* newname);

#endif

