// Copyright (c) 2014-2016, XMOS Ltd, All rights reserved
#include <xs1.h>
//#include <xassert.h>
#include "sdram.h"
#include "structs_and_enums.h"

void sdram_init_state(streaming chanend c_sdram_server, s_sdram_state &s){
    unsafe {
        c_sdram_server :> s.cmd[0];
        for(unsigned i=1;i<SDRAM_MAX_CMD_BUFFER;i++)
            s.cmd[i] = s.cmd[i-1] + 1;
    }
    s.head = 0;
    s.pending_cmds = 0;

    unsigned local_tile_id;
    c_sdram_server :> local_tile_id;

    //if(local_tile_id != get_local_tile_id())
    //    fail(1);//FIXME

}

static int send_cmd(streaming chanend c_sdram_server, s_sdram_state &state, unsigned address, unsigned word_count,
        unsigned * buffer, e_command cmd){
    if(state.pending_cmds == SDRAM_MAX_CMD_BUFFER)
        return 1;

    unsigned index = (state.head + state.pending_cmds)%SDRAM_MAX_CMD_BUFFER;

    unsafe {
        sdram_cmd * unsafe c = state.cmd[index];
        c->address = address;
        c->word_count = word_count;
        c->buffer = (buffer);
        c_sdram_server <: (char)cmd;
    }

    state.pending_cmds++;
    return 0;
}

int sdram_read (streaming chanend c_sdram_server, s_sdram_state &state, unsigned address, unsigned word_count,
        unsigned * buffer){
    return send_cmd(c_sdram_server, state, address, word_count, (buffer), SDRAM_CMD_READ);
}

int sdram_write (streaming chanend c_sdram_server, s_sdram_state &state, unsigned address, unsigned word_count,
        unsigned * buffer){
    return send_cmd(c_sdram_server, state, address, word_count, (buffer), SDRAM_CMD_WRITE);
}

void sdram_complete(streaming chanend c_sdram_server, s_sdram_state &state/*, unsigned * & buffer*/) {
    char c;
    c_sdram_server :> c;
    state.pending_cmds--;
    //unsigned index = state.head%SDRAM_MAX_CMD_BUFFER;
#if 0
    unsafe {
      buffer = (state.cmd[index]->buffer);
    }
#endif
    state.head++;
}

void sdram_shutdown(streaming chanend c_sdram_server){
    c_sdram_server <: (char)SDRAM_CMD_SHUTDOWN;
}

int sdram_read_complete (streaming chanend c_sdram_server, s_sdram_state &state, unsigned address, unsigned word_count,
        unsigned * buffer){
    int res = send_cmd(c_sdram_server, state, address, word_count, (buffer), SDRAM_CMD_READ);
    sdram_complete(c_sdram_server, state);
    return res;
}

int sdram_write_complete (streaming chanend c_sdram_server, s_sdram_state &state, unsigned address, unsigned word_count,
        unsigned * buffer){
    int res = send_cmd(c_sdram_server, state, address, word_count, (buffer), SDRAM_CMD_WRITE);
    sdram_complete(c_sdram_server, state);
    return res;
}

