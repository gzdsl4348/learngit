// Copyright (c) 2014-2016, XMOS Ltd, All rights reserved
#ifndef SDRAM_CONTROL_H_
#define SDRAM_CONTROL_H_

/*                                 BURST                   LOAD
 *  signal  NOP  ACTIVE WRITE READ TERM  REFRESH PRECHARGE MODEREG
 *  RAS     1    0      1     1    1     0       0         0
 *  CAS     1    1      0     0    1     0       1         0
 *  WE      1    1      0     1    0     1       0         0
*/

#define CTRL_RAS_PIN          0x1
#define CTRL_CAS_PIN          0x2
#define CTRL_WE_PIN           0x0

#define CTRL_EXTRA_VAL        0x1

#define CTRL_RAS_NOP          0x1
#define CTRL_CAS_NOP          0x1
#define CTRL_WE_NOP           0x1

#define CTRL_RAS_ACTIVE       0x0
#define CTRL_CAS_ACTIVE       0x1
#define CTRL_WE_ACTIVE        0x1

#define CTRL_RAS_WRITE        0x1
#define CTRL_CAS_WRITE        0x0
#define CTRL_WE_WRITE         0x0

#define CTRL_RAS_READ         0x1
#define CTRL_CAS_READ         0x0
#define CTRL_WE_READ          0x1

#define CTRL_RAS_REFRESH      0x0
#define CTRL_CAS_REFRESH      0x0
#define CTRL_WE_REFRESH       0x1

#define CTRL_RAS_PRECHARGE    0x0
#define CTRL_CAS_PRECHARGE    0x1
#define CTRL_WE_PRECHARGE     0x0

#define CTRL_RAS_TERM         0x1
#define CTRL_CAS_TERM         0x1
#define CTRL_WE_TERM          0x0

#define CTRL_RAS_LOAD_MODEREG 0x0
#define CTRL_CAS_LOAD_MODEREG 0x0
#define CTRL_WE_LOAD_MODEREG  0x0

#endif /* SDRAM_CONTROL_H_ */
