// Copyright (c) 2011-2016, XMOS Ltd, All rights reserved
#include <platform.h>
#include <string.h>
#include <stdlib.h>
#include "xassert.h"
#include "print.h"
#include "tftp.h"
//#include "tftp_app.h"
#include <debug_print.h>

static unsigned short prev_block_num = 0;
static int tftp_block_size = 0;

static void reverse_array(char buf[], unsigned size)
{
  int begin = 0;
  int end = size - 1;
  int tmp;
  for (;begin < end; begin++,end--) {
    tmp = buf[begin];
    buf[begin] = buf[end];
    buf[end] = tmp;
  }
}
static int itoa(unsigned n, char *buf, unsigned base, int fill)
{
  static const char digits[] = "0123456789ABCDEF";
  unsigned i = 0;

  if (n==0 && fill==0)
    fill += 1;

  while (n > 0) {
    unsigned next = n / base;
    unsigned cur  = n % base;
    buf[i] = digits[cur];
    i += 1;
    fill--;
    n = next;
  }
  for (;fill > 0; fill--) {
    buf[i] = '0';
    i++;
  }
  reverse_array(buf, i);
  buf[i] = 0;
  return i;
}

static char* memstr(char* full_data, int full_data_len, const char* substr)
{
    if (full_data == NULL || full_data_len <= 0 || substr == NULL) {
        return NULL;
    }
 
    if (*substr == '\0') {
        return NULL;
    }
 
    int sublen = strlen(substr);
 
    int i;
    char* cur = full_data;
    int last_possible = full_data_len - sublen + 1;
    for (i = 0; i < last_possible; i++) {
        if (*cur == *substr) {
            //assert(full_data_len - i >= sublen);
            if (memcmp(cur, substr, sublen) == 0) {
                //found
                return cur;
            }
        }
        cur++;
    }
 
    return NULL;
}
static const char str_timeout[] = "timeout";
static const char str_tsize[] = "tsize";
static const char str_blksize[] = "blksize";

int tftp_get_timeout(unsigned char playload[])
{
    char *p_fsize = memstr((char*)playload, 150, str_timeout);
    if(p_fsize == 0) return -1;
    
    p_fsize += sizeof(str_timeout);
    
    return atoi(p_fsize);
}

int tftp_get_tsize(unsigned char playload[])
{
    char *p_fsize = memstr((char*)playload, 150, str_tsize);
    if(p_fsize == 0) return -1;
    
    p_fsize += sizeof(str_tsize);
    
    return atoi(p_fsize);
}

int tftp_get_blksize(unsigned char playload[])
{
    char *p_fsize = memstr((char*)playload, 150, str_blksize);
    if(p_fsize == 0) {
        return -1;
    }
    
    p_fsize += sizeof(str_blksize);
    
    return atoi(p_fsize);
}

static int tftp_make_expand_ack_pkt(unsigned char tx_buf[], int write_mode, unsigned char option[], unsigned int option_size)
{
    int blksize = 0;
    int tsize = 0;
    int timeout = 0;
    int offset = 0;
    tftp_expand_ack_t *pkt = (tftp_expand_ack_t*) &tx_buf[0];

    pkt->opcode = hton16((unsigned short) TFTP_OPCODE_EXPAND);
    
#if 0
    memcpy(pkt->data, option, option_size);
    offset = option_size;
#else
    blksize = tftp_get_blksize((unsigned char*)option);
    tsize = tftp_get_tsize((unsigned char*)option);
    timeout = tftp_get_timeout((unsigned char*)option);
    
    // blksize字段
    if(blksize > 0)
    {
        memcpy(pkt->data+offset, str_blksize, sizeof(str_blksize));
        offset += sizeof(str_blksize);
        offset = offset+itoa(blksize, (char*)pkt->data+offset, 10, 0)+1;
    }
    // tsize字段, 只有在GET时才会需要回复tsize字段
    if((tsize==0) || (write_mode==0))//
    {
        memcpy(pkt->data+offset, str_tsize, sizeof(str_tsize));
        offset += sizeof(str_tsize);
        tsize = 3*1024*1024;
        offset = offset+itoa(tsize, (char*)pkt->data+offset, 10, 0)+1;
    }
#endif

#if 1||TFTP_DEBUG_PRINT
    debug_printf("TFTP: Gen EXPAND ACK %d\n", offset);
#endif

    return 2+offset;
}

