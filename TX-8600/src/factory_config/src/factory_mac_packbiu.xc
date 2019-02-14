#include "factory_mac_packbiu.h"

//==============================================================================
//MAC Config SingleCheck
//==============================================================================
uint8_t macsingle_recheck_bulid(uint8_t mactxbuff[50],uint8_t macadress[6])
{
  uint8_t len=0;
  uint8_t Sum=0;
  //----------------------------------
  //Star Byte
  mactxbuff[len]=0xAA;
  len++;
  //-----------------------------------
  //Len
  mactxbuff[len]=0x00;
  mactxbuff[len+1]=0x0D;
  len+=2;
  //------------------------------------
  //Reserved
  mactxbuff[len]=0x00;
  mactxbuff[len+1]=0x00;
  len+=2;
  //------------------------------------
  //Commend
  mactxbuff[len]=maccom_signalrecheck;    //Signed
  len++;
  //-------------------------------------
  memcpy(mactxbuff+len,macadress,6);
  len+=6;
  //-------------------------------------
  //Sum
  mactxbuff[len]=0x00;
  len++;
  //---------------------------------------
  //End
  mactxbuff[len]=0xAD;
  len++;
  //---------------------------------------
  //CheckSum
  for(uint8_t i=1; i<len-2; i++)
      Sum+=mactxbuff[i+1];
  mactxbuff[len-2]=Sum;
  return len;
}

//=============================================================================
//MAC Config ID Mac Recheck
//=============================================================================
uint8_t macid_recheck_bulid(uint8_t mactxbuff[50],uint8_t macadress[6],uint16_t id)
{
  uint8_t len=0;
  uint8_t Sum=0;
  //----------------------------------
  //Star Byte
  mactxbuff[len]=0xAA;
  len++;
  //-----------------------------------
  //Len
  mactxbuff[len]=0x00;
  mactxbuff[len+1]=0x0F;
  len+=2;
  //------------------------------------
  //Reserved
  mactxbuff[len]=0x00;
  mactxbuff[len+1]=0x00;
  len+=2;
  //------------------------------------
  //Commend
  mactxbuff[len]=maccom_idrecheck;    //Signed
  len++;
  //-------------------------------------
  // Divice ID
  mactxbuff[len]= id>>8;
  mactxbuff[len+1]= id&0x00FF;
  len+=2;
  //-------------------------------------
  memcpy(mactxbuff+len,macadress,6);
  len+=6;
  //-------------------------------------
  //Sum
  mactxbuff[len]=0x00;
  len++;
  //---------------------------------------
  //End
  mactxbuff[len]=0xAD;
  len++;
  //---------------------------------------
  //CheckSum
  for(uint8_t i=1; i<len-2; i++)
      Sum+=mactxbuff[i+1];
  mactxbuff[len-2]=Sum;
  return len;
}

//===============================================================================
//MAC Config Udp Signal ReWrite
//===============================================================================
uint8_t macsignal_rewrite_bulid(uint8_t mactxbuff[50],uint8_t state)
{
  uint8_t len=0;
  uint8_t Sum=0;
  //----------------------------------
  //Star Byte
  mactxbuff[len]=0xAA;
  len++;
  //-----------------------------------
  //Len
  mactxbuff[len]=0x00;
  mactxbuff[len+1]=0x07;
  len+=2;
  //------------------------------------
  //Reserved
  mactxbuff[len]=0x00;
  mactxbuff[len+1]=0x00;
  len+=2;
  //------------------------------------
  //Commend
  mactxbuff[len]=state;    //Signed
  len++;
  //-------------------------------------
  //Sum
  mactxbuff[len]=0x00;
  len++;
  //---------------------------------------
  //End
  mactxbuff[len]=0xAD;
  len++;
  //---------------------------------------
  //CheckSum
  for(uint8_t i=1; i<len-2; i++)
      Sum+=mactxbuff[i+1];
  mactxbuff[len-2]=Sum;
  return len;
}

//==================================================================================
//MAC Config ID MAC ReWtirte
//==================================================================================
uint8_t macid_rewrite_bulid(uint8_t mactxbuff[50],uint16_t id,uint8_t state)
{
  uint8_t len=0;
  uint8_t Sum=0;
  //----------------------------------
  //Star Byte
  mactxbuff[len]=0xAA;
  len++;
  //-----------------------------------
  //Len
  mactxbuff[len]=0x00;
  mactxbuff[len+1]=0x09;
  len+=2;
  //------------------------------------
  //Reserved
  mactxbuff[len]=0x00;
  mactxbuff[len+1]=0x00;
  len+=2;
  //------------------------------------
  //Commend
  mactxbuff[len]=state;    //Signed
  len++;
  //-------------------------------------
  // Divice ID
  mactxbuff[len]= id>>8;
  mactxbuff[len+1]= id&0x00FF;
  len+=2;
  //-------------------------------------
  //Sum
  mactxbuff[len]=0x00;
  len++;
  //---------------------------------------
  //End
  mactxbuff[len]=0xAD;
  len++;
  //---------------------------------------
  //CheckSum
  for(uint8_t i=1; i<len-2; i++)
      Sum+=mactxbuff[i+1];
  mactxbuff[len-2]=Sum;
  return len;
}


