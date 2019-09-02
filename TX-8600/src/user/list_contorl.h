#ifndef __LIST_CONTORL_H
#define __LIST_CONTORL_H

#include "xtcp.h"
#include "sys_config_dat.h"

#ifndef null
#define null -1
#endif

#if defined(__cplusplus) || defined(__XC__)
extern "C" {
#endif

//===================================================================
// 连接链表
//===================================================================
extern conn_list_t *conn_list_head;
extern conn_list_t *conn_list_end;
extern conn_list_t *conn_list_tmp;

void conn_list_init();  //CONN链表初始化

uint8_t create_conn_node(xtcp_connection_t *conn);  //创建一个CONN节点 return 0 失败 return 1成功

uint8_t delete_conn_node(int id);   //删除一个CONN节点 return 0 失败 return 1成功

conn_list_t *get_conn_info_p(int id);

conn_list_t *get_conn_for_ip(xtcp_ipaddr_t ip);

//===================================================================
// 设备链表
//===================================================================
div_node_t *get_div_info_p(uint8_t mac[]);

uint8_t create_div_node();

uint8_t delete_div_node(uint8_t mac[]);

void div_list_init();

// 初始化时 按id建立链表
uint8_t div_node_creat_forid(uint8_t id);

//===================================================================
// 任务链表
//创建任务节点
uint8_t create_task_node();
// 初始化按ID建列表
uint8_t create_task_node_forid(uint16_t id);
// 删除任务节点
uint8_t delete_task_node(uint16_t id);
// 获取目标任务节点
timetask_t *get_task_info_p(uint16_t id);
//==================================================================
// 即时任务
//创建一个即时任务节点 return 0 失败 return 1成功
uint8_t create_rttask_node();
// 按id创建任务
uint8_t create_rttask_node_forid(uint16_t id);
//删除一个任务节点 
uint8_t delete_rttask_node(uint16_t id);
//创建正在运行的即时任务
uint8_t create_rttask_run_node(uint16_t id);
// 删除正在运行的即时任务
uint8_t delete_rttask_run_node(uint16_t id);
// 查找正在运行中任务
uint8_t rttask_run_chk(uint16_t id);

//==================================================================
// 分区列表
//==================================================================
void area_list_init();

//==================================================================
// 账户列表
//==================================================================
void account_list_init();

void account_list_read();


#if defined(__cplusplus) || defined(__XC__)
}
#endif

#endif //__LIST_CONTORL_H

