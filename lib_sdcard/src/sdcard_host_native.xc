// Copyright (c) 2015, XMOS Ltd, All rights reserved

#include <xs1.h>
#include <xclib.h>
#include <string.h>
#include "sdcard_host.h"

#include <platform.h>
#include <print.h>
#include <debug_print.h>
/** Status of SD Interface Functions **/

#define ST_INIT                0x00
#define ST_NOINIT              0x01    /* Drive not initialized */
#define ST_NODISK              0x02    /* No medium in the drive */
#define ST_PROTECT             0x04    /* Write protected */

/** Miscellaneous Functions - Control Code  **/
#define CL_SYNC             0       /* Flush disk cache (for write functions) */
#define SECTOR_COUNT        1       /* Get media size (for only f_mkfs()) */
#define SECTOR_SIZE         2       /* Get sector size (for multiple sector size (_MAX_SS >= 1024)) */
#define BLOCK_SIZE          3       /* Get erase block size (for only f_mkfs()) */
#define ERASE_SECTOR        4       /* Force erased a block of sectors (for only _USE_ERASE) */


/* MMC/SDC specific ioctl command */
#define GET_TYPE        10  /* Get card type */
#define GET_CSD         11  /* Get CSD */
#define GET_CID         12  /* Get CID */
#define GET_OCR         13  /* Get OCR */
#define GET_CARDSTAT    14  /* Get CARD status */

/* Card type flags (MMC_GET_TYPE) */
#define CT_MMC      0x01        /* MMC ver 3 */
#define CT_SD1      0x02        /* SD ver 1 */
#define CT_SD2      0x04        /* SD ver 2 */
#define CT_SDC      (CT_SD1|CT_SD2) /* SD */
#define CT_BLOCK    0x08        /* Block addressing */


#define INIT_CLOCK  250
#define TF_CLOCK    1

#define SDCLK_16CLOCKS    0xAAAAAAAA
#define SDCLK_8CLOCKS_RV  0xAAAABFFF
#define SDCLK_8CLOCKS     0xAAAAFFFF
#define SDCLK_4CLOCKS     0xAAFFFFFF
#define SDCLK_2CLOCKS     0xAFFFFFFF
#define SDCLK_1CLOCKS     0xBFFFFFFF

#define CMD_WIDTH 8
#define CLK_WIDTH 32
#define DAT_WIDTH 32


#define CRC_TOKEN_OK  0xF7F7    //0xFEFE
#define CRC_TOKEN_CRC 0xFFEF
#define CRC_TOKEN_WE  0xFEFF
#define CRC_TOKEN_DC  0xFFFF


#define setc(a,b) {__asm__  __volatile__("setc res[%0], %1": : "r" (a) , "r" (b));}
#define settw(a,b) {__asm__  __volatile__("settw res[%0], %1": : "r" (a), "r" (b));}


///** Status of SD Interface Functions **/
typedef unsigned char    MDSTATUS;
MDSTATUS Stat = ST_NOINIT;  /* Disk status */

/** Return values from SD Transcations **/
typedef enum {
        MD_OK = 0,             /* 0: Successful */
        MD_ERROR,              /* 1: R/W Error */
        MD_WRPRT,              /* 2: Write Protected */
        MD_NOTRDY,             /* 3: Not Ready */
        MD_PARERR              /* 4: Invalid Parameter */
} MDRESULT;

static unsigned char CardType,          /* Card type flag */
                     CardInfo[16+16+4]; /* CSD(16), CID(16), OCR(4) */


/*********** SD Host-Card Registers Structure************/
typedef struct sd_host_reg_t
{
 /* fields returned after initialization */
  unsigned long CardRCA; // Relative Card Address
  unsigned char CCS;     // Card Capacity Status : 0 = SDSC; 1 = SDHC/SDXC
  unsigned long BlockNr; // Number of 512 bytes blocks
} sd_host_reg_t;


/***********Macro Definitions & Enumerations*************/

typedef enum RespType
{R0, R1, R1B, R6, R7, R2, R3} RESP_TYPE;
typedef unsigned char RESP[17]; // type for SD responses


#define CRC7_POLY (0x91 >> 1) //x^7+X^3+x^0
#define CRC16_POLY (0x10811 >> 1) //x^16+X^12+x^5+x^0


/* MMC/SD command (native mode) */
#define CMD0   (0)     /* GO_IDLE_STATE */
#define CMD1   (1)     /* SEND_OP_COND */
#define CMD2   (2)     /* SEND CID*/
#define CMD3   (3)     /* SEND RCA */
#define CMD6   (6)     /* CARD SELECT */
#define CMD7   (7)     /* WIDE BUS MODE SELECT*/
#define CMD8   (8)     /* SEND_IF_COND */
#define CMD9   (9)     /* SEND_CSD */
#define CMD10  (10)    /* SEND_CID */
#define CMD12  (12)    /* STOP_TRANSMISSION */
#define CMD13  (13)    /* SEND_STATUS */
#define CMD16  (16)    /* SET_BLOCKLEN */
#define CMD17  (17)    /* READ_SINGLE_BLOCK */
#define CMD18  (18)    /* READ_MULTIPLE_BLOCK */
#define CMD23  (23)    /* SET_BLOCK_COUNT */
#define CMD24  (24)    /* WRITE_BLOCK */
#define CMD25  (25)    /* WRITE_MULTIPLE_BLOCK */
#define CMD41  (41)    /* SEND_OP_COND (ACMD) */
#define CMD55  (55)    /* APP_CMD */
#define CMD58  (58)    /* READ_OCR */

