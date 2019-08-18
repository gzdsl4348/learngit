#include "list_contorl.h"
#include "xtcp.h"
#include <stdint.h>
#include <string.h>
#include "debug_print.h"
#include "fl_buff_decode.h"
#include "user_unti.h"
#include "sys_log.h"

//==============================================================================================================
// 链接列表
//==============================================================================================================

conn_list_t *conn_list_head=null;
conn_list_t *conn_list_end=null;
conn_list_t *conn_list_tmp=null;


//创建一个CONN节点 return 0 失败 return 1成功
uint8_t create_conn_node(xtcp_connection_t *conn){
	if(conn_list_head==null){
		conn_list_head = &conn_list[0];
		conn_list_end = conn_list_head;
        memcpy(&conn_list_head->conn,conn,sizeof(xtcp_connection_t));
        conn_list_end->over_time = 0;
		return 1;
	}
	for(uint8_t i=0;i<MAX_UDP_CONNET;i++){
		if((conn_list[i].next_p==null)&&(&conn_list[i]!=conn_list_end)){// 找空置元素
			conn_list_end->next_p = &conn_list[i];
			conn_list_end = &conn_list[i]; //插入新元素
            memcpy(&conn_list[i].conn,conn,sizeof(xtcp_connection_t)); //插入新数值
            conn_list_end->over_time = 0;
            return 1;
		}
	}	
	return 0;	
}

//删除一个CONN节点 return 0 失败 return 1成功
uint8_t delete_conn_node(int id){
	if(conn_list_head==null)
		return 0;
    conn_list_t *node_last;
	conn_list_t *node_tmp = conn_list_head;
    //删除第一个节点
    if(node_tmp->conn.stack_conn==id){
        conn_list_head = conn_list_head->next_p;
        node_tmp->next_p=null;
        return 1;
    }
    //删除其他节点
	while(node_tmp->next_p != null){
        node_last = node_tmp;
        node_tmp = node_tmp->next_p;
		if(node_tmp->conn.stack_conn==id){
            node_last->next_p = node_tmp->next_p;
            node_tmp->next_p=null;
            if(node_tmp == conn_list_end)
                conn_list_end = node_last;
			return 1;
		}	
	}
    return 0;
}

conn_list_t *get_conn_info_p(int id){
    conn_list_tmp = conn_list_head;
    if(conn_list_tmp == null)    // NO POINT
        return null;
    for(uint8_t i=0;i<MAX_UDP_CONNET;i++){
        if(conn_list_tmp->conn.id==id)
            return conn_list_tmp;
        if(conn_list_tmp->next_p==null)
            return null;
        conn_list_tmp = conn_list_tmp->next_p;
    }
}

conn_list_t *get_conn_for_ip(xtcp_ipaddr_t ip){
    conn_list_tmp = conn_list_head;
    if(conn_list_tmp == null)    // NO POINT
        return null;
    for(uint8_t i=0;i<MAX_UDP_CONNET;i++){
        if(ip_cmp(conn_list_tmp->conn.remote_addr,ip))
            return conn_list_tmp;
        if(conn_list_tmp->next_p==null)
            return null;
        conn_list_tmp = conn_list_tmp->next_p;
    }
}

void conn_list_init(){
	for(uint8_t i=0;i<MAX_UDP_CONNET;i++){
		conn_list[i].next_p=null;
		memset(&conn_list[i].conn,0x00,sizeof(xtcp_connection_t));
	}
}

//==============================================================================================================
// 设备列表
//==============================================================================================================

//创建一个设备节点 return 0 失败 return 1成功
uint8_t create_div_node(){
	if(div_list.div_head_p==null){
		div_list.div_head_p = &div_list.div_node[0];
		div_list.div_end_p = div_list.div_head_p;
        div_list.div_head_p->div_info.id = 0;
        div_list.div_tol++;
		return 1;
	}
	for(uint8_t i=0;i<MAX_DIV_LIST;i++){
		if((div_list.div_node[i].next_p==null)&&(&div_list.div_node[i]!=div_list.div_end_p)){// 找空置元素
			div_list.div_end_p->next_p = &div_list.div_node[i];
			div_list.div_end_p = &div_list.div_node[i]; //插入新元素
			div_list.div_end_p->div_info.id = i;
            div_list.div_tol++;
            return 1;
		}
	}	
	return 0;	
}

