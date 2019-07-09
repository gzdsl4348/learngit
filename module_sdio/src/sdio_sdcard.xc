#include "diskio.h"
//#ifdef BUS_MODE_4BIT
#include <platform.h>
#include <xs1.h>
#include <xclib.h>
#include <stdio.h>
#include "xassert.h"
#include "debug_print.h"
#include "sdio_sdcard.h"
#include "trycatch.h"

typedef struct SDHostInterface
{
  out port p_clk;   // a 1 bit port
  port p_cmd;       // a 1 bit port. Need an external pull-up resistor if not an XS1_G core
  port p_dat;       // a 4 bit port. Beware: connect D0 to PortBit3, D1 to PortBit2, D2 to PortBit1, D3 to PortBit0
                    // D0 (PortBit3) need an external pull-up resistor if not an XS1_G core
/*
   D D C     C   D D
   a a m     l   a a
   t t d     k   t t
   1 0           3 2
   __________________
  /  | | | | | | | | |
  || D C - + C - D D |
  |D 3 M G 3 L G 0 1 |
  |2   D N . K N     |
  |      D 3   D     |
  |        V         |
*/
  /* fields returned after initialization */
  unsigned long rca;        // RCA returned by SD card during initialization. Relative card address.
  unsigned char ccs;        // CCS returned by SD card during initialization. Card capacity status: 0 = SDSC; 1 = SDHC/SDXC 大容量卡
  unsigned long block_nr;   // number of 512 bytes blocks. Returned by initialization.
} SDHostInterface;

#define SDIO_ON_TILE 0

static SDHostInterface sdio_ch[] = // LIST HERE THE PORTS USED FOR THE INTERFACES
//       CLK,         CMD,     DAT3..0,
{on tile[SDIO_ON_TILE]:{XS1_PORT_1E, XS1_PORT_1F, XS1_PORT_4D, 0, 1, 0}}; // ports used for interface #0
//{XS1_PORT_1O, XS1_PORT_1P, XS1_PORT_4F, 0, 0, 0}; // ports used for interface #1


//{XS1_PORT_1O, XS1_PORT_1P, XS1_PORT_4F, 0, 0, 0}; // ports used for interface #1
on tile[SDIO_ON_TILE]: clock SDIO_CLK = XS1_CLKBLK_5;

/***************************/

typedef enum {NO_RESP, R1, R1B, R2, R3, R6, R7} RESP_TYPE;

typedef enum
{
    SD_OK = 0,
    SD_CMD_RSP_TIMEOUT,
    SD_DATA_TIMEOUT,
    SD_R7_START_ERROR,
    SD_R7_CRC_ERROR,
    SD_R7_ENDBIT_ERROR,
    SD_R2_START_ERROR,
    SD_R2_ENDBIT_ERROR,
    SD_R3_START_ERROR,
    SD_R3_ENDBYT_ERROR,
} SD_DRESULT;

typedef unsigned char RESP[17]; // type for SD responses

#define RESP_IDLE                0
#define RESP_WAITING_START_BIT   1
#define RESP_RECEIVING_BITS      2

#define DAT_IDE                  0
#define DAT_WAITING_START_NIBBLE 1
#define DAT_RECEIVING_NIBBLE_H   2
#define DAT_RECEIVING_NIBBLE_L   3
#define DAT_RECEIVING_CRC        4

#define CRC7_POLY (0x91 >> 1) //x^7+X^3+x^0
#define CRC16_POLY (0x10811 >> 1) //x^16+X^12+x^5+x^0

// Temp - add no-ops
#define NOP_PAUSE  //asm("nop");

#define CMD_BIT(data) sdio_ch[ch_num].p_clk <: 0;sdio_ch[ch_num].p_cmd <: >> data;sdio_ch[ch_num].p_clk <: 1;


