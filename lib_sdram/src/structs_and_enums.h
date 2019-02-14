// Copyright (c) 2014-2016, XMOS Ltd, All rights reserved
#ifndef STRUCTS_AND_ENUMS_H_
#define STRUCTS_AND_ENUMS_H_

#ifdef __XC__
#define POINTER_PARAM(type, name) type *unsafe name
#else
#define POINTER_PARAM(type, name) type *name
#endif

#define SDRAM_MAX_CMD_BUFFER 8

typedef struct {
    unsigned address;
    unsigned word_count;
    POINTER_PARAM(unsigned, buffer);
} sdram_cmd;

/*
 * \typedef
 * This is the internal state used to manage the command buffers. It should not be accessed.
 */
typedef struct s_sdram_state {
  unsigned head;
  unsigned pending_cmds;
  POINTER_PARAM(sdram_cmd, cmd[SDRAM_MAX_CMD_BUFFER]);
} s_sdram_state;

typedef enum {
    SDRAM_CMD_READ,
    SDRAM_CMD_WRITE,
    SDRAM_CMD_SHUTDOWN
} e_command;

#endif /* STRUCTS_AND_ENUMS_H_ */
