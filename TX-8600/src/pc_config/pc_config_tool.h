#ifndef _PC_CONFIG_TOOL_H_
#define _PC_CONFIG_TOOL_H_

#include <platform.h>
#include <stdio.h>
#include "xtcp.h"
#include "string.h"

#define  PC_CONFIG_TOOL_PORT  5121

typedef struct
{
    xtcp_ipconfig_t ipcfg;
    uint8_t server_ip[4];
    uint8_t mac[6];
    uint8_t iptype;
} n_pc_cfg_t;

uint8_t pc_config_handle(client xtcp_if i_xtcp, xtcp_connection_t &conn, uint8_t rxbuff[], 
                         uint32_t rxlen);

#endif // _PC_CONFIG_TOOL_H_

