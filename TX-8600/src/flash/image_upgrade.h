#ifndef _IMAGE_UPGRADE_H_
#define _IMAGE_UPGRADE_H_
#include "flash_user.h"
#include "kfifo.h"


enum {
    IMAGE_UPGRADE_IDLE = 0,
    IMAGE_UPGRADING
};

#define SUCCEED_REPLY (0)
#define INVALID_REPLY (-1)

#define UPDATE_IAMGE_ERROR_START_CRC    0x01
#define UPDATE_IAMGE_ERROR_NOT_FACTORY  0x02
#define UPDATE_IAMGE_ERROR_START_IMAGE  0x03
#define UPDATE_IAMGE_ERROR_WRITE_IMAGE  0x04
#define UPDATE_IAMGE_ERROR_STOP_CRC     0x05
#define UPDATE_IAMGE_ERROR_END_IMAGE    0x06
#define UPDATE_IAMGE_ERROR_TIMEOUT      0x10

#define IMAGE_UPGRADE_FIFO_SIZE 512


typedef struct image_upgrade_mgr_t
{
    unsigned int state;
    fl_BootImageInfo image_info;    
    unsigned int image_max_size;
    unsigned int writed_size;
    unsigned int timeout_tick;

    char last_page_flag;
    
    int end_reply;

    kfifo_t kf;
}image_upgrade_mgr_t;

void _image_upgrade_init();
void _get_image_upgrade_reply(int &event, int &data);
int  _begin_image_upgrade(unsigned int image_size);
void _stop_image_upgrade(int error);
void _image_upgrade_poll(server image_upgrade_if i_image, int interval_ms);

extern image_upgrade_mgr_t g_upgrade_manager;

#endif
