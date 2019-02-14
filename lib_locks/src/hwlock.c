// Copyright (c) 2014-2016, XMOS Ltd, All rights reserved
#include "hwlock.h"

extern inline hwlock_t hwlock_alloc(void);
extern inline void hwlock_free(hwlock_t lock);
extern inline void hwlock_acquire(hwlock_t lock);
extern inline void hwlock_release(hwlock_t lock);
