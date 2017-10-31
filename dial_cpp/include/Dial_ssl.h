#ifndef DIAL_SSL_H

#define DIAL_SSL_H
#include <pthread.h>

int
https_get_status_code(char *sendbuf,
									int buflen,
									char *cert_file,
									char *ip,
									int port);

int
https_get_check_result(char *rcvbuf,
								char *sendbuf,
								int snd_buflen,
								int rcv_buflen,
								char *cert_file,
								char *ip,
								int port);

void 
init_openssl();





#endif

