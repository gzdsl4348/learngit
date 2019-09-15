#include <xs1.h>
#include <timer.h>
#include <string.h>
#include <quadflashlib.h>
#include "image_upgrade.h"
#include "debug_print.h"

#define IMAGE_UPGRADE_DEBUG_ENBLE 0

#if IMAGE_UPGRADE_DEBUG_ENBLE 
#define DBG_PRINTF(...) debug_printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#endif

#define BYTE_ALIGNMENT_4096(x) (((x)+4096-1) & (~(4096-1)))

#define FLASH_PAGE_SIZE 256

extern unsigned int start_image_crc(unsigned char page_256[]);
extern void put_image_crc(unsigned char page_256[], unsigned int size);
extern unsigned int stop_image_crc();

static inline unsigned int nibblerev(unsigned int data)
{
    return ((data<<4)&0xf0f0f0f0) | ((data>>4)&0x0f0f0f0f);
}

image_upgrade_mgr_t g_upgrade_manager;

static unsigned g_boot_size = 0;
static fl_BootImageInfo g_delete_image = {0};

void _image_upgrade_init()
{
    static unsigned char kfifo_buff[IMAGE_UPGRADE_FIFO_SIZE];
    
    set_core_high_priority_on();
    
    g_upgrade_manager.state = IMAGE_UPGRADE_IDLE;
    
    memset(&g_upgrade_manager.image_info, 0, sizeof(fl_BootImageInfo));
	
    g_upgrade_manager.last_page_flag = 0;
    
    g_upgrade_manager.writed_size = 0;
    
    g_upgrade_manager.timeout_tick = 0;

    g_upgrade_manager.end_reply = INVALID_REPLY;
	
	g_boot_size = fl_getBootPartitionSize();
	memset(&g_delete_image, 0, sizeof(g_delete_image));
    
	DBG_PRINTF("IMAGE: g_boot_size:%d\n", g_boot_size);
    
    KFIFO_INIT(g_upgrade_manager.kf, kfifo_buff, IMAGE_UPGRADE_FIFO_SIZE);
}


void _get_image_upgrade_reply(int &event, int &data)
{
    if(g_upgrade_manager.end_reply > INVALID_REPLY)
    {
        event = UPGRADE_END_REPLY;
        data = g_upgrade_manager.end_reply;
        g_upgrade_manager.end_reply = INVALID_REPLY;
    }
    else 
    {
        event = UPGRADE_BUFF_SIZE;
        data = IMAGE_UPGRADE_FIFO_SIZE-KFIFO_SIZE(g_upgrade_manager.kf);
    }
}

int _begin_image_upgrade(unsigned int image_size)
{
    //init image flags data
    KFIFO_CLEAR(g_upgrade_manager.kf);
    
    g_upgrade_manager.state = IMAGE_UPGRADING;
    
    g_upgrade_manager.last_page_flag = 0;
    
    memset(&g_upgrade_manager.image_info, 0, sizeof(g_upgrade_manager.image_info));
    g_upgrade_manager.timeout_tick = 0;
    g_upgrade_manager.end_reply = INVALID_REPLY;
    g_upgrade_manager.writed_size = 0;
    
    g_upgrade_manager.image_max_size = image_size;
    DBG_PRINTF("IMAGE: begin_image_upgrade image_max_size:%d\n", g_upgrade_manager.image_max_size);
    return 0;
}

void _stop_image_upgrade(int error)
{
    DBG_PRINTF("IMAGE: stop_image_upgrade state:%d error:%d\n",g_upgrade_manager.state, error);
    if(error && g_upgrade_manager.state==IMAGE_UPGRADING) 
    {
        //delete error image data
        if(g_upgrade_manager.writed_size && g_upgrade_manager.image_info.startAddress!=0)
        {
            DBG_PRINTF("IMAGE: stop_image_upgrade fl_deleteImage\n");
            fl_deleteImage(g_upgrade_manager.image_info);
        }
        
        fl_endWriteImage();
    }

    g_upgrade_manager.state = IMAGE_UPGRADE_IDLE;
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

    return BYTE_ALIGNMENT_4096(image_info.startAddress);
}