static int gs_timeout_tick = 4000000;

static void sync_sdclk(buffered out port:32 p_sdclk,
           [[bidirectional]] buffered port:8 p_sdcmd,
           [[bidirectional]] buffered port:32 p_sddata,
                             clock cb)
{
    stop_clock(cb);
    
    setc(p_sddata,0x0);
    setc(p_sddata,0x8);
    setc(p_sddata,0x200f);
    settw(p_sddata,DAT_WIDTH);

    setc(p_sdcmd,0x0);
    setc(p_sdcmd,0x8);
    setc(p_sdcmd,0x200f);
    settw(p_sdcmd,CMD_WIDTH);

    setc(p_sdclk,0x0);
    setc(p_sdclk,0x8);
    setc(p_sdclk,0x200f);
    settw(p_sdclk,CLK_WIDTH);
    
    start_clock(cb);    
    
    p_sdclk <:  SDCLK_8CLOCKS;
    sync(p_sdclk);       

}

/*-----------------------------------------------------------------------
* Send a command token to the card and receive a response
-----------------------------------------------------------------------*/
#pragma unsafe arrays
//01CMD[6]ARG[32]CRC[7]1
static unsigned char send_cmd(unsigned char Cmd,
                             unsigned long Arg,
                             RESP_TYPE RespType,
                             RESP Resp,
                             buffered out port:32 p_sdclk,
           [[bidirectional]] buffered port:8 p_sdcmd,
           [[bidirectional]] buffered port:32 p_sddata,
                             clock cb){

    unsigned int i=0, Crc0 = 0;
    unsigned int RespStat, RespLen;
    unsigned char R = 0xFF;
    unsigned int arg = Arg;
    unsigned int Dat;


    RespStat = (R0 == RespType) ?  0 : 1;
    RespLen  = (R2 == RespType) ? 17 : 6;

    /*Start Bit|Transmit Bit|Cmd - 8 bit*/
    i = bitrev(Cmd | 0b01000000) >> 24; // build first byte of command: start bit, host sending bit, Cmd
    crc8shr(Crc0, i, CRC7_POLY);
    /*Content - 32 Bit*/
    arg = bitrev(arg);
    crc32(Crc0, arg, CRC7_POLY);

    /* CRC - 8bit*/
    crc32(Crc0, 0, CRC7_POLY); // flush crc engine
    Crc0 |= 0x80; // build last byte of command: crc7 and stop bit

    sync(p_sdclk);

    /*Trasmit Command*/
    p_sdcmd <: i;
    p_sdclk <: SDCLK_8CLOCKS;

    /*Trasmit Argument*/
    p_sdcmd <: >>arg;

    p_sdclk <: SDCLK_8CLOCKS;

    p_sdcmd <: >>arg;
    p_sdclk <: SDCLK_8CLOCKS;

    p_sdcmd <: >>arg;
    p_sdclk <: SDCLK_8CLOCKS;

    p_sdcmd <: >>arg;
    p_sdclk <: SDCLK_8CLOCKS;
    
    /*Trasmit CRC*/
    p_sdcmd <: Crc0;
    p_sdclk <: SDCLK_8CLOCKS;
    
    sync(p_sdclk);

    i = 0;
    /* Response Read Start*/
    if(RespStat)
    {
        stop_clock(cb);
        setc(p_sdcmd,0x0);
        setc(p_sdcmd,0x8);
        setc(p_sdclk,0x0);
        setc(p_sdclk,0x8);
        start_clock(cb);

        /* Check for Start Bit of Response*/
        i = gs_timeout_tick;
        do{
            p_sdclk <: 0; p_sdclk <: 1; p_sdcmd :> >>R;
            if(!i--) // timeout
            {
                sync_sdclk(p_sdclk, p_sdcmd, p_sddata, cb);    
                return MD_ERROR;
            }
        }while(R == 0xFF);

        /* Collect the Response Packets*/
        for (i=0;i<7;i++)
        {
            p_sdclk <: 0; p_sdclk <: 1;p_sdcmd :> >> R;
        }
        Resp[0] = R;
        i=1;
        stop_clock(cb);
        setc(p_sdcmd,0x0);
        setc(p_sdcmd,0x8);
        setc(p_sdcmd,0x200f);
        settw(p_sdcmd,CMD_WIDTH);

        setc(p_sdclk,0x0);
        setc(p_sdclk,0x8);
        setc(p_sdclk,0x200f);
        settw(p_sdclk,CLK_WIDTH);

        start_clock(cb);

        do{
            p_sdclk <: SDCLK_8CLOCKS_RV;
            p_sdcmd :> Resp[i];
            i++;
        }while (i<RespLen);

    }// Response Read End

    /* Command Response Check*/
    switch(RespType)
    {
        case R0: break;
        case R1:
        case R1B:
        case R6:
        case R7:
            Crc0 = 0;
            crc8shr(Crc0, Resp[0], CRC7_POLY);
            i = bitrev(Resp[0]) >> 24;
            if(i != Cmd)
            {
                sync_sdclk(p_sdclk, p_sdcmd, p_sddata, cb);  
                return MD_ERROR;
            }
            Arg = (Resp[4] << 24) | (Resp[3] << 16) | (Resp[2] << 8) | Resp[1];
            crc32(Crc0, Arg, CRC7_POLY);
            Arg = bitrev(Arg); // if R1: card status; if R6: RCA; if R7: voltage accepted, echo pattern
            crc32(Crc0, 0, CRC7_POLY); // flush crc engine
            if(Crc0 != (Resp[5] & 0x7F)) //crc error
            {
                sync_sdclk(p_sdclk, p_sdcmd, p_sddata, cb);     
                return MD_ERROR;
            }
            if((Resp[5] & 0x80) == 0)//end bit error
            {
                sync_sdclk(p_sdclk, p_sdcmd, p_sddata, cb);
                return MD_ERROR;
            }            
            break;
        case R2: // 136 bit response
            if(0xFC != Resp[0])// R2 beginning error
            {
                sync_sdclk(p_sdclk, p_sdcmd, p_sddata, cb);
                return MD_ERROR;
            }            
            if(0x80 != (Resp[16] & 0x80))// R2 end bit error
            {
                sync_sdclk(p_sdclk, p_sdcmd, p_sddata, cb);    
                return MD_ERROR;
            }            
            break;
        case R3:
            if(0xFC != Resp[0])// R3 beginning error
            {
                sync_sdclk(p_sdclk, p_sdcmd, p_sddata, cb);
                return MD_ERROR;
            }            
            if(0xFF != Resp[5])// R3 end byte error
            {
                sync_sdclk(p_sdclk, p_sdcmd, p_sddata, cb); 
                return MD_ERROR;
            }            
            break;
    }


    stop_clock(cb);
    setc(p_sddata,0x0);
    setc(p_sddata,0x8);
    setc(p_sdclk,0x0);
    setc(p_sdclk,0x8);
    start_clock(cb);

    // Wait for busy state clearing.
    if(R1B == RespType)
    {
        i = gs_timeout_tick;
        do {
            p_sdclk <: 0; p_sdclk <: 1;p_sddata :> Dat;
            if(!i--)// busy timeout
            {
                sync_sdclk(p_sdclk, p_sdcmd, p_sddata, cb);
                return MD_ERROR;
            }             
        } while(!(Dat & 0x8));//0x1
    }

    stop_clock(cb);
    setc(p_sddata,0x0);
    setc(p_sddata,0x8);
    setc(p_sddata,0x200f);
    settw(p_sddata,DAT_WIDTH);

    setc(p_sdclk,0x0);
    setc(p_sdclk,0x8);
    setc(p_sdclk,0x200f);
    settw(p_sdclk,CLK_WIDTH);
    start_clock(cb);

    // Clocks for Card Processing
    p_sdclk <:  SDCLK_8CLOCKS;
    sync(p_sdclk);

    return MD_OK;
}