int tftp_make_ack_pkt(unsigned char tx_buf[], unsigned short block_num)
{
    tftp_ack_t *pkt = (tftp_ack_t*) &tx_buf[0];

    pkt->opcode = hton16((unsigned short) TFTP_OPCODE_ACK);
    pkt->block_number = hton16(block_num);

#if TFTP_DEBUG_PRINT
    debug_printf("TFTP: Gen ACK, block #%d\n", block_num);
#endif

    return TFTP_MIN_PKT_SIZE;
}

#if TFTP_READ_REQUEST_ENABLE
int tftp_make_send_pkt(unsigned char tx_buf[], unsigned short block_num, int *complete)
{
    tftp_ack_t *pkt = (tftp_ack_t*) &tx_buf[0];
    
    pkt->opcode = hton16((unsigned short) TFTP_OPCODE_DATA);
    pkt->block_number = hton16(block_num);

#if TFTP_DEBUG_PRINT
    debug_printf("TFTP: Gen send, block #%d\n", block_num);
#endif

    return tftp_app_process_send_data_block(tx_buf+TFTP_MIN_PKT_SIZE, block_num, tftp_block_size, complete)+TFTP_MIN_PKT_SIZE;
}
#endif


int tftp_make_error_pkt(unsigned char tx_buf[], unsigned short code, const char msg[], int *error)
{
    tftp_error_t *pkt = (tftp_error_t*) &tx_buf[0];

    if (error != 0)
    {
        *error = 1;
    }

    // The length of the error message must always be less than TFTP_ERROR_MSG_MAX_LENGTH
    assert(strlen(msg) < TFTP_ERROR_MSG_MAX_LENGTH);

    pkt->opcode = hton16((unsigned short) TFTP_OPCODE_ERROR);
    pkt->error_code = hton16(code);

    strcpy(pkt->error_msg, msg);

#if TFTP_DEBUG_PRINT
    debug_printf("TFTP: Gen ERROR, code %d\n", code);
#endif

    return (TFTP_MIN_PKT_SIZE + strlen(msg) + TFTP_NULL_BYTE);
}

int tftp_process_app_error(unsigned char *tx_buf, unsigned char data)
{
    char error[] = "Application error:  ";
    error[strlen(error)-1] = '0'+(data%10);
    if(data>=10)error[strlen(error)-2] = '0'+data/10;
    return tftp_make_error_pkt(tx_buf, TFTP_ERROR_NOT_DEFINED, error, 0);
}


