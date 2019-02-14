#include "mac_factory_write.h"
#include "factory_mac_packbiu.h"
#include "debug_print.h"
#include "string.h"

#ifndef INIT_VAL
#define INIT_VAL -1
#endif

#define MAC_FACTORY_WRITE_PORT 50000


static xtcp_connection_t responding_connection = {0, INIT_VAL};
static unsigned response_len=0;
static uint16_t divce_id=0;

static char mac_factory_flag = 0;
static mac_facinfo_t  t_mac_facinfo;

int mac_factory_init(client xtcp_if i_xtcp, uint8_t mac[])
{
	unsafe{
	if ( (((uint32_t *)mac)[0]!=0x00454C42) ||
		  (((uint16_t *)mac)[2]!=0x00))
		return 0;	//MAC Not ture

	xtcp_ipconfig_t ipconfig=
    {
        {192,168,168,168},
        {255,255,255,0},
        {0,0,0,0}
     };
    //------------------------------------------------
	// Set ip :192.168.168.168
	// Set mac:42 4C 45 00 00 00
	i_xtcp.xtcp_init(ipconfig,mac);
    memcpy(t_mac_facinfo.mac,mac,6);
    mac_factory_flag = 1;
   }
    return 1;
}
    


void mac_fatctory_xtcp_event(client xtcp_if i_xtcp, xtcp_connection_t &conn, unsigned char rx_buffer[], unsigned int data_len)
{
    // Check if this connection is TFTP
    if (conn.event != XTCP_IFUP && conn.event != XTCP_IFDOWN &&
        conn.local_port != MAC_FACTORY_WRITE_PORT)
    {
        return;
    }
    
    switch (conn.event){
    case XTCP_IFUP:
        // Show the IP address of the interface
#if 0						
        xtcp_ipconfig_t ipconfig;
        i_xtcp.get_ipconfig(ipconfig);

        debug_printf("Link up\n IP Adr: %d,%d,%d,%d\n",ipconfig.ipaddr[0],
                                                         ipconfig.ipaddr[1],
                                                         ipconfig.ipaddr[2],
                                                         ipconfig.ipaddr[3]);    
        
#endif
        //---------------------------------
        // build listening form eth PORT
        i_xtcp.listen(MAC_FACTORY_WRITE_PORT, XTCP_PROTOCOL_UDP);
        break;
    case XTCP_IFDOWN:
        i_xtcp.unlisten(MAC_FACTORY_WRITE_PORT);
#if 0
        debug_printf("Link down\n");
#endif
        break;
    case XTCP_NEW_CONNECTION:
#if 1
        debug_printf("MAC FATCTORY new connection:%d\n", conn.id);
#endif
        if(responding_connection.id==INIT_VAL)
            responding_connection=conn;
        else
            i_xtcp.close(conn);
        break;
    case XTCP_RECV_DATA:
        if(divce_id==0){
            uint32_t tmptime;
            timer tmr;
            tmr:> tmptime;
            tmptime+=(tmptime>>16);
            divce_id=(uint16_t)tmptime;
        }   
        if(rx_buffer[0]!=0xAA){
            responding_connection.id = INIT_VAL;
            i_xtcp.close(conn);
            break;
        }
        switch (rx_buffer[5])
        {
            case maccom_signalcheck:
                response_len = macsingle_recheck_bulid(rx_buffer,t_mac_facinfo.mac);
                conn.remote_port = MAC_FACTORY_WRITE_PORT;
                i_xtcp.send(conn, rx_buffer, response_len);
                break;
            case maccom_idcheck:
                response_len = macid_recheck_bulid(rx_buffer,t_mac_facinfo.mac,divce_id);
                i_xtcp.send(conn, rx_buffer, response_len);
                break;
            case maccom_signalwrite:      
                debug_printf("rec writ mac\n");
                mac_writeflash(&rx_buffer[6]);
                memcpy(t_mac_facinfo.mac,rx_buffer+6,6);
                response_len = macsignal_rewrite_bulid(rx_buffer,maccom_signalwrite_ok);
                i_xtcp.send(conn, rx_buffer, response_len);
                //
                break;
            case maccom_idwrite:
                mac_writeflash(&rx_buffer[8]);
                memcpy(t_mac_facinfo.mac,rx_buffer+6,6);
                response_len = macid_rewrite_bulid(rx_buffer,divce_id,maccom_signalwrite_ok);
                i_xtcp.send(conn, rx_buffer, response_len);
                //
                break;
            default:
                break;
        }
#if 0
        debug_printf("rec_data:\n");
#endif
        break;
    case XTCP_RESEND_DATA:  
        //debug_printf("resend_data:\n");
        //break;
    case XTCP_SENT_DATA:
        responding_connection.id = INIT_VAL;
        i_xtcp.close(conn); 
#if 0
        debug_printf("send_data:\n");
#endif
        break;
    case XTCP_TIMED_OUT:
    case XTCP_ABORTED:
    case XTCP_CLOSED:
#if 0
        debug_printf("Closed connection:%x\n",conn.id);
#endif
        break;
    }

    if(conn.event != XTCP_IFUP && conn.event != XTCP_IFDOWN)
    {
        conn.event = XTCP_DNS_RESULT+1;
    }
}