//删除一个设备节点 return 0 失败 return 1成功
uint8_t delete_div_node(uint8_t mac[]){
	if(div_list.div_head_p==null)
		return 0;
    div_node_t *node_last;
	div_node_t *node_tmp = div_list.div_head_p;
    //删除第一个节点
    if(mac_cmp(div_list.div_head_p->div_info.mac,mac)){
        div_list.div_head_p = div_list.div_head_p->next_p;
        node_tmp->next_p = null;
        node_tmp->div_info.id=0xFF;
        div_list.div_tol--;
        return 1;
    }
    //删除其他节点
	while(node_tmp->next_p != null){
        node_last = node_tmp;
        node_tmp = node_tmp->next_p;
		if(mac_cmp(node_tmp->div_info.mac,mac)){
            node_last->next_p = node_tmp->next_p;
            node_tmp->next_p=null;
            node_tmp->div_info.id=0xFF;
            div_list.div_tol--;
            if(node_tmp == div_list.div_end_p)
                div_list.div_end_p = node_last; //刷新尾指针
			return 1;
		}	
	}
    return 0;
}

//获取设备信息指针
div_node_t *get_div_info_p(uint8_t mac[]){
    div_node_t *node_tmp;
    node_tmp = div_list.div_head_p;
    if(node_tmp == null)    // NO POINT
        return null;
    for(uint8_t i=0;i<MAX_DIV_LIST;i++){
        if(mac_cmp(node_tmp->div_info.mac,mac))
            return node_tmp;
        if(node_tmp->next_p==null)
            return null;
        node_tmp = node_tmp->next_p;
    }
    return null;
}

//初始化设备列表
void div_list_init(){
    div_list.div_tol=0;
    div_list.div_head_p = null;
    div_list.div_end_p = null;
	for(uint8_t i=0;i<MAX_DIV_LIST;i++){
		div_list.div_node[i].next_p=null;
        div_list.div_node[i].div_info.id = 0xFF;
	}     
}

// 初始化时 按id建立链表
uint8_t div_node_creat_forid(uint8_t id){
    if(div_list.div_node[id].div_info.id==0xFF)
        return 0;
	if(div_list.div_head_p ==null){
		div_list.div_head_p = &div_list.div_node[id];
		div_list.div_end_p = div_list.div_head_p;
        div_list.div_tol++;
		return 1;
	}
    div_list.div_end_p->next_p = &div_list.div_node[id];
    div_list.div_end_p = &div_list.div_node[id];
    div_list.div_tol++;
    return 1;
}

//==============================================================================================================
// 任务链表

//创建一个任务节点 return 0 失败 return 1成功
uint8_t create_task_node(){
	if(timetask_list.all_timetask_head==null){
		timetask_list.all_timetask_head = &timetask_list.timetask[1];
		timetask_list.all_timetask_end = timetask_list.all_timetask_head;
        timetask_list.all_timetask_head->id = 1;
        timetask_list.task_total++;
		return 1;
	}
	for(uint16_t i=1;i<MAX_HOST_TASK;i++){
		if((timetask_list.timetask[i].all_next_p==null)&&(timetask_list.timetask[i].id==0xFFFF)){// 找空置元素
			timetask_list.all_timetask_end->all_next_p = &timetask_list.timetask[i];
			timetask_list.all_timetask_end = &timetask_list.timetask[i]; //插入新元素
			timetask_list.all_timetask_end->id = i;
            timetask_list.task_total++;
            return 1;
		}
	}	
    return 0;
}

// 初始化 按ID建列表
uint8_t create_task_node_forid(uint16_t id){
    if(id>MAX_HOST_TASK)
        return 0;
    if(timetask_list.timetask[id].id==0xFFFF)
        return 0;
	if(timetask_list.all_timetask_head==null){
		timetask_list.all_timetask_head = &timetask_list.timetask[id];
		timetask_list.all_timetask_end = timetask_list.all_timetask_head;
        timetask_list.task_total++;
		return 1;
	}
    timetask_list.all_timetask_end->all_next_p = &timetask_list.timetask[id];
    timetask_list.all_timetask_end = &timetask_list.timetask[id];
    timetask_list.task_total++;
    return 1;
}


//删除一个任务节点 return 0 失败 return 1成功
uint8_t delete_task_node(uint16_t id){
	if(timetask_list.all_timetask_head==null)
		return 0;
    timetask_t *node_last;
	timetask_t *node_tmp = timetask_list.all_timetask_head;
    //删除第一个节点
    if(node_tmp->id == id){
        timetask_list.all_timetask_head = timetask_list.all_timetask_head->all_next_p;
        node_tmp->all_next_p=null; //旧指针
        node_tmp->id = 0xFFFF;
        timetask_list.task_total--;
        return 1;
    }
    //删除其他节点
	while(node_tmp->all_next_p != null){
        node_last = node_tmp;
        node_tmp = node_tmp->all_next_p; //下一个节点
		if(node_tmp->id == id){
            node_last->all_next_p = node_tmp->all_next_p;
            node_tmp->all_next_p=null;
            node_tmp->id=0xFFFF;
            timetask_list.task_total--;
            if(node_tmp == timetask_list.all_timetask_end )
                timetask_list.all_timetask_end = node_last; //刷新尾指针
			return 1;
		}	
	}
    return 0;
}


