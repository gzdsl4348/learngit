// Copyright (c) 2011-2016, XMOS Ltd, All rights reserved
#ifndef __xtcp_h__
#define __xtcp_h__

#include <mii.h>
#include <smi.h>
#include <ethernet.h>
#include <otp_board_info.h>
#include <xccompat.h>

#ifdef __xtcp_conf_h_exists__
#include "xtcp_conf.h"
#endif

#ifdef __xtcp_client_conf_h_exists__
#include "xtcp_client_conf.h"
#endif

#ifndef XTCP_CLIENT_BUF_SIZE
#define XTCP_CLIENT_BUF_SIZE (1472)
#endif
#ifndef XTCP_MAX_RECEIVE_SIZE
#ifdef UIP_CONF_RECEIVE_WINDOW
#define XTCP_MAX_RECEIVE_SIZE (UIP_CONF_RECEIVE_WINDOW)
#else
#define XTCP_MAX_RECEIVE_SIZE (1472)
#endif
#endif

/** Used by the LWIP and uIP callback functions to
 *  correctly pass packets to the DHCP functions
 */
#define DHCPC_SERVER_PORT 67
#define DHCPC_CLIENT_PORT 68

/** Maximum number of listening ports for XTCP */
#ifndef NUM_TCP_LISTENERS
#define NUM_TCP_LISTENERS 20
#endif
#ifndef NUM_UDP_LISTENERS
#define NUM_UDP_LISTENERS 20
#endif

/** Maximum number of connected XTCP clients */
#ifndef MAX_XTCP_CLIENTS
#define MAX_XTCP_CLIENTS 2
#endif

/** Maximum number of events in a client queue */
#ifndef CLIENT_QUEUE_SIZE
#define CLIENT_QUEUE_SIZE 70
#endif

/** Value used by lwIP's RX buffer */
#define MAX_PACKET_BYTES 1518

/** As UDP connections are stateless, we need to include extra
 *  information in the UDP PCB to determine when a new connection
 *  has arrived. LWIP ONLY.
 */
#define CONNECTIONS_PER_UDP_PORT 10

typedef unsigned int xtcp_appstate_t;

/** XTCP IP address.
 *
 *  This data type represents a single ipv4 address in the XTCP
 *  stack.
 */
typedef unsigned char xtcp_ipaddr_t[4];
typedef unsigned char xtcp_macaddr_t[6];
typedef struct xtcp_mac_t{
	unsigned char addr[6];
}xtcp_mac_t;

/** IP configuration information structure.
 *
 *  This structure describes IP configuration for an ip node.
 *
 **/
typedef struct xtcp_ipconfig_t {
  xtcp_ipaddr_t ipaddr;    /**< The IP Address of the node */
  xtcp_ipaddr_t netmask;   /**< The netmask of the node. The mask used
                                to determine which address are routed locally.*/
  xtcp_ipaddr_t gateway;   /**< The gateway of the node */
} xtcp_ipconfig_t;

/** XTCP protocol type.
 *
 * This determines what type a connection is: either UDP or TCP.
 *
 **/
typedef enum xtcp_protocol_t {
  XTCP_PROTOCOL_TCP = 1, /**< Transmission Control Protocol */
  XTCP_PROTOCOL_UDP  /**< User Datagram Protocol */
} xtcp_protocol_t;


/** XTCP event type.
 *
 *  The event type represents what event is occuring on a particular connection.
 *  It is instantiated as part of the xtcp_connection_t structure in the function
 *  get_packet().
 *
 **/
typedef enum xtcp_event_type_t {
  XTCP_NEW_CONNECTION,  /**<  This event represents a new connection has been
                              made. In the case of a TCP server connections it
                              occurs when a remote host firsts makes contact
                              with the local host. For TCP client connections
                              it occurs when a stream is setup with the remote
                              host.
                              For UDP connections it occurs as soon as the
                              connection is created. **/

  XTCP_RECV_DATA,       /**<  This event occurs when the connection has received
                              some data. The return_len in get_packet() will
                              indicate the length of the data. The data will be
                              present in the buffer passed to get_packet(). **/

  XTCP_SENT_DATA,       /**<  This event occurs when the server has successfully
                              sent the previous piece of data that was given
                              to it via a call to send(). **/

  XTCP_RESEND_DATA,     /**<  This event occurs when the server has failed to
                              send the previous piece of data that was given
                              to it via a call to send(). The server
                              is now requesting for the same data to be sent
                              again. **/

  XTCP_TIMED_OUT,      /**<   This event occurs when the connection has
                              timed out with the remote host (TCP only).
                              This event represents the closing of a connection
                              and is the last event that will occur on
                              an active connection. */

  XTCP_ABORTED,        /**<   This event occurs when the connection has
                              been aborted by the local or remote host
                              (TCP only).
                              This event represents the closing of a connection
                              and is the last event that will occur on
                              an active connection. */

  XTCP_CLOSED,         /**<   This event occurs when the connection has
                              been closed by the local or remote host.
                              This event represents the closing of a connection
                              and is the last event that will occur on
                              an active connection. */

  XTCP_IFUP,           /**<   This event occurs when the link goes up (with
                              valid new ip address). This event has no
                              associated connection. */

  XTCP_IFDOWN,         /**<   This event occurs when the link goes down.
                              This event has no associated connection. */

  XTCP_DNS_RESULT,     /**<   This event occurs when the XTCP connection has a DNS
                              result for a request. **/
  XTCP_ALREADY_HANDLED                              
} xtcp_event_type_t;

