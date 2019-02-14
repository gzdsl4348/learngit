#ifndef _MISIC_DECODER_SERVER_H_
#define _MISIC_DECODER_SERVER_H_


/*
音频解码后
pcm传输接口
1.一次全部通道传输
2.分次通道传输
3.都兼容
做一个标志位
*/
typedef interface music_decoder_output_if
{
    
    unsigned get_pcmbuff_active(unsigned char          ch, unsigned int &samplerate, unsigned char pcmbuff[n], unsigned n, unsigned &length);
    
}music_decoder_output_if;

[[combinable]]
void music_decoder_server(server interface music_decoder_output_if if_mdo);


#endif
