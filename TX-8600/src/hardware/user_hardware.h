#ifndef  __USER_HARDWARE_H
#define  __USER_HARDWARE_H

#ifdef __GLOBAL_HARDWARE_
//----------------------------------------------------------
// Hardware IO Define
//----------------------------------------------------------
//Power Led
on tile[0]: out port p_powerled = XS1_PORT_4C;

//----------------------------------------------------------
//Extern IO define
#else	
// extern user powerled
extern	out port p_powerled; 

//----------------------------------------------------------
#endif	//__GLOBAL_HARDWARE_

#endif	//__USER_HARDWARE_H