void sdio_init()
{
    set_clock_on(SDIO_CLK);
    
    configure_clock_ref(SDIO_CLK, 0);
    
    configure_out_port(sdio_ch[0].p_clk, SDIO_CLK, 1);

    configure_out_port(sdio_ch[0].p_cmd, SDIO_CLK, 1);

    configure_in_port(sdio_ch[0].p_dat, SDIO_CLK);
    
    start_clock(SDIO_CLK);
}

static int fl = 0;
exception_t e;

#pragma unsafe arrays
static SD_DRESULT send_cmd(BYTE ch_num, BYTE cmd, DWORD arg, RESP_TYPE resp_type, int data_blocks, BYTE buff[], RESP resp)
{ 
    //01CMD[6]ARG[32]CRC[7]1
    static unsigned int i, j, crc0, crc1, crc2, crc3;
    unsigned int D0, D1, D2, D3;
    unsigned int resp_stat, respbit_len, respbit_cnt, respbyte_cnt;
    unsigned int dat_stat, datbytes_len, datbyte_cnt, dat;
    unsigned char R;
    crc0 = 0;
//TRY{    
    fl = __LINE__;
    //ch_num = 0;
    //发送CMD指令,在每个CMD_BIT宏定义之间进行参数初始化
    set_port_drive(sdio_ch[0].p_cmd);
    fl = __LINE__;
    i = bitrev(cmd | 0b01000000) >> 24; // build first byte of command: start bit, host sending bit, cmd
    crc8shr(crc0, i, CRC7_POLY);
    fl = __LINE__;
    CMD_BIT(i); // send first byte of command
    arg = bitrev(arg);
    CMD_BIT(i);
    crc32(crc0, arg, CRC7_POLY);
    CMD_BIT(i);
    crc32(crc0, 0, CRC7_POLY); // flush crc engine
    CMD_BIT(i);
    crc0 |= 0x80; // build last byte of command: crc7 and stop bit
    CMD_BIT(i);
    resp_stat = ((NO_RESP == resp_type) ? RESP_IDLE : RESP_WAITING_START_BIT);
    CMD_BIT(i);
    respbit_len = (R2 == resp_type) ? 136 : 48;
    CMD_BIT(i);
    respbit_cnt = 0;
    CMD_BIT(i);fl = __LINE__;
    for(i = 32; i; i--) { CMD_BIT(arg); } // send arg
    fl = __LINE__;CMD_BIT(crc0); // send CRC7 and stop bit
    respbyte_cnt = 0;
    CMD_BIT(crc0);
    dat = 0xFFFFFFFF;
    CMD_BIT(crc0);
    R = 0xFF;
    CMD_BIT(crc0);
    dat_stat = (0 < data_blocks) ? DAT_WAITING_START_NIBBLE : DAT_IDE;
    CMD_BIT(crc0);
    datbytes_len = data_blocks * 512;
    CMD_BIT(crc0);
    datbyte_cnt = 0;
    CMD_BIT(crc0);
    i = 0;
    CMD_BIT(crc0);
    fl = __LINE__;
    //这个不应该放在CMD_BIT执行前吗？
    // if an XS1-G can enable internal pull-up otherwise need an external pull-up resistor for p_cmd pin
    // set_port_pull_up(sdio_ch[0].p_cmd); 

    sdio_ch[0].p_cmd :> void;
    fl = __LINE__;
    //response 操作 或 读操作(读第一块与response读取有重叠部分)
    while(resp_stat!=RESP_IDLE || dat_stat!=DAT_IDE)
    {fl = __LINE__;
        // Temp - add no-ops
        sdio_ch[0].p_clk <: 0; sdio_ch[0].p_clk <: 1; // 1 clock pulse
        i++;//这个代码可替代上一行的NOP_PAUSE代码,结果显示有一点点的优化作用
        fl = __LINE__;
        //接收response
        switch(resp_stat)
        {
        case RESP_IDLE:break;
        
        case RESP_WAITING_START_BIT:
            sdio_ch[0].p_cmd :> >> R;
            if(0xFF == R)
            {
                if(4000000 == i) return SD_CMD_RSP_TIMEOUT; // busy timeout
                break;
            }
            respbit_cnt = 1;
            resp_stat = RESP_RECEIVING_BITS; // next state
            break;
        case RESP_RECEIVING_BITS:
            sdio_ch[0].p_cmd :> >> R;
            if(++respbit_cnt % 8) break;
            
            if(respbit_cnt == respbit_len)
                resp_stat = RESP_IDLE;
            resp[respbyte_cnt++] = R;
            break;
        }
        fl = __LINE__;
        //接收dat引脚数据
        switch(dat_stat)
        {
        case DAT_WAITING_START_NIBBLE:fl = __LINE__;
            sdio_ch[0].p_dat :> >> dat;
            
            if(0x0FFFFFFF == dat) dat_stat = DAT_RECEIVING_NIBBLE_H; // if start nibble arrived -> next state
            else if(400000 == i) return SD_DATA_TIMEOUT; // busy timeout
            break;
            
        case DAT_RECEIVING_NIBBLE_H:fl = __LINE__;
            sdio_ch[0].p_dat :> >> dat;
            dat_stat = DAT_RECEIVING_NIBBLE_L; // next state
            break;
            
        case DAT_RECEIVING_NIBBLE_L:fl = __LINE__;
            sdio_ch[0].p_dat :> >> dat;
            buff[datbyte_cnt++] = bitrev(dat);
        
            if(resp_stat == RESP_IDLE) // if response received... (can continue just sampling dat lines)
            {
                while(datbyte_cnt % 512)
                { /* todo: doing this stuff with assembly would highly increase performance */// Temp - add no-ops
                    sdio_ch[0].p_clk <: 0; asm("nop"); asm("nop");sdio_ch[0].p_clk <: 1;  // 1 clock pulse
                    sdio_ch[0].p_dat :> >> dat;
                    sdio_ch[0].p_clk <: 0; asm("nop"); asm("nop");sdio_ch[0].p_clk <: 1; // 1 clock pulse
                    sdio_ch[0].p_dat :> >> dat;
                    buff[datbyte_cnt++] = bitrev(dat);
                }
                j = 17; dat_stat = DAT_RECEIVING_CRC; // next state
                break;
            }
            else
            {
                if(datbyte_cnt % 512) {
                    dat_stat = DAT_RECEIVING_NIBBLE_H;
                }
                else {
                    j = 17; 
                    dat_stat = DAT_RECEIVING_CRC; 
                }
            }
            
            break;
            
        case DAT_RECEIVING_CRC: fl = __LINE__;// ignoring crc. todo?
            //sdio_ch.p_dat :> dat;
            if(--j) break; // discard 17 nibbles ( 8 bytes CRC + 1 nibble end data )
            if(datbyte_cnt < datbytes_len)
            {
                dat = 0xFFFFFFFF; i = 0;
                dat_stat = DAT_WAITING_START_NIBBLE;
            }
            else
            {
                dat_stat = DAT_IDE;
            }
            break;
            
        }
    }
    fl = __LINE__;

    //处理response数据
    switch(resp_type) // response check
    {
    case NO_RESP: break;
    case R1:
    case R1B:
    case R6:
    case R7:
        crc0 = 0;
        crc8shr(crc0, resp[0], CRC7_POLY);
        i = bitrev(resp[0]) >> 24;
        if(i != cmd)
            return SD_R7_START_ERROR;
        arg = (resp[4] << 24) | (resp[3] << 16) | (resp[2] << 8) | resp[1];
        crc32(crc0, arg, CRC7_POLY);
        arg = bitrev(arg); // if R1: card status; if R6: RCA; if R7: voltage accepted, echo pattern
        crc32(crc0, 0, CRC7_POLY); // flush crc engine
        if(crc0 != (resp[5] & 0x7F))
            return SD_R7_CRC_ERROR; //crc error
        if((resp[5] & 0x80) == 0)
            return SD_R7_ENDBIT_ERROR; //end bit error
        break;
    case R2: // 136 bit response
        if(0xFC != resp[0])
            return SD_R2_START_ERROR; // R2 beginning error
        if(0x80 != (resp[16] & 0x80))
            return SD_R2_ENDBIT_ERROR; // R2 end bit error
        break;
    case R3:
        if(0xFC != resp[0])
            return SD_R3_START_ERROR; // R3 beginning error
        if(0xFF != resp[5])
            return SD_R3_ENDBYT_ERROR; // R3 end byte error
        break;
    }
    fl = __LINE__;    
    for(i = 8; i; i--) // send 8 clocks // Temp - add no-ops
    { sdio_ch[0].p_clk <: 0; NOP_PAUSE; sdio_ch[0].p_clk <: 1; NOP_PAUSE; }

    fl = __LINE__;

    //根据data_blocks判断进行写操作
    if(0 > data_blocks) // a write operation
    {
        xassert((((unsigned)&buff)%4)==0);//printf("sdio buff 4-byte alignment error\n");
        do {
            set_port_drive(sdio_ch[0].p_dat);

            crc0 = crc1 = crc2 = crc3 = 0;
            // Temp - add no-ops
            sdio_ch[0].p_clk <: 0; NOP_PAUSE; sdio_ch[0].p_dat <: 0; NOP_PAUSE; sdio_ch[0].p_clk <: 1;  // start data block

            for(j = 512/4; j; j--) // send bytes of data (512/4 int)
            {
                //约51 tick运算时间
                dat = byterev(bitrev((buff, int[])[datbyte_cnt++]));

                D3 = dat & 0x11111111; // compacting the 1st bit of the 8 nibbles...
                D3 |= D3 >> 3; D3 &= 0x03030303; D3 |= D3 >> 6; D3 |= D3 >> 12; //... in a single byte
                crc8shr(crc3, D3, CRC16_POLY);

                D2 = (dat >> 1) & 0x11111111; // compacting the 2nd bit of the 8 nibbles...
                D2 |= D2 >> 3; D2 &= 0x03030303; D2 |= D2 >> 6; D2 |= D2 >> 12; //... in a single byte
                crc8shr(crc2, D2, CRC16_POLY);

                D1 = (dat >> 2) & 0x11111111;
                D1 |= D1 >> 3; D1 &= 0x03030303; D1 |= D1 >> 6; D1 |= D1 >> 12;
                crc8shr(crc1, D1, CRC16_POLY);

                D0 = (dat >> 3) & 0x11111111;
                D0 |= D0 >> 3; D0 &= 0x03030303; D0 |= D0 >> 6; D0 |= D0 >> 12;
                crc8shr(crc0, D0, CRC16_POLY);                

                for(i = 8; i; i--) // send 8 nibbles // Temp - add no-ops // todo: do this in assembly
                { sdio_ch[0].p_clk <: 0; NOP_PAUSE; sdio_ch[0].p_dat <: >> dat; NOP_PAUSE; sdio_ch[0].p_clk <: 1; NOP_PAUSE; } 
            }

            // write CRCs, end nibble and wait busy， 约9 tick运算时间
            crc32(crc0, 0, CRC16_POLY); // flush crc engine
            crc32(crc1, 0, CRC16_POLY); // flush crc engine
            crc32(crc2, 0, CRC16_POLY); // flush crc engine
            crc32(crc3, 0, CRC16_POLY); // flush crc engine


            for(i = 16; i; i--)
            {
                dat = (crc3 & 1) | ((crc2 & 1) << 1) | ((crc1 & 1) << 2) | ((crc0 & 1) << 3);
                sdio_ch[0].p_clk <: 0; NOP_PAUSE; sdio_ch[0].p_dat <: dat; NOP_PAUSE; sdio_ch[0].p_clk <: 1;
                crc3 >>= 1; crc2 >>= 1; crc1 >>= 1; crc0 >>= 1;
            }
            sdio_ch[0].p_clk <: 0; NOP_PAUSE; sdio_ch[0].p_dat <: 0xF; NOP_PAUSE; sdio_ch[0].p_clk <: 1; //NOP_PAUSE; // end data block

            // if an XS1-G can enable internal pull-up otherwise need an external pull-up resistor D0 (Dat3) pin
            set_port_pull_up(sdio_ch[0].p_dat); 
            sdio_ch[0].p_dat :> void;
            
            for(i = 8; i; i--) // send 8 clocks
            { sdio_ch[0].p_clk <: 0; NOP_PAUSE; sdio_ch[0].p_clk <: 1;NOP_PAUSE;}

            //写数据等待繁忙，长等待60000tick，短等待83tick，16k写操作平均有1.5次长等待
            i = 4000000;
            do {// wait busy // Temp - add no-ops
                sdio_ch[0].p_clk <: 0; NOP_PAUSE; sdio_ch[0].p_clk <: 1; NOP_PAUSE; sdio_ch[0].p_dat :> dat; //NOP_PAUSE;
                if(!i--) return RES_ERROR; // busy timeout
            } while(!(dat & 0x8));
            
            
        } while(++data_blocks);
    }
    fl = __LINE__;

    //R1B response 的等待繁忙
    if(R1B == resp_type)
    {
        i = 4000000;
        do // wait busy // Temp - add no-ops
        {
            sdio_ch[0].p_clk <: 0; NOP_PAUSE; sdio_ch[0].p_clk <: 1; NOP_PAUSE; sdio_ch[0].p_dat :> dat; //NOP_PAUSE;
            if(!i--) return RES_ERROR; // busy timeout
        } while(!(dat & 0x8));
    }
    fl = __LINE__;
    return RES_OK;
//} 
/*CATCH(e) {
    debug_printf("SDIO exception: type=%d data=%d %d %d\n",e.type, e.data, fl, i);
    return 1;
}*/
    return 1;
}

