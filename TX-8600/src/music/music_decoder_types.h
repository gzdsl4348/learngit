#ifndef _MUSIC_DECODER_TYPES_H_
#define _MUSIC_DECODER_TYPES_H_

#define MUSIC_CHANNEL_NUM   48

typedef enum
{
    MUSIC_DECODER_STOP = 0,
    MUSIC_DECODER_START,
    MUSIC_DECODER_FILE_END,
    MUSIC_DECODER_ERROR1,
    MUSIC_DECODER_ERROR2
}MUSIC_DECODER_STATUS_E;


typedef struct 
{
    char status; // MUSIC_DECODER_STATUS_E
    char is_new; // 1 - ֪ͨ
}music_decoder_status_t;


#endif
