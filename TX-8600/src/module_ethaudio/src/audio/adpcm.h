#ifndef __ADPCM_h__
#define __ADPCM_h__

#include <stdint.h>

struct adpcm_state {
    int32_t    valprev;    /* Previous output value */
    int32_t    index;      /* Index into stepsize table */
};

#ifdef __XC__
uint8_t adpcm_coder(int32_t Sample,struct adpcm_state &state);
//int32_t adpcm_decoder(struct adpcm_state &state,uint8_t ADPCM_BUFF);
#endif

#endif //__ADPCM_h__

