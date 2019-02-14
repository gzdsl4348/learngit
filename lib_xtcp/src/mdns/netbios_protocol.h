// Copyright (c) 2011-2016, XMOS Ltd, All rights reserved


#ifndef _netbios_protocol_h_
#define _netbios_protocol_h_

/** default port number for "NetBIOS Name service */
#define NETBIOS_PORT     137

/** size of a NetBIOS name */
#define NETBIOS_NAME_LEN 16

/** The Time-To-Live for NetBIOS name responds (in seconds)
 * Default is 300000 seconds (3 days, 11 hours, 20 minutes) */
#define NETBIOS_NAME_TTL 300000

/** NetBIOS header flags */
#define NETB_HFLAG_RESPONSE           0x8000
#define NETB_HFLAG_OPCODE             0x7800
#define NETB_HFLAG_OPCODE_NAME_QUERY  0x0000
#define NETB_HFLAG_AUTHORATIVE        0x0400
#define NETB_HFLAG_TRUNCATED          0x0200
#define NETB_HFLAG_RECURS_DESIRED     0x0100
#define NETB_HFLAG_RECURS_AVAILABLE   0x0080
#define NETB_HFLAG_BROADCAST          0x0010
#define NETB_HFLAG_REPLYCODE          0x0008
#define NETB_HFLAG_REPLYCODE_NOERROR  0x0000

/** NetBIOS name flags */
#define NETB_NFLAG_UNIQUE             0x8000
#define NETB_NFLAG_NODETYPE           0x6000
#define NETB_NFLAG_NODETYPE_HNODE     0x6000
#define NETB_NFLAG_NODETYPE_MNODE     0x4000
#define NETB_NFLAG_NODETYPE_PNODE     0x2000
#define NETB_NFLAG_NODETYPE_BNODE     0x0000

/** NetBIOS message header */
struct netbios_hdr {
  u16_t trans_id;
  u16_t flags;
  u16_t questions;
  u16_t answerRRs;
  u16_t authorityRRs;
  u16_t additionalRRs;
};

#define ENCODED_NETBIOS_NAME_LEN (NETBIOS_NAME_LEN*2)+1

/** NetBIOS message name part */
struct netbios_name_hdr {
  u8_t  nametype;
  u8_t  encname[ENCODED_NETBIOS_NAME_LEN];
  u16_t type;
  u16_t class;
  u32_t ttl;
  u16_t datalen;
  u16_t flags;
  u8_t addr[4];
}  __attribute__((packed));


/** NetBIOS message */
struct netbios_resp
{
  struct netbios_hdr      resp_hdr;
  struct netbios_name_hdr resp_name;
} __attribute__((packed));

#endif // _netbios_protocol_h_
