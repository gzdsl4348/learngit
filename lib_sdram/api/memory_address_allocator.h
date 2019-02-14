// Copyright (c) 2014-2016, XMOS Ltd, All rights reserved
#ifndef MEMORY_ALLOCATOR_H_
#define MEMORY_ALLOCATOR_H_

typedef enum {
    e_success,
    e_overflow,
    e_out_of_memory_range
} e_memory_address_allocator_return_code;

/** This interface is used to communication with a memory address allocator.
 *  It provides facilities for requesting an address for a region of memory from within a shared memory.
 *
 **/
interface memory_address_allocator_i {
  /** Request an amount of memory from the common memory space. 
   *
   *  \param bytes     The address of the slave device to write to.
   *  \param address   A return value for the base address of the memory requested.
   *
   *  \returns         Whether the allocation succeeded.
   **/
    e_memory_address_allocator_return_code request(unsigned bytes, unsigned &address);
} [[sametile]];

/**
 * The distributable server for providing memory address to multiple clients.
 *
 *  \param client_count         The number of clients.
 *  \param rx                   Array of the clients wanting to request memory address space.
 *  \param base_address         Value to be used as the base of memory address.
 *  \param memory_size          The size of the memory.
 **/
[[distributable]]
void memory_address_allocator(
        unsigned client_count,
        server interface memory_address_allocator_i rx[client_count],
        unsigned base_address,
        unsigned memory_size);

#endif /* MEMORY_ALLOCATOR_H_ */
