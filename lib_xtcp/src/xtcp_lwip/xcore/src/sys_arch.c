// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include "lwip/sys.h"
#include "lwip/opt.h"
#include "arch/sys_arch.h"
#include "hwlock.h"

#if SYS_LIGHTWEIGHT_PROT
hwlock_t lwip_lock;

/** sys_prot_t sys_arch_protect(void)

This optional function does a "fast" critical region protection and returns
the previous protection level. This function is only called during very short
critical regions. An embedded system which supports ISR-based drivers might
want to implement this function by disabling interrupts. Task-based systems
might want to implement this by using a mutex or disabling tasking. This
function should support recursive calls from the same task or interrupt. In
other words, sys_arch_protect() could be called while already protected. In
that case the return value indicates that it is already protected.

sys_arch_protect() is only required if your port is supporting an operating
system.
*/
sys_prot_t
sys_arch_protect(void)
{
    hwlock_acquire(lwip_lock);
    return 0;
}
/*-----------------------------------------------------------------------------------*/
/** void sys_arch_unprotect(sys_prot_t pval)

This optional function does a "fast" set of critical region protection to the
value specified by pval. See the documentation for sys_arch_protect() for
more information. This function is only required if your port is supporting
an operating system.
*/
void
sys_arch_unprotect(sys_prot_t pval)
{
    LWIP_UNUSED_ARG(pval);
    hwlock_release(lwip_lock);
}
#endif /* SYS_LIGHTWEIGHT_PROT */