#ifndef __USER_XCCODE_H_
#define __USER_XCCODE_H_

#include "xtcp.h"
#include "eth_audio.h"

void stop_all_timetask();

void wifi_ioset(uint8_t io_tmp);

void user_lan_uart0_tx(uint8_t *data,uint8_t len,uint8_t mode);

void user_uart_tx(uint8_t *data,uint8_t len);

void user_xtcp_connect(uint8_t ipaddr[]);

void user_xtcp_connect_tcp(xtcp_ipaddr_t ipaddr);

void set_audio_vol(uint8_t ch,uint8_t vol);

void user_could_send(uint8_t pol_type);

void user_xtcp_send(xtcp_connection_t conn,uint8_t colud_f);

void user_xtcp_unlisten(unsigned port_number);

void user_audio_senden(uint8_t ch);
   
void user_audio_send_dis(uint8_t ch);

void user_xtcp_close(xtcp_connection_t conn);

void user_udpconn_close(xtcp_connection_t conn);

void user_xtcp_ipconfig(xtcp_ipconfig_t ipconfig);

void user_fl_sector_read(unsigned sector_num);

void user_fl_sector_write(unsigned sector_num);

void user_music_play(uint8_t ch);

void user_music_stop(uint8_t ch);

void audio_moudle_set();

void user_audio_desip_set(uint8_t ch);

void user_updatip_set(uint8_t mac[],uint8_t ip[]);

void set_audio_type(enum AUDIO_TYPE_E aux_type[]);

void user_fl_get_patchlist(uint8_t *buff);

void user_fl_get_musiclist(uint8_t index,uint8_t *buff);

int user_file_rename(uint8_t *fsrc,uint8_t *fdes);

int user_file_copy(uint8_t *fsrc,unsigned n1,uint8_t *fdes,unsigned n2);

int user_file_delete(uint8_t *fsrc,unsigned n);

int user_file_move(uint8_t *fsrc,unsigned n1,uint8_t *fdes,unsigned n2);

int user_file_mir(uint8_t *f_name);

int user_filefolder_del(uint8_t *f_name);

uint8_t user_file_progress(uint8_t *progress);

void user_file_bat_write(uint8_t num,uint8_t *buff);

void user_file_bat_read(uint8_t num,uint8_t *buff);

void user_divsrv_write(uint8_t num,uint8_t *buff);

void user_divsrv_read(uint8_t num,uint8_t *buff);

void user_file_stop();

int user_xtcp_connect_udp(unsigned port_number, xtcp_ipaddr_t ipaddr, xtcp_connection_t *new_conn);

uint8_t start_sysdat_backup();

void backup_system_chk(uint8_t *state,uint8_t *bar);

void user_xtcp_fifo_get(uint8_t num,uint8_t buff[],uint8_t tx_rx_f);

void user_xtcp_fifo_put(uint8_t num,uint8_t buff[],uint8_t tx_rx_f);


#endif //__USER_XCCODE_H_

