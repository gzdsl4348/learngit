// Copyright (c) 2016-2017, XMOS Ltd, All rights reserved
#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <stdint.h>
#include <stddef.h>

#ifndef REFERENCE_PARAM
#ifdef __XC__
#define REFERENCE_PARAM(type, name) type &name
#else
#define REFERENCE_PARAM(type, name) type *name
#endif
#endif

/** Type representing a random number generator.
 */
typedef unsigned random_generator_t;

/** Function that creates a random number generator from a seed.
 *
 * \param seed  seed for the generator.
 *
 * \returns     a random number generator.
 */
random_generator_t random_create_generator_from_seed(unsigned seed);


/** Function that attempts to create a random number generator from
 *  a true random value into the seed, using
 *  an asynchronous timer. To use this function you must enable the
 *  ``RANDOM_ENABLE_HW_SEED`` define in your application's ``random_conf.h``.
 *
 *  \returns a random number generator.
 */
random_generator_t random_create_generator_from_hw_seed(void);

/** Function that produces a random number. The number has a cycle of 2^32
 *  and is produced using a LFSR.
 *
 *  \param g    the used generator to produce the seed.
 *
 *  \returns    a random 32 bit number.
 */
unsigned
random_get_random_number(REFERENCE_PARAM(random_generator_t, g));

void random_get_random_bytes(REFERENCE_PARAM(random_generator_t, g), uint8_t in_buffer[], size_t byte_count);

#endif // __RANDOM_H__
