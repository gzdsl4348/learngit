#ifndef _SDIO_SDCARD_H_
#define _SDIO_SDCARD_H_

////////////////////////////////////////////////////////////////////////////////////////////////////
//SDIO 指令集
#define SD_CMD_GO_IDLE_STATE                       ((unsigned char)0)
#define SD_CMD_SEND_OP_COND                        ((unsigned char)1)
#define SD_CMD_ALL_SEND_CID                        ((unsigned char)2)
#define SD_CMD_SET_REL_ADDR                        ((unsigned char)3) /*!< SDIO_SEND_REL_ADDR for SD Card */
#define SD_CMD_SET_DSR                             ((unsigned char)4)
#define SD_CMD_SDIO_SEN_OP_COND                    ((unsigned char)5)
#define SD_CMD_HS_SWITCH                           ((unsigned char)6)
#define SD_CMD_SEL_DESEL_CARD                      ((unsigned char)7)
#define SD_CMD_HS_SEND_EXT_CSD                     ((unsigned char)8)
#define SD_CMD_SEND_CSD                            ((unsigned char)9)
#define SD_CMD_SEND_CID                            ((unsigned char)10)
#define SD_CMD_READ_DAT_UNTIL_STOP                 ((unsigned char)11) /*!< SD Card doesn't support it */
#define SD_CMD_STOP_TRANSMISSION                   ((unsigned char)12)
#define SD_CMD_SEND_STATUS                         ((unsigned char)13)
#define SD_CMD_HS_BUSTEST_READ                     ((unsigned char)14)
#define SD_CMD_GO_INACTIVE_STATE                   ((unsigned char)15)
#define SD_CMD_SET_BLOCKLEN                        ((unsigned char)16)
#define SD_CMD_READ_SINGLE_BLOCK                   ((unsigned char)17)
#define SD_CMD_READ_MULT_BLOCK                     ((unsigned char)18)
#define SD_CMD_HS_BUSTEST_WRITE                    ((unsigned char)19)
#define SD_CMD_WRITE_DAT_UNTIL_STOP                ((unsigned char)20) 
#define SD_CMD_SET_BLOCK_COUNT                     ((unsigned char)23) 
#define SD_CMD_WRITE_SINGLE_BLOCK                  ((unsigned char)24)
#define SD_CMD_WRITE_MULT_BLOCK                    ((unsigned char)25)
#define SD_CMD_PROG_CID                            ((unsigned char)26)
#define SD_CMD_PROG_CSD                            ((unsigned char)27)
#define SD_CMD_SET_WRITE_PROT                      ((unsigned char)28)
#define SD_CMD_CLR_WRITE_PROT                      ((unsigned char)29)
#define SD_CMD_SEND_WRITE_PROT                     ((unsigned char)30)
#define SD_CMD_SD_ERASE_GRP_START                  ((unsigned char)32) /*!< To set the address of the first write
                                                                  block to be erased. (For SD card only) */
#define SD_CMD_SD_ERASE_GRP_END                    ((unsigned char)33) /*!< To set the address of the last write block of the
                                                                  continuous range to be erased. (For SD card only) */
#define SD_CMD_ERASE_GRP_START                     ((unsigned char)35) /*!< To set the address of the first write block to be erased.
                                                                  (For MMC card only spec 3.31) */

#define SD_CMD_ERASE_GRP_END                       ((unsigned char)36) /*!< To set the address of the last write block of the
                                                                  continuous range to be erased. (For MMC card only spec 3.31) */

#define SD_CMD_ERASE                               ((unsigned char)38)
#define SD_CMD_FAST_IO                             ((unsigned char)39) /*!< SD Card doesn't support it */
#define SD_CMD_GO_IRQ_STATE                        ((unsigned char)40) /*!< SD Card doesn't support it */
#define SD_CMD_LOCK_UNLOCK                         ((unsigned char)42)
#define SD_CMD_APP_CMD                             ((unsigned char)55)
#define SD_CMD_GEN_CMD                             ((unsigned char)56)
#define SD_CMD_NO_CMD                              ((unsigned char)64)

/** 
  * @brief Following commands are SD Card Specific commands.
  *        SDIO_APP_CMD ：CMD55 should be sent before sending these commands. 
  */
