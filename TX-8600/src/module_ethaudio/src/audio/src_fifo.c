#include "src_fifo.h"
#include "string.h"

#define GETMIN_M(a,b)   (a>b)?b:a

unsigned int kfifo_put(kfifo_t *fifo,char *buffer, unsigned len) 	
{ 	
    unsigned l;	  
  
    len = GETMIN_M(len, fifo->size - fifo->wptr + fifo->rptr); 	
  
    // first put the data starting from fifo->in to buffer end  	
    l = GETMIN_M(len, fifo->size - (fifo->wptr & (fifo->size - 1)));
    memcpy(fifo->buffer + (fifo->wptr & (fifo->size - 1)), buffer, l);	   
  
    // then put the rest (if any) at the beginning of the buffer 	  
    memcpy(fifo->buffer, buffer + l, len - l);	 
  
    fifo->wptr += len;	   
  
    return len;	  
}    

unsigned int kfifo_get(kfifo_t *fifo,char *buffer, unsigned len)	 
{	 
    unsigned l;	   
   
    len = GETMIN_M(len, fifo->wptr - fifo->rptr);	 

    // first get the data from fifo->out until the end of the buffer 	   
    l = GETMIN_M(len, fifo->size - (fifo->rptr & (fifo->size - 1)));	
    memcpy(buffer, fifo->buffer + (fifo->rptr & (fifo->size - 1)), l);	 
   
    // then get the rest (if any) from the beginning of the buffer 
    memcpy(buffer + l, fifo->buffer, len - l);	
   
    fifo->rptr += len;	 
   
	return len;	   
}	 