int tftp_process_packet(unsigned char *tx_buf, unsigned char *rx_buf, int num_bytes, unsigned short *block_num_glob, int *write_mode, int *can_put_data, int *error, int *complete)
{
    tftp_packet_t *pkt = (tftp_packet_t*) &rx_buf[0];

    n16_t opcode = ntoh16(pkt->opcode);

    if(opcode == TFTP_OPCODE_RRQ)
        *write_mode = 0;
    else if(opcode == TFTP_OPCODE_WRQ)
        *write_mode = 1;
    
    switch (opcode)
    {
        case TFTP_OPCODE_RRQ: // Read Request
             
#if !TFTP_READ_REQUEST_ENABLE
        {
            // We don't support read requests - reply with an error packet
            return tftp_make_error_pkt(tx_buf, TFTP_ERROR_NOT_DEFINED, "Read not supported", error);
        }
#endif
        case TFTP_OPCODE_WRQ: // Write Request
        {
            char *filename;
            char *mode;
            char *option;        
            filename = (char *) pkt->payload;

#if !TFTP_ACCEPT_ANY_FILENAME
            // Check that the requested filename matches what we expect
            if (strncmp(filename, TFTP_IMAGE_FILENAME, strlen(TFTP_IMAGE_FILENAME)) != 0)
            {
                return tftp_make_error_pkt(tx_buf, TFTP_ERROR_NOT_DEFINED, "Invalid filename", error);
            }
#endif

            // Generate a pointer to the mode string
            mode = filename + strlen(filename) + TFTP_NULL_BYTE;

            // Must be a binary transfer
            if (strncmp(mode, "octet", 5) != 0)
            {
                return tftp_make_error_pkt(tx_buf, TFTP_ERROR_NOT_DEFINED, "Invalid transfer mode", error);
            }

            
            option = mode+strlen(mode)+TFTP_NULL_BYTE;

            prev_block_num = 0;
            
            tftp_block_size = tftp_get_blksize((unsigned char*)option);
            tftp_block_size = (tftp_block_size<0)?0:tftp_block_size;
            
            if(*write_mode)
            {
                if(tftp_block_size)
                {
                    return tftp_make_expand_ack_pkt(tx_buf, 1, (unsigned char*)option, num_bytes-((int)option-(int)rx_buf));
                }
                else
                {
                    tftp_block_size = 512;
                    // ACK with data block number zero
                    return tftp_make_ack_pkt(tx_buf, 0);                
                }
            }
            else
            {
#if TFTP_READ_REQUEST_ENABLE
                if(tftp_block_size)
                {
                    return tftp_make_expand_ack_pkt(tx_buf, 0, (unsigned char*)option, num_bytes-((int)option-(int)rx_buf));
                }
                else
                {
                    tftp_block_size = 512;
                    // ACK with data block number zero
                    return tftp_make_send_pkt(tx_buf, 1, complete);           
                }                
#endif
            }
        }
        case TFTP_OPCODE_DATA: // Data Packet
        {
            unsigned short block_num;
            tftp_data_t *data_pkt = (tftp_data_t*) &rx_buf[0];

            if (num_bytes < (tftp_block_size + TFTP_MIN_PKT_SIZE))
            {
                // last block
                *complete = 1;
            }

            block_num = ntoh16(data_pkt->block_num);

            *block_num_glob = block_num;

            // Check that we've received the correct block of data and it's not a duplicate
            if (block_num == (unsigned short)(prev_block_num + 1))
            {
                *can_put_data = 1;
                
                prev_block_num = block_num;

#if TFTP_DEBUG_PRINT
                debug_printf("TFTP: Rcvd data, block #%d\n", block_num);
#endif

                if ((block_num * tftp_block_size) >= TFTP_MAX_FILE_SIZE)
                {
                    // We have received more data that the allowed maximum - send an error
                    return tftp_make_error_pkt(tx_buf, TFTP_ERROR_DISK_FULL, "", error);
                }

                // Here the data is passed to the application for processing. It can signal an error to TFTP
                // by returning a non-zero value.
//                if (tftp_app_process_data_block(data_pkt->data, num_bytes - TFTP_MIN_PKT_SIZE) != 0)
//                {
//                    // We send an access violation error, but this could be modified to send a custom
//                    // error from the application layer */
//                    return tftp_make_error_pkt(tx_buf, TFTP_ERROR_ACCESS_VIOLATION, "", error);
//                }
            }
            else
            {
                *can_put_data = 0;
#if 1||TFTP_DEBUG_PRINT
                debug_printf("TFTP: Rvcd invalid data, block #%d %d\n", block_num, (unsigned short)(prev_block_num + 1));
#endif

            }

            // Make the ACK packet for the received data
            return tftp_make_ack_pkt(tx_buf, block_num);
        }
        case TFTP_OPCODE_ACK: // Acknowledgement
        {
#if !TFTP_READ_REQUEST_ENABLE
            // We never expect to receive an ACK packet from the active connection
            return tftp_make_error_pkt(tx_buf, TFTP_ERROR_ILLEGAL_OPERATION, "", error);
#else
            unsigned short block_num;
            tftp_data_t *data_pkt = (tftp_data_t*) &rx_buf[0];

            block_num = ntoh16(data_pkt->block_num);
            
#if TFTP_DEBUG_PRINT
            debug_printf("TFTP: Rcvd Ack, block #%d\n", block_num);
#endif

            *block_num_glob = block_num;
            
            return tftp_make_send_pkt(tx_buf, block_num+1, complete);
#endif      
        }
        case TFTP_OPCODE_ERROR: // Error packet
        {
            return -1;
        }
        default:
        {
            // Error: Not a valid TFTP opcode
            return tftp_make_error_pkt(tx_buf, TFTP_ERROR_ILLEGAL_OPERATION, "", error);
        }
    }

}