/** This type represents a TCP or UDP connection.
 *
 *  This is the main type containing connection information for the client
 *  to handle. Elements of this type are instantiated by the xtcp_event()
 *  function which informs the client about an event and the connection
 *  the event is on.
 *
 **/
typedef struct xtcp_connection_t {
  int client_num;             /**< The number of the client connected */
  int id;                     /**< A unique identifier for the connection */
  xtcp_protocol_t protocol;   /**< The protocol of the connection (TCP/UDP) */
  xtcp_event_type_t event;    /**< The last reported event on this connection. */
  xtcp_appstate_t appstate;   /**< The application state associated with the
                                   connection. This is set using the
                                   set_appstate() function. */
  xtcp_ipaddr_t remote_addr;  /**< The remote ip address of the connection. */
  unsigned int remote_port;   /**< The remote port of the connection. */
  unsigned int local_port;    /**< The local port of the connection. */
  unsigned int mss;           /**< The maximum size in bytes that can be send using
                                   xtcp_send() after a send event */
  unsigned packet_length;     /**< Length of packet recieved */
  int stack_conn;             /**< Pointer to the associated uIP/LWIP connection.
                                   Only to be used by XTCP. */
} xtcp_connection_t;

#ifdef __XC__
typedef interface xtcp_if {
  /** \brief Recieve information/data from the XTCP server.
   *
   *  After the client is notified by packet_ready() it must call this function
   *  to receive the packet from the server.
   *
   *  If the data buffer is not large enough then an exception will be raised.
   *
   * \param conn        The connection structure to be passed in that will
   *                    contain all the connection information.
   * \param data        An array where XTCP server can write data to. This data
   *                    array must be large enough to receive the packets being
   *                    sent to the client. In most cases it should be assumed
   *                    that packets of ETHERNET_MAX_PACKET_SIZE can be received.
   * \param n           Size of the data array.
   * \param length      An integer where the server can indicate
   *                    the length of the sent packet.
   */
  [[clears_notification]] void get_packet(xtcp_connection_t &conn, char data[n], unsigned n, unsigned &length);

  /** \brief Notifies the client that there is data/information
   *         ready for them.
   *
   *  After this notification is raised a call to get_packet() is needed.
   */
  [[notification]] slave void packet_ready();

  /** \brief Listen to a particular incoming port.
   *
   *  After this call, when a connection is established an
   *  XTCP_NEW_CONNECTION event is signalled.
   *
   * \param port_number The local port number to listen to
   * \param protocol    The protocol to connect with (XTCP_PROTOCOL_TCP
   *                    or XTCP_PROTOCOL_UDP)
   */
  void listen(int port_number, xtcp_protocol_t protocol);

  /** \brief Stop listening to a particular incoming port.
   *
   * \param port_number local port number to stop listening on
   */
  void unlisten(unsigned port_number);

  /** \brief Close a connection.
   *
   *  May still recieve data on a TCP connection. Use abort() if
   *  you wish to completely stop all data. Will continue to listen
   *  on the open port the connection came from.
   *
   * \param conn        The connection structure to be passed in that will
   *                    contain all the connection information.
   */
  void close(const xtcp_connection_t &conn);
  /**
   * new_conn: 返回xtcp_connection_t数据结构
   * 
   * 返回值: 0 - 关闭成功, conn类型不为udp
   *         1 - 关闭失败, 协议栈conn已满
   */  
  int close_udp(const xtcp_connection_t &conn);
  
  /** \brief Abort a connection.
   *
   *  For UDP this is the same as closing the connection. For TCP
   *  the server will send a RST signal and stop all incoming data.
   *
   * \param conn        The connection structure to be passed in that will
   *                    contain all the connection information.
   */
  void abort(const xtcp_connection_t &conn);

  /** \brief Try to connect to a remote port.
   *
   *  For TCP this will initiate the three way handshake.
   *  For UDP this will assign a random local port and bind the remote
   *  end of the connection to the host specified.
   *
   * \param port_number The remote port to try to connect to
   * \param ipaddr      The ip addr of the remote host
   * \param protocol    The protocol to connect with (XTCP_PROTOCOL_TCP
   *                    or XTCP_PROTOCOL_UDP)
   */
  int connect(unsigned port_number, xtcp_ipaddr_t ipaddr, xtcp_protocol_t protocol);
  /**
   * new_conn: 返回xtcp_connection_t数据结构
   * 
   * 返回值: 0 - 发送成功
   *         1 - 连接失败, 协议栈conn已满
   */
  int connect_udp(unsigned port_number, xtcp_ipaddr_t ipaddr, xtcp_connection_t &new_conn);
  
  /** \brief Send data to the connection.
   *
   * \param conn        The connection structure to be passed in that will
   *                    contain all the connection information.
   * \param data        An array of data to send
   * \param len         The length of data to send. If this is 0, no data will
   *                    be sent and a XTCP_SENT_DATA event will not occur.
   */
  void send(const xtcp_connection_t &conn, char data[], unsigned len);
  /**
   * 返回值: 0 - 发送成功
   *         1 - 发送失败, conn类型不为udp
   *         2 - 发送失败, arp表未能找到对应mac, 等待产生resend事件
   */
  int send_udp(const xtcp_connection_t &conn, char data[], unsigned len);
  
  /** \brief Subscribe to a particular IP multicast group address.
   *
   * \param addr        The address of the multicast group to join. It is
   *                    assumed that this is a multicast IP address.
   */
  void join_multicast_group(xtcp_ipaddr_t addr);

  /** \brief Unsubscribe from a particular IP multicast group address.
   *
   * \param addr        The address of the multicast group to leave. It is
   *                    assumed that this is a multicast IP address.
   */
  void leave_multicast_group(xtcp_ipaddr_t addr);

  /** \brief Set the connections application state data item
   *
   * After this call, subsequent events on this connection
   * will have the appstate field of the connection set.
   *
   * \param conn        The connection structure to be passed in that will
   *                    contain all the connection information.
   * \param appstate    An unsigned integer representing the state. In C
   *                    this is usually a pointer to some connection dependent
   *                    information.
   */
  void set_appstate(const xtcp_connection_t &conn, xtcp_appstate_t appstate);

  /** \brief Bind the local end of a connection to a particular port (UDP).
   *
   * \param conn        The connection structure to be passed in that will
   *                    contain all the connection information.
   * \param port_number The local port to set the connection to.
   */
  void bind_local_udp(const xtcp_connection_t &conn, unsigned port_number);

  /** \brief Bind the remote end of a connection to a particular port and
   *         ip address (UDP).
   *
   * After this call, packets sent to this connection will go to
   * the specified address and port
   *
   * \param conn        The connection structure to be passed in that will
   *                    contain all the connection information.
   * \param ipaddr      The intended remote address of the connection
   * \param port_number The intended remote port of the connection
   */
  void bind_remote_udp(const xtcp_connection_t &conn, xtcp_ipaddr_t ipaddr, unsigned port_number);

  /** \brief Request a hosts IP address from a URL.
   *
   * \param hostname    The human readable host name, e.g. "www.xmos.com"
   * \param name_len    The length of the hostname in characters
   * \note              LWIP ONLY.
   */
  void request_host_by_name(const char hostname[], unsigned name_len);

  /** \brief Fill the provided ipconfig address with the current state of the server.
   *
   * \param ipconfig    IPconfig to be filled.
   */
  void get_ipconfig(xtcp_ipconfig_t &ipconfig);

  void xtcp_init(xtcp_ipconfig_t &ipconfig_flash,uint8_t macaddr_flash[6]);

  void xtcp_ipconfig(xtcp_ipconfig_t &ipconfig_new);

  void xtcp_arpget(xtcp_ipaddr_t ipaddr,xtcp_mac_t &t_xtcp_mac);
} xtcp_if;

