// Copyright (c) 2017, XMOS Ltd, All rights reserved
#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <xccompat.h>

/** Type representing a random number generator.
 */
typedef unsigned random_generator_t;

/** Function that produces a random number. The number has a cycle of 2^32
 *  and is produced using a LFSR.
 *
 *  \param g    the used generator to produce the seed.
 *
 *  \returns    a random 32 bit number.
 */
unsigned random_get_random_number(REFERENCE_PARAM(random_generator_t, g));

#endif /* _RANDOM_H_ */
