// Copyright (c) 2016-2017, XMOS Ltd, All rights reserved
#include <xs1.h>
#include "random.h"

static const unsigned random_poly = 0xEDB88320;

unsigned random_get_random_number(random_generator_t &g)
{
  crc32(g, -1, random_poly);
  return (unsigned) g;
}