static int begin_image_deal()
{
    fl_BootImageInfo factory_image;
    unsigned free_boot_size = 0;                //除去出厂固件的空间大小
    unsigned upgrade_image_offset = 0;          //从出厂固件后开始的偏移值
    unsigned free_boot_half_address = 0;        //第二升级固件的开始地址
    
    int image_num = 0;
    int wait_res = 0;
    int reply = INVALID_REPLY;
	memset(&g_delete_image, 0, sizeof(g_delete_image));
	
    do {
		/*
		 * 常规操作：出厂固件 + 第一升级固件 + 第二升级固件
		 * 条件：除去出厂固件大小的程序区大小 大于 500kb
		 *
		 * 补充操作：出厂固件 + 第一升级固件
		 * 条件：除去出厂固件大小的程序区大小 小于 500kb
		 */
		
        if(fl_getFactoryImage(factory_image) != 0)
        {
            DBG_PRINTF("IMAGE: fl_getFactoryImage error\n");
            reply = UPDATE_IAMGE_ERROR_NOT_FACTORY; 
            break;
        }
		
		// 获取空闲的程序空间
        free_boot_size = factory_image.size+factory_image.startAddress;
		free_boot_size = BYTE_ALIGNMENT_4096(free_boot_size);//4k对齐
		free_boot_size = g_boot_size - free_boot_size;
        
        image_num = 1;
        
        while(fl_getNextBootImage(g_upgrade_manager.image_info) == 0) image_num++;
		
		g_upgrade_manager.image_info.factory = 0;
		g_upgrade_manager.image_info.version = 1;   
        //使用fl_getNextBootImage获取的固件大小
		//g_upgrade_manager.image_info.size = 0;    
		
		if(free_boot_size>=(500*1024) && image_num >= 2)
		{
			g_delete_image = g_upgrade_manager.image_info;
			free_boot_half_address = BYTE_ALIGNMENT_4096(free_boot_size>>1)+BYTE_ALIGNMENT_4096(factory_image.size+factory_image.startAddress);
            
			if(g_upgrade_manager.image_info.startAddress >= free_boot_half_address)	
			{
				// 原升级固件在第二固件区, 目标升级固件在第一固件区
				upgrade_image_offset = 0;

                // 固件开始地址为紧跟出厂固件后的地址(4k对齐)
				g_upgrade_manager.image_info.startAddress = get_next_image_startaddress(factory_image);
			}
			else
			{
				// 原升级固件在第一固件区, 目标升级固件在第二固件区
				upgrade_image_offset = BYTE_ALIGNMENT_4096(free_boot_size>>1);//4k对齐;

                // 固件开始地址为第二升级固件开始地址
				g_upgrade_manager.image_info.startAddress = free_boot_half_address;
			}
		}
		else
		{
			// 在出厂固件后面添加升级固件
			upgrade_image_offset = 0;
			memset(&g_delete_image, 0, sizeof(g_delete_image));
            
            // 固件开始地址为紧跟出厂固件后的地址(4k对齐)
            g_upgrade_manager.image_info.startAddress = get_next_image_startaddress(factory_image);	
		}
		
		// image_max_size特别操作
        if(g_upgrade_manager.image_max_size<10*1024)
        {
           g_upgrade_manager.image_max_size = BYTE_ALIGNMENT_4096(factory_image.size+factory_image.startAddress)+8*1024;
           DBG_PRINTF("IMAGE: image_max_size %d\n", g_upgrade_manager.image_max_size);
        }
        
        DBG_PRINTF("IMAGE: free_boot_size:%d free_boot_half_address:%d upgrade_image_offset:%d\n", free_boot_size, free_boot_half_address, upgrade_image_offset);
        
        // 补充操作, 正常情况下是不会出现image_num等于3
        if(image_num == 3)
        {
            DBG_PRINTF("IMAGE: fl_deleteImage\n");
            fl_deleteImage(g_upgrade_manager.image_info);
        }
        
        do
        {

		    //wait_res = fl_startImageAddAt(upgrade_image_offset, g_upgrade_manager.image_max_size);
            //wait_res = fl_startImageAdd(factory_image, g_upgrade_manager.image_max_size, upgrade_image_offset);
            //fl_BootImageInfo可以是虚假的
            wait_res = fl_startImageReplace(g_upgrade_manager.image_info, g_upgrade_manager.image_max_size);    
            
            if(wait_res == 0)
                break;
            
            if(wait_res < 0)
            {
                reply = UPDATE_IAMGE_ERROR_START_IMAGE;
                break;
            }
        }while (wait_res);
        
        DBG_PRINTF("IMAGE: startAddress:%d size:%d version:%d factory:%d\n", g_upgrade_manager.image_info.startAddress, g_upgrade_manager.image_info.size, g_upgrade_manager.image_info.version, g_upgrade_manager.image_info.factory);
        
    }while(0);
    
    DBG_PRINTF("IMAGE: StartImageUpgrade reply:%d wait_res:%d\n", reply, wait_res);

    return reply;

}

