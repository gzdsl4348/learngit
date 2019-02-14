#ifndef  __SRC_FIFO_H_
#define  __SRC_FIFO_H_

#include "stdint.h"

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

typedef struct kfifo_t{
	#ifdef __XC__
	char *unsafe buffer;
	#else
	char *buffer;
	#endif
	unsigned int size;
	unsigned int wptr;
	unsigned int rptr;
}kfifo_t;

unsigned int kfifo_put(kfifo_t *fifo,char *buffer, unsigned len); 	

unsigned int kfifo_get(kfifo_t *fifo,char *buffer, unsigned len);

#if defined(__cplusplus) || defined(__XC__)
}
#endif


#endif //__SRC_FIFO_H_

