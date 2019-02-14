#include "image_upgrade.h"
#include "debug_print.h"


#define FLASH_PAGE_SIZE 256

extern unsigned int start_image_crc(unsigned char page_256[]);
extern void put_image_crc(unsigned char page_256[], unsigned int size);
extern unsigned int stop_image_crc();

static inline unsigned int nibblerev(unsigned int data)
{
    return ((data<<4)&0xf0f0f0f0) | ((data>>4)&0x0f0f0f0f);
}

image_upgrade_mgr_t gt_ium;

void _image_upgrade_init()
{
    static unsigned char kfifo_buff[IMAGE_UPGRADE_FIFO_SIZE];
    
    gt_ium.state = IMAGE_UPGRADE_IDLE;
    
    memset(&gt_ium.image_info, 0, sizeof(fl_BootImageInfo));

    gt_ium.last_page_flag = 0;
    
    gt_ium.writed_size = 0;
    
    gt_ium.timeout_tick = 0;

    gt_ium.end_reply = INVALID_REPLY;

    KFIFO_INIT(gt_ium.kf, kfifo_buff, IMAGE_UPGRADE_FIFO_SIZE);
}


void _get_image_upgrade_reply(int &event, int &data)
{
    if(gt_ium.end_reply > INVALID_REPLY)
    {
        event = UPGRADE_END_REPLY;
        data = gt_ium.end_reply;
        gt_ium.end_reply = INVALID_REPLY;
    }
    else 
    {
        event = UPGRADE_BUFF_SIZE;
        data = IMAGE_UPGRADE_FIFO_SIZE-KFIFO_SIZE(gt_ium.kf);
    }
}

int _begin_image_upgrade(unsigned int image_size)
{
    //init image flags data
    KFIFO_CLEAR(gt_ium.kf);
    
    gt_ium.state = IMAGE_UPGRADING;
    
    gt_ium.last_page_flag = 0;
    
    memset(&gt_ium.image_info, 0, sizeof(gt_ium.image_info));
    gt_ium.timeout_tick = 0;
    gt_ium.end_reply = INVALID_REPLY;
    gt_ium.writed_size = 0;
    
    gt_ium.image_max_size = image_size;
    debug_printf("_begin_image_upgrade size:%d\n", gt_ium.image_max_size);
    return 0;
}

void _stop_image_upgrade(int error)
{
    gt_ium.state = IMAGE_UPGRADE_IDLE;
}
static int get_next_image_startaddress(fl_BootImageInfo image_info)
{
    image_info.startAddress = image_info.startAddress+image_info.size;
#if 0
    if((image_info.startAddress%(4*1024)) != 0)
    {
        image_info.startAddress += (4*1024)-(image_info.startAddress%(4*1024));
    }
    return image_info.startAddress;
#endif

    return (image_info.startAddress+4096-1) & (~(4096-1));
}

static int begin_image_deal()
{
    int image_num = 0;
    int wait_res = 0;
    int reply = INVALID_REPLY;
    do {
        if(fl_getFactoryImage(gt_ium.image_info) != 0)
        {
            debug_printf("fl_getFactoryImage error\n");
            reply = UPDATE_IAMGE_ERROR_NOT_FACTORY; 
            break;
        }
        
        if(gt_ium.image_max_size<10*1024)
        {
           gt_ium.image_max_size = gt_ium.image_info.size+20*1024;
           debug_printf("image_max_size %d\n", gt_ium.image_max_size);
        }
        
        image_num = 1;
        
        while(fl_getNextBootImage(gt_ium.image_info) == 0) image_num++;
        
        do
        {
            if(image_num >= 2)  //replace upgrade image
                wait_res = fl_startImageReplace(gt_ium.image_info, gt_ium.image_max_size);
            else //add upgrade image back in factory
                wait_res = fl_startImageAdd(gt_ium.image_info, gt_ium.image_max_size, 0);
            if(wait_res == 0)
            {
                break;
            }
            if(wait_res != 1)
            {
                reply = UPDATE_IAMGE_ERROR_START_IMAGE;
                break;
            }
        }while (wait_res);
        
        if(image_num == 1)
        {
            gt_ium.image_info.factory = 0;
            gt_ium.image_info.version = 1;            
            gt_ium.image_info.startAddress = get_next_image_startaddress(gt_ium.image_info);
            gt_ium.image_info.size = gt_ium.writed_size;
        }
        
        debug_printf("image: %d %d %d %d\n", gt_ium.image_info.startAddress, gt_ium.image_info.size, gt_ium.image_info.version, gt_ium.image_info.factory);
        
    }while(0);
    
    debug_printf("FLASH:StartImageUpgrade %d\n", reply);

    return reply;

}