static void upgrading_image_poll_deal(server image_upgrade_if i_image, int interval_ms)
{
    unsigned char page[FLASH_PAGE_SIZE];
    unsigned int got_len = 0;
    do {
        if((g_upgrade_manager.timeout_tick++)>6000/interval_ms)//6s
        {
            g_upgrade_manager.end_reply = UPDATE_IAMGE_ERROR_TIMEOUT;
            break;
        }
        
        if(g_upgrade_manager.last_page_flag==0 && KFIFO_SIZE(g_upgrade_manager.kf)<FLASH_PAGE_SIZE)
            break;
    
        g_upgrade_manager.timeout_tick = 0;
    
        //get data
        got_len = _kfifo_get(g_upgrade_manager.kf, page, FLASH_PAGE_SIZE);
        if(got_len!=0 && got_len<FLASH_PAGE_SIZE)
        {
            DBG_PRINTF("IMAGE: got_len(%u) < FLASH_PAGE_SIZE \n", got_len);
            memset(page+got_len, 0, FLASH_PAGE_SIZE-got_len);
        }
   
        //start image crc
        if(g_upgrade_manager.writed_size == 0)
        {

            if(start_image_crc(page) != 0)
            {
                g_upgrade_manager.end_reply = UPDATE_IAMGE_ERROR_START_CRC;
                break;
            }
            
            g_upgrade_manager.end_reply = begin_image_deal();
            
            g_upgrade_manager.image_info.size = nibblerev(page[0x10]|(page[0x11]<<8)|(page[0x12]<<16)|(page[0x13]<<24));
            DBG_PRINTF("IMAGE: image_info.size = %u\n", g_upgrade_manager.image_info.size);

            if(g_upgrade_manager.end_reply != INVALID_REPLY)
                break;
            
        }
        else if(g_upgrade_manager.image_info.size >= g_upgrade_manager.writed_size)
        {
            unsigned int wlen = got_len;
            if(g_upgrade_manager.image_info.size < (g_upgrade_manager.writed_size+got_len))
            {
                wlen = g_upgrade_manager.image_info.size - g_upgrade_manager.writed_size;
            }
            put_image_crc(page, wlen);
        }

        //write flash
        if(got_len != 0)
        {
            int wtite_cnt = 3;
            while(wtite_cnt)
            {
                if(fl_writeImagePage(page) != 0)
                {
                    DBG_PRINTF("IMAGE: fl_writeImagePage failed\n");
                    wtite_cnt--;
                }
                else
                {
                    break;
                }
            }
            if(wtite_cnt == 0)
            {
                g_upgrade_manager.end_reply = UPDATE_IAMGE_ERROR_WRITE_IMAGE;
                break;
            }
        }
    
        g_upgrade_manager.writed_size+=got_len;
    
    }while(KFIFO_SIZE(g_upgrade_manager.kf));


    if(g_upgrade_manager.end_reply!=SUCCEED_REPLY && 
        g_upgrade_manager.end_reply!=INVALID_REPLY && 
        g_upgrade_manager.end_reply>=3)
    {
        //delete error image data
        if(g_upgrade_manager.writed_size && g_upgrade_manager.image_info.startAddress!=0)
        {
            DBG_PRINTF("IMAGE: fl_deleteImage1\n");
            fl_deleteImage(g_upgrade_manager.image_info);
        }
        fl_endWriteImage();
    }
    else if(g_upgrade_manager.last_page_flag==1 && KFIFO_SIZE(g_upgrade_manager.kf)==0)
    {
        //end write image, check image crc
        if(stop_image_crc() != 0)
        {
            DBG_PRINTF("IMAGE: delete image: %d %d %d %d\n", g_upgrade_manager.image_info.startAddress, g_upgrade_manager.image_info.size, g_upgrade_manager.image_info.version, g_upgrade_manager.image_info.factory);
            if(g_upgrade_manager.image_info.startAddress!=0)
            {
                DBG_PRINTF("IMAGE: fl_deleteImage2\n");
                fl_deleteImage(g_upgrade_manager.image_info);
            } 
            fl_endWriteImage();
            g_upgrade_manager.end_reply = UPDATE_IAMGE_ERROR_STOP_CRC;
        }
        else if(fl_endWriteImage() != 0)
        {
            g_upgrade_manager.end_reply = UPDATE_IAMGE_ERROR_END_IMAGE;
        }
		
		// 删除替换的固件
		if(g_delete_image.startAddress!=0)
			fl_deleteImage(g_delete_image);
		
        if(g_upgrade_manager.end_reply == INVALID_REPLY)
        {
            g_upgrade_manager.end_reply = SUCCEED_REPLY;
        }
    }

    if(g_upgrade_manager.end_reply > INVALID_REPLY)
    {
        //通知上层应用 end_reply 变化
        DBG_PRINTF("IMAGE: end_reply result %d\n", g_upgrade_manager.end_reply);
        g_upgrade_manager.state = IMAGE_UPGRADE_IDLE;    
        i_image.image_upgrade_ready();
    }
    else if(got_len!=0 && KFIFO_SIZE(g_upgrade_manager.kf)==0)
    {
        //通知上层应用 buff_size 变化
        i_image.image_upgrade_ready();
    }
}

void _image_upgrade_poll(server image_upgrade_if i_image, int interval_ms)
{
    if(g_upgrade_manager.state == IMAGE_UPGRADING)
    {
        upgrading_image_poll_deal(i_image, interval_ms);
    }
}

