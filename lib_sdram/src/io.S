// Copyright (c) 2014-2017, XMOS Ltd, All rights reserved
.text
#ifdef __XS2A__
//xCore 200 optimised version with support for >8bit row addresses
.cc_top sdram_block_write.function
.align 4
.globl sdram_block_write
.type  sdram_block_write, @function
sdram_block_write:
.issue_mode dual

#define WRITE_STACK_WORDS 4
	DUALENTSP_lu6 WRITE_STACK_WORDS

#define buffer_pointer		r0
#define num_words			r3
#define new_buff_ptr		r6
#define new_num_words		r7
	//r0 buffer pointer
	//r1 sdram_ports struct
	//r2 t0	- start time
	//r3 word_count
	//sp[1 + WRITE_STACK_WORDS] row_words

#define w_ras	 			r0
#define w_dq_ah	 			r1
#define w_term_time 		r2
#define w_we 				r3
#define w_two				r5
#define w_temp 				r11


	//Save registers
	stw r6, sp[0]
	stw r5, sp[2]
	stw r7, sp[3]

	//calc the new buffer pointer
	ldaw new_buff_ptr, buffer_pointer[num_words]

	//copy negative num_words to new_num_words, because we count up to zero
	ldc w_temp, 0
	sub new_num_words, w_temp, num_words

	//calc burst termination time
	add w_term_time, r2, 2
	add w_term_time, w_term_time, r3 //Add word count * 2
	add w_term_time, w_term_time, r3

	//Load port resource IDs
	ldw w_ras, 		r1[2]	//ras - TRASHES r0, buffer_pointer
	ldw w_we, 		r1[3]	//we - TRASHES r3, word count
	ldw w_dq_ah, 	r1[0]	//dq_ah - TRASHES r1, port struct
	ldc w_two, 2

.align 8
wr_loop:
	//Dual issue write loop
    {ldw w_temp, new_buff_ptr[new_num_words]; add new_num_words, new_num_words, 1}
    {out res[w_dq_ah], w_temp; bt new_num_words, wr_loop}

wr_loop_done:
	//Burst stop
	setpt res[w_we], w_term_time
	outpw res[w_we], w_two, 2

	mkmsk w_temp, 32
	out res[w_dq_ah], w_temp

	//Precharge
	add w_term_time, w_term_time, 8
	setpt res[w_we],  w_term_time
	outpw res[w_we],  w_two, 2
	setpt res[w_ras], w_term_time
	outpw res[w_ras], w_two, 2

	//Restore registers
	ldw r6, sp[0]
	ldw r5, sp[2]
	ldw r7, sp[3]


	retsp WRITE_STACK_WORDS
.globl sdram_block_write.nstackwords
.linkset sdram_block_write.nstackwords, WRITE_STACK_WORDS
.globl sdram_block_write.maxtimers
.linkset sdram_block_write.maxtimers, 0
.globl sdram_block_write.maxchanends
.linkset sdram_block_write.maxchanends, 0
.globl sdram_block_write.maxthreads
.linkset sdram_block_write.maxthreads, 1
.cc_bottom sdram_block_write.function

////////////////////////////////////////////////
#else
//XS1 version - maximum 8b row address
.cc_top sdram_block_write.function
.align 4
.globl sdram_block_write
.type  sdram_block_write, @function
sdram_block_write:
.issue_mode single

#define WRITE_STACK_WORDS 3
	ENTSP_lu6 WRITE_STACK_WORDS

 	//r0 buffer pointer
	//r1 sdram_ports struct
	//r2 t0	- start time
	//r3 word_count
	//sp[1 + WRITE_STACK_WORDS] row_words

#define w_ras	 			r0
#define w_dq_ah	 			r1
#define w_term_time 		r2
#define w_we 				r3
#define w_cp_save			r4
#define w_two				r5
#define w_temp 				r11

	stw r4, sp[0]
	stw r5, sp[2]

	//save the cp
	ldaw r11, cp[0]
	mov w_cp_save, r11

	//adjust the buffer pointer
	ldw w_temp, 	sp[1+WRITE_STACK_WORDS]	//row_words
	sub w_temp, w_temp, r3 //word_count
	ldaw w_temp, r0[-w_temp]
	set cp, w_temp

	add w_term_time, r2, 2
	add w_term_time, w_term_time, r3 //Add word count * 2
	add w_term_time, w_term_time, r3

	ldw w_temp, 	sp[1+WRITE_STACK_WORDS]	//row_words
	sub w_temp, w_temp, r3
	add w_temp, w_temp, w_temp

	ldw w_ras, 		r1[2]	//ras
	ldw w_we, 		r1[3]	//we
	ldw w_dq_ah, 	r1[0]	//dq_ah

