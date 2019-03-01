#include "checksum.h"

//==========================================================================
//Check Summer Moudle
uint16_t chksum_16bit(uint16_t sum, uint8_t *byte_data, uint16_t lengthInBytes) {
    uint16_t i=0;;
    unsigned s = sum;
	short *data = (short *)byte_data;
    for(i = 0; i < (lengthInBytes>>1); i++) {
        s += byterev(data[i]) >> 16;
    }
    if (lengthInBytes & 1) {
        s += byte_data[2*i] << 8;
    }
    s = ((s & 0xffff) + (s >> 16)) ;
    if(s>0xffff)
        s = ((s & 0xffff) + (s >> 16));
    //printf("%x\n",s);
	return ~s;
}

uint16_t chksum_8bit(uint16_t sum, uint8_t *byte_data, uint16_t lengthInBytes) {
    uint16_t i=0;;
    unsigned s = sum;
    for(i = 0; i < lengthInBytes; i++) {
        s += byte_data[i];
    }
    s = ((s & 0xffff) + (s >> 16)) ;
    while(s>0xffff)
        s = ((s & 0xffff) + (s >> 16));
    //printf("%x\n",s);
	return ~s;
}