/*-----------------------------------------------------------------------
* Data Transfer over DAT lines are managed in this routine.
* This routine does the read of single or multiple block of data
* from SD Card.
-----------------------------------------------------------------------*/
#pragma unsafe arrays
static unsigned char read_datablock(int DataBlocks,
                                    unsigned char buff[],
                                    buffered out port:32 p_sdclk,
                  [[bidirectional]] buffered port:8 p_sdcmd,
                  [[bidirectional]] buffered port:32 p_sddata,
                                    clock cb){

    unsigned int  DatWordCount,DatByteCount=0;
    unsigned char R = 0xFF;
    unsigned int crc_packet,curr_read_data,prev_read_data;


    while(DataBlocks > 0){

        DatWordCount = 0;
        R = 0xFF;

        stop_clock(cb);
        setc(p_sddata,0x0);
        setc(p_sddata,0x8);
        setc(p_sdclk,0x0);
        setc(p_sdclk,0x8);
        start_clock(cb);

        /* Check for Start Bit of Data Packet*/
       int i = gs_timeout_tick;
        do
        {
           p_sdclk <: 0;p_sdclk <: 1; p_sddata :>  >> R;
           if(!i--) return MD_ERROR; // timeout
        }while(R == 0xFF);

        stop_clock(cb);
        setc(p_sddata,0x0);
        setc(p_sddata,0x8);
        setc(p_sddata,0x200f);
        settw(p_sddata,DAT_WIDTH);

        setc(p_sdclk,0x0);
        setc(p_sdclk,0x8);
        setc(p_sdclk,0x200f);
        settw(p_sdclk,CLK_WIDTH);
        start_clock(cb);

        p_sdclk <: SDCLK_8CLOCKS;
        p_sddata :>  curr_read_data;
        DatWordCount++;


        while(DatWordCount < 128)
        {
            prev_read_data = byterev(bitrev(curr_read_data));
            p_sdclk <: SDCLK_8CLOCKS;
            
            memcpy(&buff[DatByteCount], &prev_read_data, 4);
            DatByteCount+=4;
            
            DatWordCount++;
            p_sddata :> curr_read_data;
        }
        prev_read_data = byterev(bitrev(curr_read_data));
        
        memcpy(&buff[DatByteCount], &prev_read_data, 4);
        DatByteCount+=4;
        
        p_sdclk <: SDCLK_8CLOCKS;
        p_sddata :>  crc_packet;//no data

        p_sdclk <: SDCLK_8CLOCKS;
        p_sddata :> crc_packet;//no data

        DataBlocks--;
    }
    // Clocks for Card Processing
    p_sdclk <: SDCLK_8CLOCKS;
    sync(p_sdclk);

    return MD_OK;

}

