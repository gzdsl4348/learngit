#include "pc_config_tool.h"
#include "debug_print.h"
#include "list_instance.h"
#include "user_unti.h"



#define  CFG_INDEX_IPADDR     2
#define  CFG_INDEX_NETMASK    6
#define  CFG_INDEX_GATEWAY    10
#define  CFG_INDEX_HOSTIP     14
#define  CFG_INDEX_IPTYPE     26

const uint8_t CHECK_DEV_INFO[6] = {0x5A,0,0,0,0,0};  
const uint8_t CONFIG_DEV_INFO[2] = {0x5A,0x01}; 
const uint8_t NUM_TAB[11]={"0123456789"};


static void num2char(uint8_t num, uint8_t out_tmp[])
{
	if(num < 10)
	{
		out_tmp[0] = NUM_TAB[num];
		out_tmp[1] = '.';
		out_tmp[2] = 0;
		out_tmp[3] = 0;
		out_tmp[4] = 0;
	}
	else if(num < 100)
	{
		out_tmp[0] = NUM_TAB[num/10];
		out_tmp[1] = NUM_TAB[num%10];
		out_tmp[2] = '.';
		out_tmp[3] = 0;
		out_tmp[4] = 0;
	}
	else 
	{
		out_tmp[0] = NUM_TAB[num/100];
		out_tmp[1] = NUM_TAB[(num/10)%10];
		out_tmp[2] = NUM_TAB[num%10];
		out_tmp[3] = '.';
		out_tmp[4] = 0;
	}
}

static void mac_hex2char(uint8_t num, uint8_t out_tmp[])
{
	uint8_t h_tmp=0, l_tmp=0;
	h_tmp = (num & 0xF0)>>4;
	l_tmp = num & 0x0F;
	
	if(h_tmp < 10)
	{
		out_tmp[0] = h_tmp + '0';
	}
	else 
	{
		out_tmp[0] = h_tmp - 10 + 'A';
	}

	if(l_tmp < 10)
	{
		out_tmp[1] = l_tmp + '0';
	}
	else 
	{
		out_tmp[1] = l_tmp - 10 + 'A';
	}

	out_tmp[2] = '-';
}