.align 4
	ldc w_two, 2
	bru w_temp

	//(the next IO is blocking)
	#include "sdram_block_write_body.inc"

	//Burst stop
	setpt res[w_we], w_term_time
	outpw res[w_we], w_two, 2

	mkmsk w_temp, 32
	out res[w_dq_ah], w_temp

	//Precharge
	add w_term_time, w_term_time, 8
	setpt res[w_we],  w_term_time
	outpw res[w_we],  w_two, 2
	setpt res[w_ras], w_term_time
	outpw res[w_ras], w_two, 2

	set cp, w_cp_save

	ldw r5, sp[2]
	ldw r4, sp[0]


	retsp WRITE_STACK_WORDS
.globl sdram_block_write.nstackwords
.linkset sdram_block_write.nstackwords, WRITE_STACK_WORDS
.globl sdram_block_write.maxtimers
.linkset sdram_block_write.maxtimers, 0
.globl sdram_block_write.maxchanends
.linkset sdram_block_write.maxchanends, 0
.globl sdram_block_write.maxthreads
.linkset sdram_block_write.maxthreads, 1
.cc_bottom sdram_block_write.function
#endif

//////////////////////////////////////////////////////////////////

#ifdef __XS2A__
//xCore 200 optimised version with support for >8bit row addresses
.cc_top sdram_block_read.function
.align 4
.globl sdram_block_read
.type  sdram_block_read, @function
sdram_block_read:
#define READ_STACK_WORDS 6
.issue_mode dual

    DUALENTSP_lu6 READ_STACK_WORDS

 	//r0 buffer pointer
	//r1 sdram_ports struct
	//r2 t0	- start time
	//r3 word_count
	//sp[1] row_words
	//sp[2] cas_latency

#define ras	 			r0
#define dq_ah	 		r1
#define term_time 		r2
#define jump_reg 		r3
#define we 				r4
#define start_time		r5
#define new_buf_ptr			r6
#define temp 			r11

	//Save registers on stack
	stw r4, sp[0]
	stw r5, sp[3]
	stw r6, sp[4]

	//Copy buffer pointer to new_buf_ptr
	sub new_buf_ptr, r0, 0

	//set the first input time (t+CAS+N)
	ldw temp, 	sp[READ_STACK_WORDS+2]	//cas_latency
	add start_time, r2, temp

	//calculate the time to issue the terminate cmd
	add term_time, start_time, r3
	add term_time, term_time, r3	//Add word count * 2

	//Calculate the entry point to the unrolled inner loop
	ldc temp, 	256						//The number of in/store cycles unrolled in the jump table
	sub jump_reg, temp, r3				//Subtract the number of words to read
	shl jump_reg, jump_reg, 1			//Multiply by 2 (2 x 16b instructions at a time)

	//load the ports
	ldw ras, 	r1[2]	//ras
	ldw we, 	r1[3]	//we
	ldw dq_ah, 	r1[0]	//dq_ah

	//issue the we term signals(non-blocking)
	setpt res[we], term_time
	ldc temp, 2
	outpw res[we], temp, 2

	//inc the term time by a few for the precharge
	ldc temp, 12
	add term_time, term_time, temp

	//Delay port read until start time
	setpt res[dq_ah], start_time	//(blocking)

.align 8
	in temp, res[dq_ah]	//input junk
	bru jump_reg

sdram_block_read_jump_table:
	#include "sdram_block_read_body_xs2.inc"

	//r0	ras
	//r1	dq_ah
	//r2	term_time
	//r3	-
	//r4	we
	//r5	-
	//r11	used as a temp