/*-----------------------------------------------------------------------
* Data Transfer over DAT lines are managed in this routine.
* This routine does the write of single or multiple block of data
* to SD Card
-----------------------------------------------------------------------*/
#pragma unsafe arrays
static unsigned char write_datablock(int DataBlocks,
                                    const unsigned char buff[],
                                    buffered out port:32 p_sdclk,
                 [[bidirectional]]  buffered port:8 p_sdcmd,
                 [[bidirectional]]  buffered port:32 p_sddata,
                                    clock cb){

    unsigned int i, Crc0, Crc1, Crc2, Crc3;
    unsigned int  DatWordCount;

    unsigned short crc_token;
    unsigned char R = 0xF;
    unsigned int crc_packets[2]={0,0},packet_cnt;
    unsigned int Dat;
    unsigned int DatByteCount=0;

    unsigned int D0, D1, D2, D3;
    unsigned int tDat;
    unsigned int curr_data,prev_data;

    timer tim;
    unsigned t1,t2,t3,t4;

    while(DataBlocks > 0) {

        Crc0 = Crc1 = Crc2 = Crc3 = 0;
        crc_packets[0]= 0;
        crc_packets[1]= 0;
        DatWordCount =0;
        R = 0XF;
        
        //crc_token = 0xFFFF;
        // Start data block
        p_sddata  <: 0x0FFFFFFF;
        p_sdclk   <: SDCLK_8CLOCKS;

        tDat = ((buff, int[])[DatByteCount++]);
        prev_data =  byterev(bitrev(tDat));
        DatWordCount++ ;
        
        p_sddata  <: prev_data;
        p_sdclk   <: SDCLK_8CLOCKS;

        // Send bytes of data (512/4 int)
        while(DatWordCount < 128)
        {
            // Compacting the 1st bit of the 8 nibbles in a single byte
            D3 = prev_data & 0x11111111;
            D3 |= D3 >> 3; D3 &= 0x03030303; D3 |= D3 >> 6; D3 |= D3 >> 12;
            crc8shr(Crc3, D3, CRC16_POLY);

            tDat = ((buff, int[])[DatByteCount++]);

           // Compacting the 2nd bit of the 8 nibbles in a single byte
            D2 = (prev_data >> 1) & 0x11111111;
            D2 |= D2 >> 3; D2 &= 0x03030303; D2 |= D2 >> 6; D2 |= D2 >> 12;
            crc8shr(Crc2, D2, CRC16_POLY);

            curr_data =  byterev(bitrev(tDat));

            // Compacting the 3rd bit of the 8 nibbles in a single byte
            D1 = (prev_data >> 2) & 0x11111111;
            D1 |= D1 >> 3; D1 &= 0x03030303; D1 |= D1 >> 6; D1 |= D1 >> 12;
            crc8shr(Crc1, D1, CRC16_POLY);

            p_sddata  <: curr_data;
            p_sdclk   <: SDCLK_8CLOCKS;

            // Compacting the 4th bit of the 8 nibbles in a single byte
            D0 = (prev_data >> 3) & 0x11111111;
            D0 |= D0 >> 3; D0 &= 0x03030303; D0 |= D0 >> 6; D0 |= D0 >> 12;
            crc8shr(Crc0, D0, CRC16_POLY);

            prev_data = curr_data;

            DatWordCount ++;
        }

        D3 = curr_data & 0x11111111;
        D3 |= D3 >> 3; D3 &= 0x03030303; D3 |= D3 >> 6; D3 |= D3 >> 12;
        crc8shr(Crc3, D3, CRC16_POLY);

        D2 = (curr_data >> 1) & 0x11111111;
        D2 |= D2 >> 3; D2 &= 0x03030303; D2 |= D2 >> 6; D2 |= D2 >> 12;
        crc8shr(Crc2, D2, CRC16_POLY);

        D1 = (curr_data >> 2) & 0x11111111;
        D1 |= D1 >> 3; D1 &= 0x03030303; D1 |= D1 >> 6; D1 |= D1 >> 12;
        crc8shr(Crc1, D1, CRC16_POLY);

        D0 = (curr_data >> 3) & 0x11111111;
        D0 |= D0 >> 3; D0 &= 0x03030303; D0 |= D0 >> 6; D0 |= D0 >> 12;
        crc8shr(Crc0, D0, CRC16_POLY);
        
        // write CRCs, end nibble and wait busy
        crc32(Crc0, 0, CRC16_POLY); // flush crc engine
        crc32(Crc1, 0, CRC16_POLY); // flush crc engine
        crc32(Crc2, 0, CRC16_POLY); // flush crc engine
        crc32(Crc3, 0, CRC16_POLY); // flush crc engine
        
        for (packet_cnt = 0;packet_cnt<2;packet_cnt++)
        {
            for(i =0; i<8; i++)
            {
                Dat = ((Crc3 & 1) | ((Crc2 & 1) << 1 ) | ((Crc1 & 1) << 2) | ((Crc0 & 1) << 3));
                crc_packets[packet_cnt] = crc_packets[packet_cnt] |(Dat <<((i)*4));
                Crc3 >>= 1; Crc2 >>= 1; Crc1 >>= 1; Crc0 >>= 1;
            }
            p_sddata <: crc_packets[packet_cnt];
            p_sdclk <: SDCLK_8CLOCKS;
        }

        // end data block
        p_sddata  <: 0xF;
        p_sdclk   <: SDCLK_1CLOCKS;
        sync(p_sdclk);

        stop_clock(cb);
        setc(p_sddata,0x0);
        setc(p_sddata,0x8);
        setc(p_sdclk,0x0);
        setc(p_sdclk,0x8);
        start_clock(cb);
        
        //CRC token start detection
        i = gs_timeout_tick;
                
        do{
            p_sdclk <: 0; p_sdclk <: 1;p_sddata :> R;
            //p_sdclk <: SDCLK_1CLOCKS; p_sddata :> R;

            if(!i--) 
            {
                text_debug("busy timeout 1\n");
                return MD_ERROR; // busy timeout
            }            
        }while(R == 0xF);
            

        //CRC token
        for (i=0;i<4;i++){
            p_sdclk <: 0; p_sdclk <: 1;  p_sddata :> >>crc_token;
        }

        if (CRC_TOKEN_OK != crc_token)
        {
            stop_clock(cb);
            setc(p_sddata,0x0);
            setc(p_sddata,0x8);
            setc(p_sddata,0x200f);
            settw(p_sddata,DAT_WIDTH);

            setc(p_sdclk,0x0);
            setc(p_sdclk,0x8);
            setc(p_sdclk,0x200f);
            settw(p_sdclk,CLK_WIDTH);
            start_clock(cb);
            
            text_debug("write_datablock error crc_token 0x%x\n", crc_token);
            
            return MD_ERROR;
        }

        // Clocks for Card Processing
        for (i=0;i<8;i++)
        {
           p_sdclk <: 0; p_sdclk <: 1;
        }
        
        // Wait for busy state clearing.
        i = gs_timeout_tick;

        
        tim :> t3;
        do {
            p_sdclk <: 0; p_sdclk <: 1; p_sddata :> Dat;
            if(!i--)
            {
                text_debug("busy timeout 2\n");
                return MD_ERROR; // busy timeout
            }
        }while(!(Dat & 0x8));//0x1
        
            tim :> t4;
            if(t4-t3 >50*100000)
                text_debug("w4 %dms\n",(t4-t3)/100000);

        stop_clock(cb);
        setc(p_sddata,0x0);
        setc(p_sddata,0x8);
        setc(p_sddata,0x200f);
        settw(p_sddata,DAT_WIDTH);

        setc(p_sdclk,0x0);
        setc(p_sdclk,0x8);
        setc(p_sdclk,0x200f);
        settw(p_sdclk,CLK_WIDTH);
        start_clock(cb);

        DataBlocks--;
   }
    // Clocks for Card Processing
    p_sdclk <: SDCLK_8CLOCKS;
    sync(p_sdclk);

   return MD_OK;
}
/*-----------------------------------------------------------------------*/
/* Wait for card to be in ready state                                    */
/*-----------------------------------------------------------------------*/
static int wait_ready(unsigned char timeout,
                                sd_host_reg_t &sd_reg,
                                buffered out port:32 p_sdclk,
            [[bidirectional]]   buffered port:8 p_sdcmd,
            [[bidirectional]]   buffered port:32 p_sddata,
                                clock cb){
    RESP Resp;
    unsigned int tmr;

    if(!(sd_reg.CardRCA)) return 0;
    tmr = (timeout * gs_timeout_tick);
    do
    {
        if(MD_OK == send_cmd(CMD13, sd_reg.CardRCA, R1, Resp,p_sdclk,p_sdcmd,p_sddata,cb))
            if (!(tmr--)) break;
    }while((Resp[3] & 0x70) != 0x10);

    return tmr? 1: 0;
}

