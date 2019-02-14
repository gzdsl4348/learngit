.. include:: ../../../README.rst

Hardware characteristics
------------------------

The signals from the xCORE required to drive an SDRAM are:

.. _sdram_wire_table:

.. list-table:: SDRAM data and signal wires
     :class: vertical-borders horizontal-borders

     * - *Clock*
       - Clock line, the master clock the SDRAM uses for 
         sampling all the other signals.
     * - *DQ_AH*
       - The 16-bit data bus and address bus multiplexed,
         see below. 
     * - *WE*
       - Write enable(Inverted).
     * - *RAS*
       - The row address strobe(Inverted).
     * - *CAS*
       - The coloumn address strobe(Inverted).

Because of the multiplexing attention must paid to the
physical wiring of the SDRAM to the xCORE. 

A typical 256Mb SDRAM requires the following signals:

* CLK		- Clock
* CKE		- Clock Enable
* CS		     - Chip Select
* RAS		- Row Address Strobe
* CAS		- Col Address Strobe
* WE		     - Write Enable
* DQ[15:0]	- Data
* DQM		- Data Input/Output Mask
* A[12:0]		- Address
* BA[1:0]		- Bank Address

.. figure:: images/sdram_hookup.pdf
   :width: 80%

   Example connection between xCORE to 256Mb SDRAM


The exact count of Address lines and Bank Address line may vary. The examples in this document assume a 256Mb SDRAM device.
This library is designed to work with a fixed 16 bit SDRAM data bus, although the API provides data in long words (32 bit).

The dq_ah bus is made up of 16 lines. The DQ bus is mapped directly to
dq_ah. The address bus is mapped in order to the lower bits of dq_ah. Finally, 
the bank address bus is mapped to the higher bits of dq_ah.

Where the Address bus is 13 bits wide and the bank address is 2 bits wide
the following setup is in place::

  dq_ah[15:0] = DQ[15:0]
  dq_ah[12:0] = A[12:0]
  dq_ah[14:13] = BA[1:0]

The number of address bits plus the number of bank address bits must not exceed 16.

The DQM signal(s) is connected to the NOR of WE and CAS. An example of a suitable
part is the TI SN74LVC1G02. In the case that the DQM is separated into high 
and low components then the output from the NOR is connected to both high and low DQM.

This library assumes that CS is pulled low, i.e. the SDRAM is always selected. If 
control of the CS is needed then it must be done from the client application 
level. This means that for the duration of the use of the SDRAM, CS must 
be asserted and when ``sdram_server`` is shutdown the CS can be deasserted.


SDRAM API
---------

All SDRAM functions can be accessed via the ``sdram.h`` header::

  #include <sdram.h>

You also have to add ``lib_sdram`` to the
``USED_MODULES`` field of your application Makefile.

SDRAM server and client are instantiated as concurrent tasks that run in a
``par`` statement. The client (your application) connects to the SDRAM server via 
a streaming channel.

.. caution::
  The SDRAM server must be provided with enough MIPS to keep up with the specified SDRAM clock rate.
  For example, for 62.5MHz operation, the server core must always see a minimum of 62.5MHz. Typically
  this will be satisfied if the core clock is 500MHz and 8 cores are used.

The clock rate of the SDRAM server is controlled by the last parameter which is the clock divider. 
The resulting SDRAM clock, assuming a 500MHz core clock, will be defined as::

  500 / (2 * n)

Typical supported clock rates are as listed in the below table. For each rate, the appropriate port
delays are set to maximize the read window. See the code in ``server.xc`` for further details.

.. list-table:: Supported SDRAM server clock rates
    :class: vertical-borders horizontal-borders
    :header-rows: 1

    * - xCORE clock Divider setting
      - SDRAM server clock (MHz)
    * - 4
      - 62.5
    * - 5
      - 50
    * - 6
      - 41.66
    * - 7
      - 35.71
    * - 8
      - 31.25
    * - 9
      - 27.78
    * - 10
      - 25