/******* public functions ********/

DSTATUS disk_initialize(BYTE ch_num)
{
    timer tmr;
    unsigned int i, j, BlockLen;
    RESP resp;
    unsigned char dummy_data[10];

    sdio_init();
    
    //if(ch_num >= sizeof(sdio_ch)/sizeof(SDHostInterface)) return RES_PARERR;
    // configure ports and clock blocks
    sdio_ch[0].p_cmd <: 1;
    sdio_ch[0].p_dat <: 0xF;
    sdio_ch[0].p_clk <: 1 @ i;
    // send 74 clocks
    for(BlockLen = 74; BlockLen; BlockLen--)
    { 
        i += 125;
        sdio_ch[0].p_clk @ i <: 0;
        i += 125;
        sdio_ch[0].p_clk @ i <: 1;
    }

    // initialize card
    sdio_ch[0].rca = 0;    


    //CMD0命令复位SD卡
    if(send_cmd(ch_num, SD_CMD_GO_IDLE_STATE, 0, NO_RESP, 0, dummy_data, resp)) return RES_ERROR;

    BlockLen = send_cmd(ch_num, SD_CMD_HS_SEND_EXT_CSD, 0x1AA, R7, 0, dummy_data, resp) ? 0x00FF8000 : 0x50FF8000; // SDHC/XC or SDSC. 2.7V..3.6V

    tmr :> i;
    do
    {
        if(send_cmd(ch_num, SD_CMD_APP_CMD, 0, R1, 0, dummy_data, resp)) return RES_ERROR;

        if(send_cmd(ch_num, SD_CMD_SD_APP_OP_COND, BlockLen, R3, 0, dummy_data, resp)) return RES_ERROR;  // ACMD41

        tmr :> j;
        if((j-i) > 20000000) return RES_ERROR; // 200ms busy timeout
     } while((resp[1] & 1) == 0); // repeat while busy

    sdio_ch[ch_num].ccs = ((resp[1] & 2)) ? 1 : 0;

//    if(sdio_ch[ch_num].ccs == 0)//ccs 0 = SDSC; 1 = SDHC/SDXC
//        printf("SDSC\n");
//    else
//        printf("SDHC/SDXC %dus\n", (j-i)/100);

    if(send_cmd(ch_num, SD_CMD_ALL_SEND_CID, 0, R2, 0, dummy_data, resp)) return RES_ERROR; // get CID
    if(send_cmd(ch_num, SD_CMD_SET_REL_ADDR, 0, R6, 0, dummy_data, resp)) return RES_ERROR; // get RCA
    sdio_ch[ch_num].rca = 0xFFFF0000 & bitrev(resp[1] | (resp[2] << 8) | (resp[3] << 16) | (resp[4] << 24)); // rca to be used in addressed commands
    if(send_cmd(ch_num, SD_CMD_SEND_CSD, sdio_ch[ch_num].rca, R2, 0, dummy_data, resp)) return RES_ERROR; // get CSD
    
    // evaluate card size
    if(0 == (resp[1] & 0x3)) // CSD ver. 1.0
    { 
        BlockLen = bitrev(resp[6] << 24) & 0x0F; // READ_BL_LEN
        BlockLen = 1 << BlockLen;
        i = ((bitrev(resp[7]) >> 14) | (bitrev(resp[8]) >> 22) | (bitrev(resp[9]) >> 30)) & 0xFFF; // C_SIZE
        
        sdio_ch[ch_num].block_nr = ((bitrev(resp[10]) >> 23) | (bitrev(resp[11]) >> 31)) & 0x07; // C_SIZE_MULT
        sdio_ch[ch_num].block_nr = 4 << sdio_ch[ch_num].block_nr; // MULT
        sdio_ch[ch_num].block_nr = (i + 1) * sdio_ch[ch_num].block_nr;
        { sdio_ch[ch_num].block_nr, BlockLen } = lmul(sdio_ch[ch_num].block_nr, BlockLen, 0, 0); // evaluate card size bytes
        sdio_ch[ch_num].block_nr = (sdio_ch[ch_num].block_nr << 23) | (BlockLen >> 9); // n. of 512 bytes blocks
    }
    else // CSD ver. 2.0: // evaluate card size
    {
        sdio_ch[ch_num].block_nr = (bitrev(resp[10]) >> 24) | (bitrev(resp[9]) >> 16) | (bitrev(resp[8]) >> 8); // C_SIZE
        sdio_ch[ch_num].block_nr = (sdio_ch[ch_num].block_nr + 1)*1024;  // n. of 512 bytes blocks
    }

    if(send_cmd(ch_num, SD_CMD_SEL_DESEL_CARD, sdio_ch[ch_num].rca, R1B, 0, dummy_data, resp)) return RES_ERROR; // select card
    if(send_cmd(ch_num, SD_CMD_APP_CMD, sdio_ch[ch_num].rca, R1, 0, dummy_data, resp)) return RES_ERROR; // ACMD6
    if(send_cmd(ch_num, SD_CMD_HS_SWITCH, 0b10, R1, 0, dummy_data, resp)) return RES_ERROR; // set bus 4 bit

    // leaving card in transfer state
    return RES_OK;
}