static void feedback_dev_info(client xtcp_if i_xtcp, xtcp_connection_t conn, n_pc_cfg_t old_param, uint8_t ackflag)
{
	uint8_t ip_head[]      = {"IP: "};
	uint8_t netmask_head[] = {" SUBNETMASK: "};
	uint8_t gateway_head[] = {" GATEWAY: "};
	uint8_t server_head[]  = {" SERVER: "};
	uint8_t mac_head[]     = {" MAC: "};
	uint8_t iptype_head[]  = {" IPTYPE: "};
	//uint8_t send_buf[256]={"IP: 172.16.13.166 SUBNETMASK: 255.255.255.0 GATEWAY: 172.16.13.254 SERVER: 172.16.13.112 MAC: 42-4c-45-00-ac-4f IPTYPE: 0"};
	uint8_t send_buf[256];
    uint8_t index = 0;
    uint8_t tmp_tab[5]={0,0,0,0,0}; 

    xtcp_ipconfig_t ipconfig;
    i_xtcp.get_ipconfig(ipconfig);

    //IP
    memcpy(&send_buf[index], ip_head, strlen(ip_head)); 
    index += strlen(ip_head);
    for(uint8_t i=0; i<4; i++)
    {
		num2char(old_param.ipcfg.ipaddr[i], tmp_tab);
		memcpy(&send_buf[index], tmp_tab, strlen(tmp_tab)); 
		index += strlen(tmp_tab);
    }
    index -= 1;

    //SUBNETMASK
    memcpy(&send_buf[index], netmask_head, strlen(netmask_head)); 
    index += strlen(netmask_head);
    for(uint8_t i=0; i<4; i++)
    {
		num2char(old_param.ipcfg.netmask[i], tmp_tab);
		memcpy(&send_buf[index], tmp_tab, strlen(tmp_tab)); 
		index += strlen(tmp_tab);
    }
    index -= 1;

    //GATEWAY
    memcpy(&send_buf[index], gateway_head, strlen(gateway_head)); 
    index += strlen(gateway_head);
    for(uint8_t i=0; i<4; i++)
    {
		num2char(old_param.ipcfg.gateway[i], tmp_tab);
		memcpy(&send_buf[index], tmp_tab, strlen(tmp_tab)); 
		index += strlen(tmp_tab);
    }
    index -= 1;

	//SERVER
    memcpy(&send_buf[index], server_head, strlen(server_head)); 
    index += strlen(server_head);
    for(uint8_t i=0; i<4; i++)
    {
		num2char(old_param.server_ip[i], tmp_tab);
		memcpy(&send_buf[index], tmp_tab, strlen(tmp_tab)); 
		index += strlen(tmp_tab);
    }
    index -= 1;
 
    //MAC
    uint8_t mactmp[3]={0,0,0};
    memcpy(&send_buf[index], mac_head, strlen(mac_head)); 
    index += strlen(mac_head);
    for(uint8_t i=0; i<6; i++)
    {
		mac_hex2char(old_param.mac[i], mactmp);
		memcpy(&send_buf[index], mactmp, 3); 
		index += 3;
    }
    index -= 1;
 
    //IPTYPE
	memcpy(&send_buf[index], iptype_head, strlen(iptype_head)); 
    index += strlen(iptype_head);
    if(old_param.iptype)
    {
		send_buf[index] = '1';
    }
    else
    {
		send_buf[index] = '0';
    }
    index += 1;
    
    if(ackflag == 0)
    {
		i_xtcp.send_udp(conn, send_buf, index);
    }
    else
    {
		xtcp_connection_t broadcast_conn;
		xtcp_ipaddr_t broadcast_ipaddr = {255,255,255,255};
		if(i_xtcp.connect_udp(conn.remote_port, broadcast_ipaddr, broadcast_conn))
		{
		    return;
		}
		i_xtcp.send_udp(broadcast_conn, send_buf, index);
		i_xtcp.close_udp(broadcast_conn);
    }
	
	//debug_printf("                feedback_dev_info\n");
}


uint8_t pc_config_handle(client xtcp_if i_xtcp, xtcp_connection_t &conn, uint8_t rxbuff[], 
                         uint32_t rxlen)
{
	if((conn.local_port != PC_CONFIG_TOOL_PORT) || (rxlen < 2))
	{
		return 0;
	}
    n_pc_cfg_t old_param;
    memcpy(&old_param.ipcfg,&host_info.ipconfig,sizeof(xtcp_ipconfig_t));
    memcpy(old_param.mac,host_info.mac,6);
    memcpy(old_param.server_ip,host_info.ipconfig.ipaddr,4);

	if((memcmp(rxbuff, CONFIG_DEV_INFO, 2) == 0) && (rxlen>=27))
	{
		//debug_printf("                config_dev_info :%d\n", rxlen);
		//for(uint16_t i=0; i<rxlen; i++)
		//debug_printf("%d ",rxbuff[i]);
		//debug_printf("\n");
		//5A 01 AC 10 0D A6 FF FF FF 00 AC 10 0D FE AC 10 0D 70 00 00 00 00 00 00 00 00 01

        //new_param.iptype = rxbuff[CFG_INDEX_IPTYPE];
		memcpy(host_info.ipconfig.ipaddr,  &rxbuff[CFG_INDEX_IPADDR], 4);
		memcpy(host_info.ipconfig.netmask, &rxbuff[CFG_INDEX_NETMASK], 4);
		memcpy(host_info.ipconfig.gateway, &rxbuff[CFG_INDEX_GATEWAY], 4);
		//memcpy(new_param.server_ip,  &rxbuff[CFG_INDEX_HOSTIP], 4);

        feedback_dev_info(i_xtcp, conn, old_param, 1);
		return 1;
    }
	else if(memcmp(rxbuff, CHECK_DEV_INFO, 6) == 0)
	{
		feedback_dev_info(i_xtcp, conn, old_param, 1);
	}
	
	return 0;
}