//获取任务信息指针
timetask_t *get_task_info_p(uint16_t id){
    timetask_t *task_tmp_p;
    task_tmp_p = timetask_list.all_timetask_head;
    if(task_tmp_p == null)    // NO POINT
        return null;
    for(uint16_t i=0;i<MAX_HOST_TASK;i++){
        if(task_tmp_p->id == id)
            return task_tmp_p;
        if(task_tmp_p->all_next_p==null)
            return null;
        task_tmp_p = task_tmp_p->all_next_p;
    }
    return null;
}
//==============================================================================================================
// 即时任务链表
//创建一个即时任务节点 return 0 失败 return 1成功
uint8_t create_rttask_node(){
	if(rttask_lsit.all_head_p==null){
        //xtcp_debug_printf("head\n");
		rttask_lsit.all_head_p = &rttask_lsit.rttask_info[1];
		rttask_lsit.all_end_p = rttask_lsit.all_head_p;
        rttask_lsit.all_head_p->rttask_id = 1;
        rttask_lsit.rttask_tol++;
		return 1;
	}
	for(uint16_t i=1;i<MAX_RT_TASK_NUM;i++){
        //xtcp_debug_printf("find %d,%d,%d\n",i,rttask_lsit.rttask_info[i].all_next_p,rttask_lsit.rttask_info[i].rttask_id);
		if((rttask_lsit.rttask_info[i].all_next_p==null)&&(rttask_lsit.rttask_info[i].rttask_id==0xFFFF)){// 找空置元素
			rttask_lsit.all_end_p->all_next_p = &rttask_lsit.rttask_info[i];
			rttask_lsit.all_end_p = &rttask_lsit.rttask_info[i]; //插入新元素
			rttask_lsit.all_end_p->rttask_id = i;
            rttask_lsit.rttask_tol++;
            
            return 1;
		}
	}	
    return 0;
}
// 初始化时凭ID 创建节点
uint8_t create_rttask_node_forid(uint16_t id){
    //if(rttask_lsit.rttask_info[id].rttask_id==0xFFFF)
    //    return 0;
	if(rttask_lsit.all_head_p==null){
		rttask_lsit.all_head_p = &rttask_lsit.rttask_info[id];
        rttask_lsit.all_end_p = rttask_lsit.all_head_p;
        rttask_lsit.all_head_p->rttask_id = id;
        rttask_lsit.rttask_tol++;
		return 1;
	}
    rttask_lsit.all_end_p->all_next_p = &rttask_lsit.rttask_info[id];
    rttask_lsit.all_end_p = &rttask_lsit.rttask_info[id];
    rttask_lsit.all_end_p->rttask_id = id;
    return 1;
}

//删除一个任务节点 return 0 失败 return 1成功
uint8_t delete_rttask_node(uint16_t id){
	if(rttask_lsit.all_head_p==null)
		return 0;
    rttask_info_t *node_last;
	rttask_info_t *node_tmp = rttask_lsit.all_head_p;
    //删除第一个节点
    if(node_tmp->rttask_id == id){
        rttask_lsit.all_head_p = rttask_lsit.all_head_p->all_next_p;
        node_tmp->all_next_p=null; //旧指针
        node_tmp->rttask_id = 0xFFFF;
        rttask_lsit.rttask_tol--;
        return 1;
    }
    //删除其他节点
	while(node_tmp->all_next_p != null){
        node_last = node_tmp;
        node_tmp = node_tmp->all_next_p;
		if(node_tmp->rttask_id == id){
            node_last->all_next_p = node_tmp->all_next_p;
            node_tmp->all_next_p=null;
            node_tmp->rttask_id=0xFFFF;
            rttask_lsit.rttask_tol--;
            if(node_tmp == rttask_lsit.all_end_p )
                rttask_lsit.all_end_p = node_last; //刷新尾指针
			return 1;
		}	
	}
    return 0;
}