static void upgrading_image_poll_deal(server image_upgrade_if i_image, int interval_ms)
{
    unsigned char page[FLASH_PAGE_SIZE];
    unsigned int got_len = 0;
    
    do {
        if((gt_ium.timeout_tick++)>6000/interval_ms)//6s
        {
            gt_ium.end_reply = UPDATE_IAMGE_ERROR_TIMEOUT;
            break;
        }
        
        if(gt_ium.last_page_flag==0 && KFIFO_SIZE(gt_ium.kf)<FLASH_PAGE_SIZE)
            break;
    
        gt_ium.timeout_tick = 0;
    
        //get data
        got_len = _kfifo_get(gt_ium.kf, page, FLASH_PAGE_SIZE);
        if(got_len!=0 && got_len<FLASH_PAGE_SIZE)
        {
            debug_printf("got_len(%u) < FLASH_PAGE_SIZE \n", got_len);
            memset(page+got_len, 0, FLASH_PAGE_SIZE-got_len);
        }
   
        //start image crc
        if(gt_ium.writed_size == 0)
        {

            if(start_image_crc(page) != 0)
            {
                gt_ium.end_reply = UPDATE_IAMGE_ERROR_START_CRC;
                break;
            }
            
            gt_ium.end_reply = begin_image_deal();
            
            gt_ium.image_info.size = nibblerev(page[0x10]|(page[0x11]<<8)|(page[0x12]<<16)|(page[0x13]<<24));
            debug_printf("FLASH:image_info.size = %u\n", gt_ium.image_info.size);

            if(gt_ium.end_reply != INVALID_REPLY)
                break;
            
        }
        else if(gt_ium.image_info.size >= gt_ium.writed_size)
        {
            unsigned int wlen = got_len;
            if(gt_ium.image_info.size < (gt_ium.writed_size+got_len))
            {
                wlen = gt_ium.image_info.size - gt_ium.writed_size;
            }
            put_image_crc(page, wlen);
        }

        //write flash
        if(got_len!=0 && fl_writeImagePage (page)!=0)
        {
            gt_ium.end_reply = UPDATE_IAMGE_ERROR_WRITE_IMAGE;
            break;
        }
    
        gt_ium.writed_size+=got_len;
    
    }while(KFIFO_SIZE(gt_ium.kf));


    if(gt_ium.end_reply!=SUCCEED_REPLY && 
        gt_ium.end_reply!=INVALID_REPLY && 
        gt_ium.end_reply>=3)
    {
        //delete error image data
        if(gt_ium.writed_size && gt_ium.image_info.startAddress!=0)
            fl_deleteImage(gt_ium.image_info);
        fl_endWriteImage();
    }
    else if(gt_ium.last_page_flag==1 && KFIFO_SIZE(gt_ium.kf)==0)
    {
        //end write image, check image crc
        if(stop_image_crc() != 0)
        {
            debug_printf("delete image: %d %d %d %d\n", gt_ium.image_info.startAddress, gt_ium.image_info.size, gt_ium.image_info.version, gt_ium.image_info.factory);
            if(gt_ium.image_info.startAddress!=0)
                fl_deleteImage(gt_ium.image_info);
            
            fl_endWriteImage();
            gt_ium.end_reply = UPDATE_IAMGE_ERROR_STOP_CRC;
        }
        else if(fl_endWriteImage() != 0)
        {
            gt_ium.end_reply = UPDATE_IAMGE_ERROR_END_IMAGE;
        }
        
        if(gt_ium.end_reply == INVALID_REPLY)
        {
            gt_ium.end_reply = SUCCEED_REPLY;
        }
    }

    //end_reply
    if(gt_ium.end_reply > INVALID_REPLY)
    {
        debug_printf("FLASH: end_reply result %d\n", gt_ium.end_reply);
        gt_ium.state = IMAGE_UPGRADE_IDLE;    
        i_image.image_upgrade_ready();
    }
    //buff_size
    else if(got_len!=0 && KFIFO_SIZE(gt_ium.kf)==0)
    {
        i_image.image_upgrade_ready();
    }
}

void _image_upgrade_poll(server image_upgrade_if i_image, int interval_ms)
{
    if(gt_ium.state == IMAGE_UPGRADING)
    {
        upgrading_image_poll_deal(i_image, interval_ms);
    }
}