typedef struct pbuf * unsafe pbuf_p;

/** WiFi/xtcp data interface - mii.h equivalent
 *  TODO: document
 */
typedef interface xtcp_pbuf_if {

  /** TODO: document */
  [[clears_notification]]
  pbuf_p receive_packet();

  [[notification]]
  slave void packet_ready();

  /** TODO: document */
  void send_packet(pbuf_p p);

  // TODO: Add function to notify clients of received packets
} xtcp_pbuf_if;

typedef enum {
  ARP_TIMEOUT = 0,
  AUTOIP_TIMEOUT,
  TCP_TIMEOUT,
  IGMP_TIMEOUT,
  DHCP_COARSE_TIMEOUT,
  DHCP_FINE_TIMEOUT,
  NUM_TIMEOUTS
} xtcp_lwip_timeout_type;

/** Function implementing the TCP/IP stack using the lwIP stack.
 *
 *  This functions implements a TCP/IP stack that clients can access via
 *  interfaces.
 *
 *  \param i_xtcp       The interface array to connect to the clients.
 *  \param n_xtcp       The number of clients to the task.
 *  \param i_mii        If this component is connected to the mii() component
 *                      in the Ethernet library then this interface should be
 *                      used to connect to it. Otherwise it should be set to
 *                      null
 *  \param i_eth_cfg    If this component is connected to an MAC component
 *                      in the Ethernet library then this interface should be
 *                      used to connect to it. Otherwise it should be set to
 *                      null.
 *  \param i_eth_rx     If this component is connected to an MAC component
 *                      in the Ethernet library then this interface should be
 *                      used to connect to it. Otherwise it should be set to
 *                      null.
 *  \param i_eth_tx     If this component is connected to an MAC component
 *                      in the Ethernet library then this interface should be
 *                      used to connect to it. Otherwise it should be set to
 *                      null.
 *  \param i_smi        If this connection to an Ethernet SMI component is
 *                      then the XTCP component will poll the Ethernet PHY
 *                      for link up/link down events. Otherwise, it will
 *                      expect link up/link down events from the connected
 *                      Ethernet MAC.
 *  \param phy_address  The SMI address of the Ethernet PHY
 *  \param mac_address  If this array is non-null then it will be used to set
 *                      the MAC address of the component.
 *  \param otp_ports    If this port structure is non-null then the component
 *                      will obtain the MAC address from OTP ROM. See the OTP
 *                      reading library user guide for details.
 *  \param ipconfig     This :c:type:`xtcp_ipconfig_t` structure is used
 *                      to determine the IP address configuration of the
 *                      component.
 */