//根据ID 创建一个正在运行的即时任务节点 return 0 失败 return 1成功
uint8_t create_rttask_run_node(uint16_t id){
    if(rttask_lsit.rttask_info[id].rttask_id==0xFFFF)
        return 0;
    if((rttask_lsit.run_head_p==null)){
		rttask_lsit.run_head_p = &rttask_lsit.rttask_info[id];
        rttask_lsit.run_end_p = rttask_lsit.run_head_p;
		return 1;
	}
    if((rttask_lsit.rttask_info[id].run_next_p != null)&&(rttask_lsit.run_end_p->rttask_id =! id))
        return 0;
    rttask_lsit.run_end_p->run_next_p = &rttask_lsit.rttask_info[id];
    rttask_lsit.run_end_p = &rttask_lsit.rttask_info[id];
    return 1;
}

uint8_t rttask_run_chk(uint16_t id){
    rttask_info_t *run_tmp_p = rttask_lsit.run_head_p;
    while(run_tmp_p!=null){
        if(run_tmp_p->rttask_id == id){
            return 1;
        }
        run_tmp_p = run_tmp_p->run_next_p;
    }
    return 0;
}

uint8_t delete_rttask_run_node(uint16_t id){
	if(rttask_lsit.run_head_p==null)
		return 0;
    rttask_info_t *node_last;
	rttask_info_t *node_tmp = rttask_lsit.run_head_p;
    //删除第一个节点
    if(node_tmp->rttask_id == id){
        rttask_lsit.run_head_p = rttask_lsit.run_head_p->run_next_p;
        node_tmp->run_next_p=null; //旧指针
        return 1;
    }
    //删除其他节点
	while(node_tmp->run_next_p != null){
        node_last = node_tmp;
        node_tmp = node_tmp->run_next_p;
		if(node_tmp->rttask_id == id){
            node_last->run_next_p = node_tmp->run_next_p;
            node_tmp->run_next_p=null;
            if(node_tmp == rttask_lsit.run_end_p )
                rttask_lsit.run_end_p = node_last; //刷新尾指针
			return 1;
		}	
	}
    return 0;
}


//==============================================================================================================
// 分区列表
//---------------------------------------
// 列表初始化
void area_list_init(){
    for(uint8_t i=0;i<MAX_AREA_NUM; i++){
        area_info[i].area_sn = 0xFFFF;
    }
}

//================================================================================================================
// 账户列表初始化
const uint8_t MAIN_USER[DIV_NAME_NUM] = {0x61,0x00,0x64,0x00,0x6D,0x00,0x69,0x00,0x6E,0x00,0x00,0x00,0x00,0x00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}; //admin
void account_list_init(){
    for(uint8_t i=1; i<MAX_ACCOUNT_NUM ;i++){
        account_info[i].id=0xFF;
    }
    account_info[0].id=0x00;
    account_info[0].type=0x00;
    account_info[0].login_state=OFFLINE;
    // admin
    memcpy(account_info[0].name,MAIN_USER,DIV_NAME_NUM);
    // sn
    memcpy(account_info[0].sn,host_info.sn,SYS_PASSWORD_NUM);
    memset(&account_info[0].phone_num,0x00,DIV_NAME_NUM);
    memset(&account_info[0].time_info,0x00,3);
    memset(&account_info[0].date_info,0x01,4);
    memset(&account_info[0].build_time_info,0x00,3);
    memset(&account_info[0].build_date_info,0x01,4);
    account_info[0].div_tol = 0;
}


//===============================================================================
// 账户列表读取
//================================================================================
void account_list_read(){
    for(uint8_t i=0; i<MAX_ACCOUNT_NUM; i++){
        account_fl_read(&g_tmp_union.account_all_info,i);
        account_info[i] = g_tmp_union.account_all_info.account_info;
    }
    account_info[0].id=0x00;
    account_info[0].type=0x00;
    account_info[0].login_state=OFFLINE;
    // admin
    memcpy(account_info[0].name,MAIN_USER,DIV_NAME_NUM);
    // sn
    //uint8_t sn[14] = {0x31,0x00,0x32,0x00,0x33,0x00,0x34,0x00,0x35,0x00,0x36,0x00,00,00};  //sn  123456 
    //memcpy(account_info[0].sn,sn,SYS_PASSWORD_NUM);
    //memset(&account_info[0].time_info,0x00,3);
    //memset(&account_info[0].date_info,0x01,4);
    //memset(&account_info[0].build_time_info,0x00,3);
    //memset(&account_info[0].build_date_info,0x01,4);
    //account_info[0].div_tol = 0;
    //g_tmp_union.account_all_info.account_info=account_info[0];
    //account_fl_write(&g_tmp_union.account_all_info,0);
}


