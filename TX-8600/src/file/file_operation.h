#ifndef _FILE_OPERATION_T_
#define _FILE_OPERATION_T_

#include <stdint.h>

uint8_t mf_copy(uint8_t *psrc,uint8_t *pdst, uint8_t *pcurpct, uint8_t *pexit, uint32_t totsize,uint32_t cpdsize,uint8_t fwmode);

uint8_t mf_mkdir(uint8_t*pname);

uint8_t mf_unlink(uint8_t *pname);

uint8_t mf_rename(uint8_t *oldname,uint8_t* newname);

uint8_t mf_open_log(char *file_newname,char *file_oldname);

uint8_t mf_add_loginfo(char *file_name,unsigned len);

uint8_t file_contorl_init(uint8_t *psrc,uint8_t *pdst,uint8_t *pcurpct,uint8_t *pexit,uint8_t fwmode,uint8_t contorl_state);

void file_copy_process();

#endif