#pragma unsafe arrays
DRESULT disk_read(BYTE ch_num, BYTE buff[], DWORD sector, BYTE count)
{
    RESP resp;
    unsigned char dummy_data[10];

    //if(ch_num >= sizeof(sdio_ch)/sizeof(SDHostInterface)) return RES_PARERR;
    
    if(count > 1)
    {   // multiblock read
        //if(send_cmd(ch_num, SD_CMD_SET_BLOCK_COUNT, count, R1, 0, dummy_data, resp)) return RES_ERROR; // set foreseen multiple block read. Remarked because only optionally supported by cards
        if(send_cmd(ch_num, SD_CMD_READ_MULT_BLOCK, sdio_ch[ch_num].ccs ? sector : 512 * sector, R1, count, buff, resp)) return RES_ERROR; // multiblock read
        if(send_cmd(ch_num, SD_CMD_STOP_TRANSMISSION, 0, R1, 0, dummy_data, resp)) return RES_ERROR; // stop multi-block read. (using stop command instead of cmd23)
    }
    else
    {
        if(send_cmd(ch_num, SD_CMD_READ_SINGLE_BLOCK, sdio_ch[ch_num].ccs ? sector : 512 * sector, R1, 1, buff, resp)) return RES_ERROR; // single block read
    }
    return RES_OK;
}

