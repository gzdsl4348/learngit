// Copyright (c) 2016-2017, XMOS Ltd, All rights reserved
#include "random.h"
#include <xs1.h>

static const unsigned random_poly = 0xEDB88320;

unsigned random_get_random_number(random_generator_t &g)
{
  crc32(g, -1, random_poly);
  return (unsigned) g;
}

void random_get_random_bytes(random_generator_t &g, uint8_t in_buffer[], size_t byte_count)
{
  for (int i=0; i < byte_count; i++)
  {
    in_buffer[i] = (uint8_t)random_get_random_number(g);
  }
}

random_generator_t random_create_generator_from_seed(unsigned seed)
{
  random_generator_t gen = (random_generator_t) seed;
  (void) random_get_random_number(gen);
  return gen;
}

static const unsigned XS1_L_RING_OSCILLATOR_VALUE_REG  = 0x070B;

random_generator_t random_create_generator_from_hw_seed(void)
{
  unsigned init_seed = getps(XS1_L_RING_OSCILLATOR_VALUE_REG);
  return random_create_generator_from_seed(init_seed);
}

