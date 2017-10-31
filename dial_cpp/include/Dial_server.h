#ifndef DIAL_SERVER_H
#define DIAL_SERVER_H

#include "Dial.h"
#include "dnsrpc_types.h"
#include "Dial_list.h"
#include <time.h>


using namespace  ::rpc::yamutech::com;




/*
typedef struct Healthgroup_node_info {

	DIAL_LIST_NODE node;
	HealthGroupInfo healthgroup_info;
	std::string 	policyname;
	DialRecord *record_head;

} healthgroup_info_t;


typedef struct Healthpolicy_node_info {
	DIAL_LIST_NODE node;
	HealthPolicyInfo healthpolicy_info;
} healthpolicy_info_t;
*/

 enum dial_type {
    DIAL_TCPPORT = 0,
    DIAL_IMCP = 1,
    DIAL_HTTPGET = 2,
    DIAL_DATABASE = 3,
    DIAL_EXTHTTPGET = 4,
 };

 enum dial_node_type {
    HEALTHGROUP = 0,
    SERVER = 1,
    NGINX = 2,
 };

  enum srv_type {
    XPROXY = 0,
    REDIRECT = 1,
    XFORWARD = 2
  };
  
//ip地址信息结构体
typedef struct ip_addr_str {
	unsigned int version;
	char addr[50];
} ip_info_t;

//dialoption match code信息结构体
typedef struct match_code_info {
	DIAL_LIST_NODE node;
	unsigned int code;
} match_code_t;


//记录节点信息结构体
typedef struct record_node_info {
	DIAL_LIST_NODE node;
	char rid[50];
	ip_info_t ip;
	unsigned int ttl;
	unsigned int priority;
	unsigned char enabled;

} record_info_t;

//nginx server节点信息结构体
typedef struct nginx_srv_node_info {
	DIAL_LIST_NODE node;
	char url[1024];
	int priority;
} nginx_srv_t;


//拨测选项结构体
typedef struct dial_option_info {
	char dest_url[1024];
	char test_method[256];
	char expect_match[256];
	DIAL_LIST_HEAD code_head;
}dial_option_t;

//健康检查策略节点信息结构体
typedef struct Healthpolicy_node_info {
	DIAL_LIST_NODE node;
	char policyname[50];
	unsigned short method;
	unsigned short port;
	unsigned int freq;
	unsigned int times;
	unsigned int passed;
	dial_option_t dial_option;
} healthpolicy_info_t;


//健康检查中拨测策略节点信息结构体
typedef struct pPolicy_node_struct {
	DIAL_LIST_NODE node;
	struct timeval t_insert;
	healthpolicy_info_t *pPolicy;
} pPolicy_node_t;


//拨测健康检查节点信息结构体
typedef struct Healthgroup_node_info {

	DIAL_LIST_NODE node;
	char zonename[200];
	char viewname[50];
	char name[200];
	healthpolicy_info_t *pPolicy;
	//struct timeval t_insert;
	DIAL_LIST_HEAD pPolicy_head;
	DIAL_LIST_HEAD record_head;
	DIAL_LIST_HEAD *precord_head;

} healthgroup_info_t;

//反向代理健康检查节点信息结构体
typedef struct nginxgroup_node_info {

	DIAL_LIST_NODE node;
	char name[200];
	bool enable;
	healthpolicy_info_t *pPolicy;
	//struct timeval t_insert;
	DIAL_LIST_HEAD pPolicy_head;
	DIAL_LIST_HEAD srv_head;
	DIAL_LIST_HEAD *psrv_head;

} nginxgroup_info_t;


//拨测服务器节点信息结构体
typedef struct dial_server_node_info {

	DIAL_LIST_NODE node;
	DialServerType::type srv_tpye;
	ip_info_t ip;
	struct timeval t_insert;

} dial_srv_node_t;

//拨测节点结构体
typedef struct dial_node_info {

	DIAL_LIST_NODE node;
	int type;
	union{
		dial_srv_node_t *srv;
		healthgroup_info_t *healthgroup;
		nginxgroup_info_t *nginxgroup;
	} dial_node;

} dial_node_t;

//拨测服务器配置信息结构体
typedef struct  dial_server_config {
	bool allowed[3];
	int health;
	int delay_weight;
	int lost_weight;
	int count;
	int timeout;
	int interval;
	char dname[200];	

} dial_srv_cfg_t;

//配置信息结构体
typedef struct  dial_config {
	dial_srv_cfg_t srv_cfg;
	int log_level;
	int agent_port;
	int dial_port;
	char agent_ip[26];
	char log_path[100];
	char certificate_file[512];
	

} dial_cfg_t;



class DialHandler : virtual public DialIf {
 public:
  DialHandler() {
    // Your initialization goes here
  }
  
  RetCode::type systemCommand(const SysCommand::type cmdType);
  RetCode::type isPrimary(const bool isPrimary);
  RetCode::type addHealthGroup(const HealthGroupInfo& health, const std::string& policyName);
  RetCode::type delHealthGroup(const HealthGroupInfo& health, const std::string& policyName);
  //RetCode::type delHealthGroup(const HealthGroupInfo& health);
  RetCode::type addHealthRecord(const HealthGroupInfo& health, const DialRecord& rec);
  RetCode::type modHealthRecord(const HealthGroupInfo& health, const DialRecord& rec);
  RetCode::type delHealthRecord(const HealthGroupInfo& health, const DialRecord& rec);
  RetCode::type addHealthPolicy(const HealthPolicyInfo& policy);
  RetCode::type modHealthPolicy(const HealthPolicyInfo& policy);
  RetCode::type delHealthPolicy(const HealthPolicyInfo& policy);
  RetCode::type dialServerConfig(const DialServerType::type typ, const bool allowed);
  RetCode::type addDialServer(const IpAddr& ip, const DialServerType::type typ);
  RetCode::type delDialServer(const IpAddr& ip, const DialServerType::type typ);
  RetCode::type addNginxGroup(const std::string& groupName, const std::string& policyName);
  RetCode::type delNginxGroup(const std::string& groupName, const std::string& policyName);
  //RetCode::type delNginxGroup(const std::string& groupName);
  RetCode::type addNginxServer(const std::string& groupName, const DialNginxServer& server);
  RetCode::type modNginxServer(const std::string& groupName, const DialNginxServer& server);
  RetCode::type delNginxServer(const std::string& groupName, const DialNginxServer& server);
  void heartBeat(HeartBeatState& _return);
  RetCode::type setServerState(const bool enable);

};


void
do_a_dial_healthgroup(healthgroup_info_t *hg);
int
do_a_dial_server(dial_srv_node_t *srv);
void
do_a_dial_nginxgroup(nginxgroup_info_t *ng);
void *
client_reconnect_thread(void *arg);

int 
get_parameters_from_url(char *src,char *resource,char *ip,int *port,bool *https_flag);

void
log_debug_open(int sig);

void
log_debug_close(int sig);

int
sys_log_timer_init();

int
signal_init();



#endif
