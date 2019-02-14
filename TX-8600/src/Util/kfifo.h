/*
 * kfifo.h
 *
 *  Created on: Sep 8, 2017
 *      Author: root
 */


#ifndef KFIFO_H_
#define KFIFO_H_

#include <xccompat.h>
#include <string.h>

#ifdef __XC__
#define POINTER_PARAM(type, name) type *unsafe name

#define UNSAFE_IN   unsafe{
#define UNSAFE_OUT  }

#else
#define POINTER_PARAM(type, name) type *name

#define UNSAFE_IN   {
#define UNSAFE_OUT  }

#endif



#define MIN(a, b) (a)>(b)?(b):(a)

typedef struct kfifo_t
{
    unsigned int in_index;
    unsigned int out_index;
    unsigned int size;
    POINTER_PARAM(unsigned char, buffer);
} kfifo_t;

#define KFIFO_INIT(kf, buff, sz)    {UNSAFE_IN kf.in_index=kf.out_index=0; kf.buffer=buff; kf.size=sz; UNSAFE_OUT}

#define KFIFO_SIZE(kf)              (kf.in_index - kf.out_index)

#define KFIFO_FREE_SIZE(kf)          (kf.size - KFIFO_SIZE(kf))

#define KFIFO_CLEAR(kf)             {kf.in_index = kf.out_index;}

#define KFIFO_PUT(kf, buff, num) \
UNSAFE_IN \
    unsigned len = MIN(num, kf.size-kf.in_index+kf.out_index);\
    unsigned l = MIN(len, kf.size - (kf.in_index & (kf.size - 1)));\
    memcpy(kf.buffer+(kf.in_index & (kf.size - 1)), buff,  l);\
    memcpy(kf.buffer, buff+l, len - l);\
    kf.in_index += len;  \
UNSAFE_OUT

#define KFIFO_PUT_BYTE(kf, byte)\
UNSAFE_IN \
    kf.buffer[kf.in_index & (kf.size - 1)] = byte;\
    kf.in_index++;\
UNSAFE_OUT


#define KFIFO_GET(kf, buff, num) \
UNSAFE_IN \
    unsigned len = MIN(num, kf.in_index-kf.out_index);\
    unsigned l = MIN(len, kf.size - (kf.out_index & (kf.size - 1)));\
    memcpy(buff, 0, kf.buffer, (kf.out_index & (kf.size - 1)), l);\
    memcpy(buff, l, kf.buffer, 0, len-l);\
    kf.out_index += len;\
    num = len;\
UNSAFE_OUT


#ifdef __XC__
static inline unsigned int _kfifo_get(kfifo_t &kf, char buff[], unsigned int len)
{
    len = MIN(len, kf.in_index-kf.out_index);
    unsigned int l = MIN(len, kf.size - (kf.out_index & (kf.size - 1)));
    memcpy(buff, kf.buffer+(kf.out_index & (kf.size - 1)), l);
    memcpy(buff + l, kf.buffer, len-l);
    kf.out_index += len;
    return len;
}
#endif

#if 0
static inline unsigned int kfifo_get(kfifo_t &kf, char buff[], unsigned int len)
{
    len = MIN(len, kf.in_index-kf.out_index);
    unsigned int l = MIN(len, kf.size - (kf.out_index & (kf.size - 1)));
    memcpy(buff, kf.buffer+(kf.out_index & (kf.size - 1)), l);
    memcpy(buff + l, kf.buffer, len-l);
    kf.out_index += len;
    return len;
}

unsafe static inline void kfifo_init(volatile kfifo_t *unsafe p_f, unsigned size)
{
    p_f->size = size;
    p_f->in_index = p_f->out_index = 0;
}

unsafe static inline void kfifo_clear(volatile kfifo_t *unsafe p_f)
{
    p_f->in_index = p_f->out_index;
}

unsafe
static inline unsigned int kfifo_size(volatile kfifo_t *unsafe p_f)
{
    return p_f->in_index - p_f->out_index;
}
unsafe static inline unsigned int kfifo_put(volatile kfifo_t *unsafe p_f, const char buff[], unsigned int len)
{
    len = MIN(len, p_f->size-p_f->in_index+p_f->out_index);
    unsigned int l = MIN(len, p_f->size - (p_f->in_index & (p_f->size - 1)));
    memcpy(p_f->buffer + (p_f->in_index & (p_f->size - 1)), buff, l);
    memcpy(p_f->buffer, buff + l, len - l);
    p_f->in_index += len;
    return len;
}
unsafe static inline unsigned int kfifo_get(volatile kfifo_t *unsafe p_f, char buff[], unsigned int len)
{
    len = MIN(len, p_f->in_index-p_f->out_index);
    unsigned int l = MIN(len, p_f->size - (p_f->out_index & (p_f->size - 1)));
    memcpy(buff, p_f->buffer+(p_f->out_index & (p_f->size - 1)), l);
    memcpy(buff + l, p_f->buffer, len-l);
    p_f->out_index += len;
    return len;
}
#endif

#endif /* KFIFO_H_ */