For example, the following code instantiates an SDRAM server running at 62.5MHz
and connects an application to it::

  out buffered port:32   sdram_dq_ah                 = XS1_PORT_16A;
  out buffered port:32   sdram_cas                   = XS1_PORT_1B;
  out buffered port:32   sdram_ras                   = XS1_PORT_1G;
  out buffered port:8    sdram_we                    = XS1_PORT_1C;
  out port               sdram_clk                   = XS1_PORT_1F;
  clock                  sdram_cb                    = XS1_CLKBLK_1;
   
  int main() {
    streaming chan c_sdram[1];
    par {
        //256Mb SDRAM
        sdram_server(c_sdram, 1,
              sdram_dq_ah,
              sdram_cas,
              sdram_ras,
              sdram_we,
              sdram_clk,
              sdram_cb,
              2,    //CAS latency
              256,  //Row long words
              16,   //Col bits (argument unused by server)
              9,    //Col addr bits
              13,   //Row bits
              2,    //Bank bits
              64,   //Milliseconds refresh
              8192, //Refresh cycles
              4);   //Clock divider
      application(c_sdram[0]);
    }
    return 0;
  }

**Note**: The client and SDRAM server must be on the same tile as the 
line buffers are transferred by moving pointers from one task to another.

The SDRAM library uses movable pointers to pass buffers between the client 
and the server. This means that if the client passes a buffer in on-chip RAM to the 
SDRAM server, the client cannot access that buffer while the server is 
processing the command. To handle this the client sends 
commands using ``sdram_read`` and ``sdram_write``, both of which take 
a movable pointer as an argument. To return the pointer to the client the 
client must call ``sdram_complete`` which will take back ownership of the 
pointer when the SDRAM server has finished processing the command.

``sdram_complete`` can  be selected to allow the client to event on data 
becoming ready or completing a write.

Client/Server model
...................

The SDRAM server must be instantiated at the same level as its clients. For example::

  streaming chan c_sdram[1];
  par {
          sdram_server(c_sdram, 1,  ... );
          client_of_the_sdram_server(c_sdram[0]);
  }

would be the minimum required to correctly setup the SDRAM server and connect it to a 
client. An example of a multi-client system would be::

  streaming chan c_sdram[4];
  par {
  	sdram_server(c_sdram, 4,  ... );
  	client_of_the_sdram_server_0(c_sdram[0]);
  	client_of_the_sdram_server_1(c_sdram[1]);
  	client_of_the_sdram_server_2(c_sdram[2]);
  	client_of_the_sdram_server_3(c_sdram[3]);
  } 

Command buffering
.................

The SDRAM server implements an 8 slot command buffer per client. This means that the 
client can queue up to 8 commands to the SDRAM server through calls to ``sdram_read`` 
or ``sdram_write``. A successful call to ``sdram_read`` or ``sdram_write`` will return 0 
and issue the command to the command buffer. When the command buffer is full, a call to 
``sdram_read`` or ``sdram_write`` will return 1 and not issue the command.  
Commands are completed (i.e. a slot is freed) when ``sdram_complete`` returns. 
Commands are processed as in a first in first out ordering.


Initialization
..............

Each client of the SDRAM server must declare the structure ``s_sdram_state`` only once 
and call ``sdram_init_state``. This does all the required setup for the command buffering. 
From here on the client can call ``sdram_read`` and ``sdram_write`` to access the physical memory. 
For example::

   s_sdram_state sdram_state;
   sdram_init_state(c_server, sdram_state);

where ``c_server`` is the streaming channel to the ``sdram_server``.


Safety through the use of movable pointers
..........................................

The API makes use of movable pointers to aid correct multi-threaded memory handling. 
``sdram_read`` and ``sdram_write`` pass ownership of the memory from the client to 
the server. The client is no longer able to access the memory. The memory ownership 
is returned to the client on a call return from ``sdram_complete``. For example::

   unsigned buffer[N];
   unsigned * movable buffer_pointer = buffer;

   //buffer_pointer is fully accessible

   sdram_read (c_sdram_server, state, address, word_count, move(buffer_pointer));

   //during this region the buffer_pointer is null and cannot be read from or written to

   sdram_complete(c_sdram_server, state, buffer_pointer);

   //now buffer_pointer is accessible again

