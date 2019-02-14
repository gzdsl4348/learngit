#include "gobal_val.h"
#include <stdio.h>

g_val_t	g_t_valu={0};

uint32_t get_gval_point(){
	return (uint32_t)&g_t_valu;
}



