#ifndef DIAL_MODE_
#define DIAL_MODE_
#include "Dial_server.h"

int
handle_tcp_and_port_dialing(char *ip,
								int port);
int
handle_httpget_dialing(char *ip,
								int port,
								char *resource,
								char *host);
int
handle_httpget_dialing2(char *ip,
								int port,
								char *resource,
								char *host,
								dial_option_t *option);
int
handle_httpsget_dialing(char *ip,
								int port,
								char *resource,
								char *host);
int
handle_exthttpget_dialing(char *ip,
								char *host,
								dial_option_t *option);
int
handle_icmp_dialing(char *dip);
int
handle_db_dialing(char *ip,char *url,char *db_cmd);

int
create_tcp_client_socket_fd();

#endif