sdram_block_read_cleanup:
	syncr res[dq_ah]

	//Precharge
	ldc temp, 2
	setpt res[ras], term_time
	outpw res[ras], temp, 2
	setpt res[we],  term_time
	outpw res[we],  temp, 2

	mkmsk temp, 32
	out res[dq_ah], temp

	ldw r6, sp[4]
	ldw r5, sp[3]
	ldw r4, sp[0]
	retsp READ_STACK_WORDS

.globl sdram_block_read.nstackwords
.linkset sdram_block_read.nstackwords, READ_STACK_WORDS
.globl sdram_block_read.maxtimers
.linkset sdram_block_read.maxtimers, 0
.globl sdram_block_read.maxchanends
.linkset sdram_block_read.maxchanends, 0
.globl sdram_block_read.maxthreads
.linkset sdram_block_read.maxthreads, 1

.cc_bottom sdram_block_read.function

//////////////////////////////////////////////////////////////////////////////
#else
//XS1 version. Supports maximum 8b row address

.cc_top sdram_block_read.function
.align 8
.globl sdram_block_read
.type  sdram_block_read, @function
sdram_block_read:
.issue_mode single
#define READ_STACK_WORDS 6
    
    ENTSP_lu6 READ_STACK_WORDS

 	//r0 buffer pointer
	//r1 sdram_ports struct
	//r2 t0	- start time
	//r3 word_count
	//sp[1] row_words
	//sp[2] cas_latency

#define ras	 			r0
#define dq_ah	 		r1
#define term_time 		r2
#define jump_reg 		r3
#define we 				r4
#define start_time		r5
#define sp_save			r6
#define temp 			r11

	stw r4, sp[0]
	stw r5, sp[3]
	stw r6, sp[4]

	//save the dp to sp[5]
	ldaw temp, dp[0]
	stw temp, sp[5]

	//adjust the buffer pointer
	ldw temp, 	sp[READ_STACK_WORDS+1]	//row_words
	sub temp, temp, r3
	ldaw temp, r0[-temp]
	set dp, temp

	ldw temp, 	sp[READ_STACK_WORDS+2]	//cas_latency
	add start_time, r2, temp

	//calculate the time to issue the terminate cmd
	add term_time, start_time, r3
	add term_time, term_time, r3	//Add word count * 2

	ldw temp, 	sp[READ_STACK_WORDS+1]	//row_words
	sub jump_reg, temp, r3
	add jump_reg, jump_reg, jump_reg

	ldaw sp_save, sp[0]

	//load the ports
	ldw ras, 	r1[2]	//ras
	ldw we, 	r1[3]	//we
	ldw dq_ah, 	r1[0]	//dq_ah

	//issue the we term signals(non-blocking)
	setpt res[we], term_time
	ldc temp, 2
	outpw res[we], temp, 2

	//inc the term time by a few for the precharge
	ldc temp, 12
	add term_time, term_time, temp

	//store the midway dp
	ldaw temp, dp[63]
	add temp, temp, 4
	set sp, temp

	setpt res[dq_ah], start_time	//(blocking)
.align 4
	in temp, res[dq_ah]	//input junk
	bru jump_reg
	#include "sdram_block_read_body.inc"

	//r0	ras
	//r1	dq_ah
	//r2	term_time
	//r3	-
	//r4	we
	//r5	-
	//r11	used as a temp

	syncr res[dq_ah]

	//Precharge
	ldc temp, 2
	setpt res[ras], term_time
	outpw res[ras], temp, 2
	setpt res[we],  term_time
	outpw res[we],  temp, 2

	mkmsk temp, 32
	out res[dq_ah], temp

	set sp, sp_save

	ldw temp, sp[5]
	set dp, temp

	ldw r6, sp[4]
	ldw r5, sp[3]
	ldw r4, sp[0]
	retsp READ_STACK_WORDS

.globl sdram_block_read.nstackwords
.linkset sdram_block_read.nstackwords, READ_STACK_WORDS
.globl sdram_block_read.maxtimers
.linkset sdram_block_read.maxtimers, 0
.globl sdram_block_read.maxchanends
.linkset sdram_block_read.maxchanends, 0
.globl sdram_block_read.maxthreads
.linkset sdram_block_read.maxthreads, 1

.cc_bottom sdram_block_read.function
#endif
