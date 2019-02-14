// Copyright (c) 2014-2016, XMOS Ltd, All rights reserved
#include <xs1.h>
//#include <xassert.h>

#include "memory_address_allocator.h"

[[distributable]]
void memory_address_allocator(
        unsigned client_count,
        server interface memory_address_allocator_i rx[client_count],
        unsigned base_address,
        unsigned memory_size){

    unsigned bottom = base_address;

    //xassert((((unsigned long long )base_address+
    //        (unsigned long long )memory_size)>>32) == 0);

    unsigned top = base_address + memory_size;

    while(1){
        select {
            case rx[int i].request(unsigned bytes, unsigned &address) ->
                    e_memory_address_allocator_return_code return_val: {

                unsigned long long b = (unsigned long long)bottom;

                if((b + (unsigned long long)bytes) >> 32){
                    return_val = e_overflow;
                    break;
                }

                if((bottom + bytes) > top){
                    return_val = e_out_of_memory_range;
                    break;
                }
                address = bottom;
                return_val = e_success;
                bottom += bytes;
                break;

            }
        }
    }

}
