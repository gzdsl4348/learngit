#ifndef  __MACBUILDPACK
#define  __MACBUILDPACK

#include <platform.h>
#include <xs1.h>
#include <string.h>
#include <stdint.h>

//===============================================================================
//MAC Config Commend Define
//===============================================================================
//Recive
#define maccom_signalcheck       0x0A
#define maccom_signalwrite       0x0C
#define maccom_idcheck           0x0D
#define maccom_idwrite           0x0F
//Resend
#define maccom_signalrecheck     0x0B
#define maccom_idrecheck         0x0E

#define maccom_signalwrite_ok     0xA5
#define maccom_signalwrite_fail   0xAE

#define maccom_idwrite_ok         0xA5
#define maccom_idwrite_fail       0xAE

//==================================================================================

uint8_t macsingle_recheck_bulid(uint8_t mactxbuff[50],uint8_t macadress[6]);
uint8_t macid_recheck_bulid(uint8_t mactxbuff[50],uint8_t macadress[6],uint16_t id);
uint8_t macsignal_rewrite_bulid(uint8_t mactxbuff[50],uint8_t state);
uint8_t macid_rewrite_bulid(uint8_t mactxbuff[50],uint16_t id,uint8_t State);

#endif