#pragma unsafe arrays
DRESULT disk_write(BYTE ch_num, const BYTE buff[],DWORD sector, BYTE count)
{
    RESP resp;
    unsigned char dummy_data[10];

    //if(ch_num >= sizeof(sdio_ch)/sizeof(SDHostInterface)) return RES_PARERR;

    if(count > 1)
    {   // multiblock write
//        if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
//        {
//            //发送ACMD55,短响应,等待R1响应 
//            if(send_cmd(ch_num, SD_CMD_APP_CMD, sdio_ch[ch_num].rca, R1, 0, dummy_data, resp)) return RES_ERROR;
//            //发送CMD23,设置块数量,短响应,等待R1响应 
//            if(send_cmd(ch_num, SD_CMD_SET_BLOCK_COUNT, count, R1, 0, dummy_data, resp)) return RES_ERROR; // set foreseen multiple block read. Remarked because only optionally supported by cards
//        }
        
        if(send_cmd(ch_num, SD_CMD_WRITE_MULT_BLOCK, sdio_ch[ch_num].ccs ? sector : 512 * sector, R1, -count, (buff, BYTE[]), resp)) return RES_ERROR; // multiblock write
        if(send_cmd(ch_num, SD_CMD_STOP_TRANSMISSION, 0, R1B, 0, dummy_data, resp)) return RES_ERROR; // stop multi-block write. (using stop command instead of cmd23)
    }
    else
    {
        if(send_cmd(ch_num, SD_CMD_WRITE_SINGLE_BLOCK, sdio_ch[ch_num].ccs ? sector : 512 * sector, R1, -1, (buff, BYTE[]), resp)) return RES_ERROR; // single block write
    }
    return RES_OK;
}