/******* private functions ********/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Driver (SDCARD)                                       */
/*-----------------------------------------------------------------------*/

MDSTATUS initialize(sd_host_reg_t &sd_reg,
                    buffered out port:32 p_sdclk,
  [[bidirectional]] buffered port:8 p_sdcmd,
  [[bidirectional]] buffered port:32 p_sddata,
                    clock cb){

    unsigned int i=0, BlockLen;
    RESP Resp;
    int loop_count = 6;
    unsigned int retval;


    if (Stat & ST_NODISK) return Stat;
    
    // Changing the SD Clock to Transfer Rate
    stop_clock(cb);
    configure_clock_rate(cb, 100, INIT_CLOCK);
    start_clock(cb);

    gs_timeout_tick = 16000*20;

    /*Provide 74 Clocks to wake-up SD Card.*/
    do{
        p_sdclk<: SDCLK_16CLOCKS;
    }while(loop_count--);

    sync(p_sdclk);
    
    /* initialize card*/
    sd_reg.CardRCA = 0;
    /* Put the card into idle state */
    if (MD_OK != send_cmd(CMD0, 0, R0, Resp,p_sdclk,p_sdcmd,p_sddata,cb)) return ST_NOINIT;

    /*---- Card is 'idle' state ----*/

    if (MD_OK != send_cmd(CMD8, 0x1AA, R7, Resp,p_sdclk,p_sdcmd,p_sddata,cb)) return ST_NOINIT; /* SDC Ver2 */
    retval = bitrev(Resp[1] | (Resp[2] << 8) | (Resp[3] << 16) | (Resp[4] << 24));

    /* The card can work at vdd range of 2.7-3.6V only */
    /* The card can be  SDHC/XC or SDSC. */
    if ((retval & 0xFFF) == 0x1AA) /* SDC Ver2 */
    {
        /* ACMD41 */
        i = gs_timeout_tick;
      
        do /* Wait while card is busy state (use ACMD41 with HCS bit) */
        {
            if (MD_OK != send_cmd(CMD55, 0, R1, Resp,p_sdclk,p_sdcmd,p_sddata,cb))return ST_NOINIT ;
            if (MD_OK != send_cmd(CMD41, 0x50FF8000, R3, Resp,p_sdclk,p_sdcmd,p_sddata,cb))return ST_NOINIT;
            if(!i--) return ST_NOINIT; // busy timeout
        } while((Resp[1] & 1) == 0); // repeat while busy

        sd_reg.CCS = ((Resp[1] & 2)) ? 1 : 0;
        CardType = (sd_reg.CCS)? CT_SD2|CT_BLOCK : CT_SD2;

    }
    else /* SDC Ver1 */
    {
        /* ACMD41 */
        i = gs_timeout_tick;
    
        do /* Wait while card is busy state (use ACMD41 with HCS bit) */
        {
            if (MD_OK != send_cmd(CMD55, 0, R1, Resp,p_sdclk,p_sdcmd,p_sddata,cb))return ST_NOINIT;
            if (MD_OK != send_cmd(CMD41, 0x00FF8000, R3, Resp,p_sdclk,p_sdcmd,p_sddata,cb))return ST_NOINIT;
            if(!i--) return ST_NOINIT; // busy timeout
        } while((Resp[1] & 1) == 0); // repeat while busy
        
        CardType = CT_SD1;
    }

    /* Save OCR */
    int j = 1;
    for (i=32;i<36;i++){CardInfo[i] =  bitrev(Resp[j++] << 24);}


    /*---- Card is 'ready' state ----*/
    if(MD_OK != send_cmd(CMD2, 0, R2, Resp,p_sdclk,p_sdcmd,p_sddata,cb)) return ST_NOINIT; // get CID
    /* Save CID */
    j = 1;
    for (i=16;i<32;i++){CardInfo[i] =  bitrev(Resp[j++] << 24);}

    /*---- Card is 'identification' state ----*/
    if(MD_OK != send_cmd( CMD3, 0, R6, Resp,p_sdclk,p_sdcmd,p_sddata,cb)) return ST_NOINIT; // get RCA

    /* Save RCA */
    sd_reg.CardRCA = 0xFFFF0000 & bitrev(Resp[1] | (Resp[2] << 8) | (Resp[3] << 16) | (Resp[4] << 24)); // Rca to be used in addressed commands

    /*---- Card is 'stby' state ----*/
    if(MD_OK != send_cmd(CMD9, sd_reg.CardRCA, R2, Resp,p_sdclk,p_sdcmd,p_sddata,cb)) return ST_NOINIT;

    /* Save CSD */
    j = 1;
    for (i=0;i<16;i++){CardInfo[i] =  bitrev(Resp[j++] << 24);}

    /* Evaluate card size*/
    if(0 == (Resp[1] & 0x3)) // CSD ver. 1.0
    {
        BlockLen = bitrev(Resp[6] << 24) & 0x0F; // READ_BL_LEN
        BlockLen = 1 << BlockLen;

        i = ((bitrev(Resp[7]) >> 14) | (bitrev(Resp[8]) >> 22) | (bitrev(Resp[9]) >> 30)) & 0xFFF; // C_SIZE
        sd_reg.BlockNr = ((bitrev(Resp[10]) >> 23) | (bitrev(Resp[11]) >> 31)) & 0x07; // C_SIZE_MULT
        sd_reg.BlockNr = 4 << sd_reg.BlockNr; // MULT
        sd_reg.BlockNr = (i + 1) * sd_reg.BlockNr;
        {sd_reg.BlockNr, BlockLen} = lmul(sd_reg.BlockNr, BlockLen, 0, 0); // evaluate card size bytes
        sd_reg.BlockNr = (sd_reg.BlockNr << 23) | (BlockLen >> 9); // n. of 512 bytes blocks
    }
    else // CSD ver. 2.0: // evaluate card size
    {
        sd_reg.BlockNr = (bitrev(Resp[10]) >> 24) | (bitrev(Resp[9]) >> 16) | (bitrev(Resp[8]) >> 8); // C_SIZE
        sd_reg.BlockNr = (sd_reg.BlockNr + 1)*1024;  // n. of 512 bytes blocks
    }

    /*---- Card is 'tran' state ----*/
    /* Select Card */
    if(MD_OK != send_cmd(CMD7, sd_reg.CardRCA, R1B, Resp,p_sdclk,p_sdcmd,p_sddata,cb)) return ST_NOINIT;

    /* ACMD6 - set bus 4 bit */
    if(MD_OK != send_cmd(CMD55, sd_reg.CardRCA, R1, Resp,p_sdclk,p_sdcmd,p_sddata,cb)) return ST_NOINIT;
    if(MD_OK != send_cmd(CMD6, 0b10, R1, Resp, p_sdclk,p_sdcmd,p_sddata,cb)) return ST_NOINIT;


    // Changing the SD Clock to Transfer Rate
    stop_clock(cb);
    configure_clock_rate(cb, 100,TF_CLOCK);
    start_clock(cb);

    gs_timeout_tick = 4000000;
    
    Stat= ST_INIT;
    return Stat;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
#pragma unsafe arrays
MDRESULT ioctl (sd_host_reg_t &sd_reg,
                   unsigned char ctrl,unsigned char RetVal[],
                   buffered out port:32 p_sdclk,
 [[bidirectional]] buffered port:8 p_sdcmd,
 [[bidirectional]] buffered port:32 p_sddata,
                   clock   cb){

  unsigned long i;
  RESP Resp;

  if (Stat & ST_NOINIT) return MD_NOTRDY;
  if (!wait_ready(500,sd_reg,p_sdclk,p_sdcmd,p_sddata,cb)) return MD_NOTRDY;
  switch (ctrl)
  {
    case CL_SYNC:                /* Make sure that no pending write process */
       break;
    case SECTOR_COUNT: /* Get number of sectors on the disk (DWORD) */
      for(i = 0; i < sizeof(unsigned long); i++)
     unsafe{
        RetVal[i] = (sd_reg.BlockNr, unsigned char[])[i];
          }
    break;

    case BLOCK_SIZE:   /* Get erase block size in unit of sector (DWORD) */
      for(unsigned long Val = 128, i = 0; i < sizeof(unsigned long); i++)
        RetVal[i] = (Val, unsigned long[])[i];
    break;

    case GET_TYPE :     /* Get card type flags (1 byte) */
     RetVal[0] = CardType;
    break;

    case GET_CSD :        /* Get CSD (16 bytes) */
    memcpy(RetVal, &CardInfo[0], 16);
    break;

    case GET_CID :      /* Get CID (16 bytes) */
    memcpy(RetVal, &CardInfo[16], 16);
    break;

    case GET_OCR :      /* Get OCR (4 bytes) */
    memcpy(RetVal, &CardInfo[32], 4);
    break;

    case GET_CARDSTAT:

     if(MD_OK == send_cmd(CMD13, sd_reg.CardRCA, R1, Resp,p_sdclk,p_sdcmd,p_sddata,cb))
     {
         RetVal[0] = Resp[1];
         RetVal[1] = Resp[2];
         RetVal[2] = Resp[3];
         RetVal[3] = Resp[4];
     }

    break;

    default:
    return MD_PARERR;
 }
  return MD_OK;
}


//////////////////////////////////////////////////////////////////////////////////

#pragma unsafe arrays


[[distributable]]
  void sd_host_native(server interface sd_host_if i[num_clients],
                      static const size_t num_clients,
                      buffered out port:32 p_sdclk,
    [[bidirectional]] buffered port:8 p_sdcmd,
    [[bidirectional]] buffered port:32 p_sddata,
                      clock sdClkblk,
                      clock cb_sclk
                     )
{

    sd_host_reg_t sd_reg;
    unsigned int CardDetect = 0;
    RESP Resp;

    /*
    set_clock_on(cb_sclk);
    configure_clock_ref(cb_sclk, 4);
    configure_port_clock_output(p_sdclk, cb_sclk);
    start_clock (cb_sclk);

    stop_clock(cb_sclk);
    */
    
    //p_sdcarddetect:> CardDetect;
    if ((CardDetect & 0x01) != 0) {
        debug_printf("Card Not Detected");
        Stat= ST_NODISK;
    }
    
    if(!isnull(sdClkblk))
    {
        // Initialize the Clocks
        configure_clock_rate(sdClkblk, 100, INIT_CLOCK);
        configure_out_port(p_sdclk, sdClkblk, 1);
        configure_clock_src(cb_sclk,p_sdclk);
    }
    // Initialize the port states
    configure_out_port(p_sdcmd, cb_sclk, 1);
    configure_out_port(p_sddata, cb_sclk, 0xF);

    //configure_port_clock_output(p_sdclk, cb_sclk);

    start_clock(sdClkblk);
    start_clock (cb_sclk);

    
    //while(1);

    timer tim;
    unsigned t1,t2,t3,t4,t5,t6;

    while(1){
            select {
            case i[int x].sd_initialize(void)->unsigned char status:{
                Stat = ST_NOINIT;
                // Changing the SD Clock to Transfer Rate            
                status = initialize(sd_reg,p_sdclk,p_sdcmd,p_sddata,sdClkblk);
            }
            break;
            case i[int x].sd_status(unsigned char drv)->unsigned char status: {

                //p_sdcarddetect:> CardDetect;
                /*if ((CardDetect & 0x1) != 0) 
                {
                    Stat |= (ST_NODISK | ST_NOINIT);
                }
                else
                {
                    Stat &= ~ST_NODISK;
                }*/
                if (Stat & ST_NOINIT) 
                    Stat |= ST_NOINIT;
                else if(send_cmd(CMD13, sd_reg.CardRCA, R1, Resp,p_sdclk,p_sdcmd,p_sddata,sdClkblk))
                    Stat |= ST_NOINIT;  
                status = Stat;
            }
            break;
             /*Read Sector(s)*/
            case i[int x].sd_read(unsigned char *buff, unsigned long sector, unsigned int count)->unsigned int result:{

                if (Stat & ST_NOINIT) result= MD_NOTRDY;
                if (!count) result = MD_PARERR;
                if (!wait_ready(500,sd_reg,p_sdclk,p_sdcmd,p_sddata,sdClkblk)) result = MD_NOTRDY;

                /* Single Block Read */
                if (count == 1)   
                {
                    if((send_cmd(CMD17, sd_reg.CCS ? sector : 512 * sector, R1, Resp,p_sdclk,p_sdcmd,p_sddata,sdClkblk) == MD_OK))
                    {
                        result = read_datablock(count, buff,p_sdclk,p_sdcmd,p_sddata,sdClkblk);
                    }
                }
                else  /* Multiple Block Read */
                {
                    if((send_cmd(CMD18, sd_reg.CCS ? sector : 512 * sector, R1, Resp,p_sdclk,p_sdcmd,p_sddata,sdClkblk)== MD_OK))
                    {
                        result = read_datablock(count, buff,p_sdclk,p_sdcmd,p_sddata,sdClkblk);
                    }
                    if(MD_OK != send_cmd(CMD12, 0, R1, Resp,p_sdclk,p_sdcmd,p_sddata,sdClkblk))  result = MD_ERROR;
                }

            }
            break;
            /*Write Sector(s)*/
            case i[int x].sd_write(const unsigned char *buff, unsigned long sector, unsigned int count)->unsigned int result:{
                tim :> t1;

                if (Stat & ST_NOINIT) result= MD_NOTRDY;
                if (!count) result = MD_PARERR;
                
                if (!wait_ready(500,sd_reg,p_sdclk,p_sdcmd,p_sddata,sdClkblk)) result = MD_NOTRDY;
                
                /* Single Block Write *//* Multiple Block Write */

                /*
                if(count == 1)
                {
                    if(MD_OK == send_cmd(CMD24, sd_reg.CCS ? sector : 512 * sector, R1, Resp,p_sdclk,p_sdcmd,p_sddata,sdClkblk))
                    {
                        result = write_datablock(count,(*buff, unsigned char[]),p_sdclk,p_sdcmd,p_sddata,sdClkblk);
                    }
                }
                else
                */
                {
                    if(send_cmd(CMD55, sd_reg.CardRCA, R1, Resp,p_sdclk,p_sdcmd,p_sddata,sdClkblk))result = MD_ERROR;
                    if(send_cmd(CMD23, count, R1, Resp,p_sdclk,p_sdcmd,p_sddata,sdClkblk)) result = MD_ERROR;
                    
                    tim :> t3;
                    if(MD_OK == send_cmd(CMD25, sd_reg.CCS  ? sector : 512 * sector, R1, Resp,p_sdclk,p_sdcmd,p_sddata,sdClkblk))
                    {
                       result = write_datablock(count,(*buff, unsigned char[]),p_sdclk,p_sdcmd,p_sddata,sdClkblk);
                    }
                    if(MD_OK != send_cmd(CMD12, 0, R1B, Resp,p_sdclk,p_sdcmd,p_sddata,sdClkblk)) result = MD_ERROR;
                    
                    tim :> t4;
                    if(t4-t3 >50*100000)
                        text_debug("sd nb write %dms\n",(t4-t3)/100000);
                }
                
                tim :> t2;
                if(t2-t1 >50*100000)
                    text_debug("sd w %dms\n",(t2-t1)/100000);

                
             }
             break;
             /* Miscellaneous Functions */
            case i[int x].sd_ioctl(unsigned char *buff,unsigned char ctrl)->unsigned int result:{
                result= ioctl (sd_reg,ctrl,buff,p_sdclk,p_sdcmd,p_sddata,sdClkblk);
            }
            break;
          } //select

    }//while loop

}


// User Provided Timer Function for FatFs module
unsigned long get_fattime(void)
{
  return ((unsigned long)(2010 - 1980) << 25)  /* Fixed to Jan. 1, 2010 */
          | ((unsigned long)1 << 21)
          | ((unsigned long)1 << 16)
          | ((unsigned long)0 << 11)
          | ((unsigned long)0 << 5)
          | ((unsigned long)0 >> 1);
}