void xtcp_lwip(server xtcp_if i_xtcp[n_xtcp],
               static const unsigned n_xtcp,
               client mii_if ?i_mii,
               client ethernet_cfg_if ?i_eth_cfg,
               client ethernet_rx_if ?i_eth_rx,
               client ethernet_tx_if ?i_eth_tx,
               client smi_if ?i_smi,
               uint8_t phy_address,
               const char (&?mac_address0)[6],
               otp_ports_t &?otp_ports,
               xtcp_ipconfig_t &ipconfig);

/** Function implementing the TCP/IP stack task using the uIP stack.
 *
 *  This functions implements a TCP/IP stack that clients can access via
 *  interfaces.
 *
 *  \param i_xtcp       The interface array to connect to the clients.
 *  \param n_xtcp       The number of clients to the task.
 *  \param i_mii        If this component is connected to the mii() component
 *                      in the Ethernet library then this interface should be
 *                      used to connect to it. Otherwise it should be set to
 *                      null
 *  \param i_eth_cfg    If this component is connected to an MAC component
 *                      in the Ethernet library then this interface should be
 *                      used to connect to it. Otherwise it should be set to
 *                      null.
 *  \param i_eth_rx     If this component is connected to an MAC component
 *                      in the Ethernet library then this interface should be
 *                      used to connect to it. Otherwise it should be set to
 *                      null.
 *  \param i_eth_tx     If this component is connected to an MAC component
 *                      in the Ethernet library then this interface should be
 *                      used to connect to it. Otherwise it should be set to
 *                      null.
 *  \param i_smi        If this connection to an Ethernet SMI component is
 *                      then the XTCP component will poll the Ethernet PHY
 *                      for link up/link down events. Otherwise, it will
 *                      expect link up/link down events from the connected
 *                      Ethernet MAC.
 *  \param phy_address  The SMI address of the Ethernet PHY
 *  \param mac_address  If this array is non-null then it will be used to set
 *                      the MAC address of the component.
 *  \param otp_ports    If this port structure is non-null then the component
 *                      will obtain the MAC address from OTP ROM. See the OTP
 *                      reading library user guide for details.
 *  \param ipconfig     This :c:type:`xtcp_ipconfig_t` structure is used
 *                      to determine the IP address configuration of the
 *                      component.
 */
void xtcp_uip(server xtcp_if i_xtcp[n_xtcp],
              static const unsigned n_xtcp,
              client mii_if ?i_mii,
              client ethernet_cfg_if ?i_eth_cfg,
              client ethernet_rx_if ?i_eth_rx,
              client ethernet_tx_if ?i_eth_tx,
              client smi_if ?i_smi,
              uint8_t phy_address);
#endif /* __XC__ */

/** Copy an IP address data structure.
 */
#define XTCP_IPADDR_CPY(dest, src) do { dest[0] = src[0]; \
                                        dest[1] = src[1]; \
                                        dest[2] = src[2]; \
                                        dest[3] = src[3]; \
                                      } while (0)

/** Compare two IP address structures.
 */
#define XTCP_IPADDR_CMP(a, b) (a[0] == b[0] && \
                                a[1] == b[1] && \
                                a[2] == b[2] && \
                                a[3] == b[3])

#endif // __xtcp_h__