DSTATUS disk_status(BYTE ch_num)
{
    unsigned char dummy_data[10];
    RESP resp;

    //if(ch_num >= sizeof(sdio_ch)/sizeof(SDHostInterface)) return STA_NOINIT;

    if(!sdio_ch[ch_num].rca) return STA_NOINIT;

    if(send_cmd(ch_num, SD_CMD_SEND_STATUS, sdio_ch[ch_num].rca, R1, 0, dummy_data, resp)) return STA_NOINIT; /* Read card status */

    return 0;
}

#pragma unsafe arrays
DRESULT disk_ioctl (BYTE ch_num, BYTE ctrl, BYTE RetVal[])
{
    unsigned long i;

    //if(ch_num >= sizeof(sdio_ch)/sizeof(SDHostInterface)) return RES_PARERR;
    
    if (disk_status(ch_num) & STA_NOINIT) return RES_NOTRDY;   /* Check if card is in the socket */
    
    switch (ctrl)
    {
        case CTRL_SYNC:                /* Make sure that no pending write process */
            return RES_OK;
            
        case GET_SECTOR_COUNT: /* Get number of sectors on the disk (DWORD) */
            for(i = 0; i < sizeof(DWORD); i++)
                RetVal[i] = (sdio_ch[ch_num].block_nr, BYTE[])[i];
            
            return RES_OK;
            
        case GET_BLOCK_SIZE:   /* Get erase block size in unit of sector (DWORD) */
            for(DWORD Val = 128, i = 0; i < sizeof(DWORD); i++)
                RetVal[i] = (Val, BYTE[])[i];
            
            return RES_OK;
            
    }
    return RES_PARERR;
}

//#endif //BUS_MODE_4BIT

// User Provided Timer Function for FatFs module
DWORD get_fattime(void)
{
    return ((DWORD)(2018 - 1980) << 25)  /* Fixed to Jan. 1, 2010 */
           | ((DWORD)1 << 21)
           | ((DWORD)1 << 16)
           | ((DWORD)0 << 11)
           | ((DWORD)0 << 5)
           | ((DWORD)0 >> 1);
}