#define SD_CMD_APP_SD_SET_BUSWIDTH                 ((unsigned char)6)  /*!< For SD Card only */
#define SD_CMD_SD_APP_STAUS                        ((unsigned char)13) /*!< For SD Card only */
#define SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS        ((unsigned char)22) /*!< For SD Card only */
#define SD_CMD_SD_APP_OP_COND                      ((unsigned char)41) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT          ((unsigned char)42) /*!< For SD Card only */
#define SD_CMD_SD_APP_SEND_SCR                     ((unsigned char)51) /*!< For SD Card only */
#define SD_CMD_SDIO_RW_DIRECT                      ((unsigned char)52) /*!< For SD I/O Card only */
#define SD_CMD_SDIO_RW_EXTENDED                    ((unsigned char)53) /*!< For SD I/O Card only */

/** 
  * @brief Following commands are SD Card Specific security commands.
  *        SDIO_APP_CMD should be sent before sending these commands. 
  */
#define SD_CMD_SD_APP_GET_MKB                      ((unsigned char)43) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_MID                      ((unsigned char)44) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CER_RN1                  ((unsigned char)45) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_CER_RN2                  ((unsigned char)46) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CER_RES2                 ((unsigned char)47) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_CER_RES1                 ((unsigned char)48) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK   ((unsigned char)18) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  ((unsigned char)25) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_ERASE                 ((unsigned char)38) /*!< For SD Card only */
#define SD_CMD_SD_APP_CHANGE_SECURE_AREA           ((unsigned char)49) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MKB             ((unsigned char)48) /*!< For SD Card only */

//支持的SD卡定义
#define SDIO_STD_CAPACITY_SD_CARD_V1_1             ((unsigned int)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0             ((unsigned int)0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD                 ((unsigned int)0x00000002)
#define SDIO_MULTIMEDIA_CARD                       ((unsigned int)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD                ((unsigned int)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD            ((unsigned int)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD          ((unsigned int)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD                ((unsigned int)0x00000007)


//SDIO相关参数定义
//#define NULL 0
#define SDIO_STATIC_FLAGS               ((unsigned int)0x000005FF)
#define SDIO_CMD0TIMEOUT                ((unsigned int)0x00010000)	  
#define SDIO_DATATIMEOUT                ((unsigned int)0xFFFFFFFF)	  
#define SDIO_FIFO_Address               ((unsigned int)0x40018080)

//Mask for errors Card Status R1 (OCR Register)  
#define SD_OCR_ADDR_OUT_OF_RANGE        ((unsigned int)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((unsigned int)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((unsigned int)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((unsigned int)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((unsigned int)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((unsigned int)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((unsigned int)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((unsigned int)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((unsigned int)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((unsigned int)0x00200000)
#define SD_OCR_CC_ERROR                 ((unsigned int)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((unsigned int)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((unsigned int)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((unsigned int)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE       ((unsigned int)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((unsigned int)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((unsigned int)0x00004000)
#define SD_OCR_ERASE_RESET              ((unsigned int)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((unsigned int)0x00000008)
#define SD_OCR_ERRORBITS                ((unsigned int)0xFDFFE008)

//Masks for R6 Response 
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((unsigned int)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((unsigned int)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((unsigned int)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((unsigned int)0x80100000)
#define SD_HIGH_CAPACITY                ((unsigned int)0x40000000)
#define SD_STD_CAPACITY                 ((unsigned int)0x00000000)
#define SD_CHECK_PATTERN                ((unsigned int)0x000001AA)
#define SD_VOLTAGE_WINDOW_MMC           ((unsigned int)0x80FF8000)

#define SD_MAX_VOLT_TRIAL               ((unsigned int)0x0000FFFF)
#define SD_ALLZERO                      ((unsigned int)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((unsigned int)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((unsigned int)0x00010000)
#define SD_CARD_LOCKED                  ((unsigned int)0x02000000)
#define SD_CARD_PROGRAMMING             ((unsigned int)0x00000007)
#define SD_CARD_RECEIVING               ((unsigned int)0x00000006)
#define SD_DATATIMEOUT                  ((unsigned int)0xFFFFFFFF)
#define SD_0TO7BITS                     ((unsigned int)0x000000FF)
#define SD_8TO15BITS                    ((unsigned int)0x0000FF00)
#define SD_16TO23BITS                   ((unsigned int)0x00FF0000)
#define SD_24TO31BITS                   ((unsigned int)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((unsigned int)0x01FFFFFF)

#define SD_HALFFIFO                     ((unsigned int)0x00000008)
#define SD_HALFFIFOBYTES                ((unsigned int)0x00000020)

//Command Class Supported  
#define SD_CCCC_LOCK_UNLOCK             ((unsigned int)0x00000080)
#define SD_CCCC_WRITE_PROT              ((unsigned int)0x00000040)
#define SD_CCCC_ERASE                   ((unsigned int)0x00000020)
																	 
//CMD8指令
#define SDIO_SEND_IF_COND               ((unsigned int)0x00000008)

#endif

