#ifndef _MUSIC_DECODER_CFG_H_
#define _MUSIC_DECODER_CFG_H_

typedef interface music_decoder_cfg_if {

//    void get_music_folders(unsigned char folders[n], unsigned int n, unsigned int &folder_num);
//    void get_music_files(unsigned char folder_name[n1], unsigned int n1, unsigned char files[n2], unsigned int n2, unsigned int &file_num);
    
    int music_decode_start(unsigned char ch, unsigned char f_name[n], static const unsigned n, unsigned f_offset);
    int music_decode_stop(unsigned char ch);
    int music_decode_play(unsigned char ch);
    int music_decode_pause(unsigned char ch);

    //·µ»Ø×´Ì¬
    [[clears_notification]] void get_status(int status[n], unsigned n);
    [[notification]] slave void status_changed();

}music_decoder_cfg_if;



#endif

