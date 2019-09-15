#ifndef  __XTCP_USER_H_
#define  __XTCP_USER_H_

#include <xs1.h>
#include <platform.h>
#include "xtcp.h"
#include "eth_audio.h"
#include "file.h"
#include "flash_user.h"
#include "image_upgrade.h"
#include "uart.h"

#include "aud_trainsmit_core.h"

void xtcp_uesr(client xtcp_if i_xtcp,client ethaud_cfg_if if_ethaud_cfg,client fl_manage_if if_fl_manage,client file_server_if if_fs,
                  client uart_tx_buffered_if if_uart_tx,client uart_rx_if if_uart_rx,client image_upgrade_if i_image,client aud_trainsmit_if if_aud_trainsmit);


#endif 	//__XTCP_USER_H_

