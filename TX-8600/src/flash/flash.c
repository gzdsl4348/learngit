#include <quadflashlib.h>
#include <debug_print.h>
#include <stdint.h>

void c_read_flash_bytes(uint32_t flash_addr, uint32_t buff_addr, int br)
{
    uint8_t * p = (uint8_t *)buff_addr;
    fl_readData(flash_addr, br, p);
}

void c_read_flash_secoter(uint32_t secoter, uint32_t buff_addr, int br)
{
    uint8_t * p = (uint8_t *)buff_addr;
    fl_readData(secoter*4*1024, br, p);
}

void c_write_flash_secoter(uint32_t secoter, uint32_t buff_addr, int bw)
{
    uint8_t * p = (uint8_t *)buff_addr;
    int btw = 0;
    fl_eraseDataSector(secoter);
    for(int i=0; i<16 && btw<bw; i++)
    {
        
        fl_writeDataPage((secoter*16)+i, p+i*256);
        btw += 256;
    }
}