.. tip::
  Note that, despite supporting a 16 bit SDRAM data bus width, the native word length of the API is 32 bits.
  This means that the address provided to the read/wrote functions is the 32b address of the memory.
  For example, address 0x0001 returns a long word (32 bit) from the 4th to 7th byte location of the SDRAM.
  Only 32 bit operations are supported; the user should read-modify-write to support modification of smaller word sizes. 

During the scope of the movable pointer variable the pointer can point at any memory location, 
however, at the end of the scope the pointer must point at its original instantiation. 

For example the following is acceptable::

  {
     unsigned buffer_0[N];
     unsigned buffer_1[N];
     unsigned * movable buffer_pointer_0 = buffer_0;
     unsigned * movable buffer_pointer_1 = buffer_1;
  
     sdram_read (c_sdram_server, state, address, word_count, move(buffer_pointer_0));
     sdram_write (c_sdram_server, state, address, word_count, move(buffer_pointer_1));

     //both buffer_pointer_0 and buffer_pointer_1 are null here
  
     sdram_complete(c_server, sdram_state, buffer_pointer_0);
     sdram_complete(c_server, sdram_state, buffer_pointer_1);
  }

but the following is not as the movable pointers are no longer 
point at the same memory when leaving scope as they were when they were instantiated::

  {
     unsigned buffer_0[N];
     unsigned buffer_1[N];
     unsigned * movable buffer_pointer_0 = buffer_0;
     unsigned * movable buffer_pointer_1 = buffer_1;
  
     sdram_read (c_sdram_server, state, address, word_count, move(buffer_pointer_0));
     sdram_write (c_sdram_server, state, address, word_count, move(buffer_pointer_1));
  
     //both buffer_pointer_0 and buffer_pointer_1 are null here
  
     sdram_complete(c_server, sdram_state, buffer_pointer_1);	//return to opposite pointer
     sdram_complete(c_server, sdram_state, buffer_pointer_0);
  }


Shutdown
........

The ``sdram_server`` may be shutdown, i.e. the thread and all its resources may 
be freed, with a call to ``sdram_shutdown``.


Memory allocator API
--------------------

The purpose of this library is to allow multiple tasks to share a common memory 
address space. All of the clients may request a number of bytes from the memory space 
and will either be allocated a base address to use the requested amount of memory from 
or will receive an error. All clients of the memory address allocator must be on the same tile.

API
---

.. doxygenfunction:: sdram_server
.. doxygenfunction:: sdram_init_state
.. doxygenfunction:: sdram_complete
.. doxygenfunction:: sdram_write
.. doxygenfunction:: sdram_read
.. doxygenfunction:: sdram_shutdown

.. doxygeninterface:: memory_address_allocator_i
.. doxygenfunction:: memory_address_allocator

|newpage|

|appendix|

Known Issues
------------
 
 - XS1 devices can support a maximum of 64 Mb SDRAM (8 MBytes) using a 8b column address. This is a technical limitation due to addressing modes in the XS1 device and cannot be worked around using the current library architecture.
 - XS2 (xCORE-200) devices can support a maximum of 256 Mb SDRAM (32 MBytes) using a 9b column address. 512 Mb devices are supportable with some modifications. Please see the following github issue https://github.com/xmos/lib_sdram/issues/20 for details.
 - No Application note is provided currently. Please see https://github.com/xmos/lib_sdram/examples for a simple usage example
 - The IP assumes a 500MHz core clock. It may be possible to support other core clock frequencies. However, the I/O timing must be re-calculated to populate the read delay constants for the apprioriate clock divider. These may be found in ``server.xc``.

.. include:: ../../../CHANGELOG.rst
