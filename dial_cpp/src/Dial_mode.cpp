#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <error.h>
#include <pcre.h>
#include <pthread.h>




#include "Dial_server.h"
#include "Dial_icmp.h"
#include "Dial_common.h"
#include "Dial_ssl.h"
#include "Dial_sql.h"



#define SSL_CERTIFICATE_FILE		"/root/mycert.pem"

//#define ERROR 	-1
//#define NO_ERROR 		0
#define HTTPGET_PORT 		80
#define PATTERN_STR  	".*?%s.*?"
#define OVECCOUNT	30

int
check_pattern(
	char	*param,
	char	*pattern)
{
	int 		rtn = ERROR;
	pcre        *re;
    const char      *error;
	int         erroffset;
    int         ovector[OVECCOUNT];
	
	//编译表达式
	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);

	//错误处理
	if (re == NULL) {
		
	        debug_printf(LOG_LEVEL_ERROR,"check_pattern:PCRE compilation failed at offset %d: %s\n", erroffset, error);
	        return ERROR;
    }


	//执行表达式
	rtn = pcre_exec(re, NULL, param, strlen(param), 0, 0, ovector, 30);
	//错误处理
	if (rtn < 0) {
		
        if (rtn == PCRE_ERROR_NOMATCH){
			
		  	debug_printf(LOG_LEVEL_BASIC,"check_pattern:no match ...\n");
        }else{
        
          	debug_printf(LOG_LEVEL_BASIC,"check_pattern:match error %d\n", rtn);
        }
		goto EXIT;
  	}
	
	rtn = NO_ERROR;
EXIT:
	//释放编译表达式
	free(re);
	return rtn;
}

int
create_pattern(char *str,char *pattern)
{
	char tmp[1024] = {0};

	if(NULL == str || NULL == pattern) {
		debug_printf(LOG_LEVEL_ERROR,"create_pattern:invalid parameters\n");
		return ERROR;
	}

	sprintf(pattern,PATTERN_STR,str);

	return NO_ERROR;

}



int
create_tcp_client_socket_fd()
{
        struct timeval  tm;
        int                             n_reuseraddr = 1;
        int                             iret = -1;
        int sock_fd;


        //sock
        if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {

                debug_printf(LOG_LEVEL_ERROR,"create_tcp_client_socket_fd:create socket error = %d\n", sock_fd);
                return ERROR;
        }

        tm.tv_sec  = 0;
        tm.tv_usec = 1000*50;
        //发送超时
        iret = setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, &tm, sizeof(struct timeval));
        if(0 != iret){
            debug_printf(LOG_LEVEL_ERROR,"create_tcp_client_socket_fd:setsockopt SO_SNDTIMEO faile\n");
			close(sock_fd);
			return ERROR;
        }


  		tm.tv_sec  = 0;
    	tm.tv_usec = 1000*100;
        //接收超时
        iret = setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tm, sizeof(struct timeval));
        if(0 != iret){
            debug_printf(LOG_LEVEL_ERROR,"create_tcp_client_socket_fd:setsockopt SO_RCVTIMEO failed\n");
			close(sock_fd);
			return ERROR;
        }

        //允许重用本地地址和端口
        iret = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&n_reuseraddr, sizeof(int));
        if(0 != iret){
            debug_printf(LOG_LEVEL_ERROR,"create_tcp_client_socket_fd:setsockopt SO_REUSEADDR failed\n");
			close(sock_fd);
			return ERROR;
        }

        return sock_fd;
}

int
build_tcp_connection(int fd,
						int  port,
						char *ip)
{
	 struct sockaddr_in addrsrv;

        addrsrv.sin_family = AF_INET;
        addrsrv.sin_port = htons(port);
	addrsrv.sin_addr.s_addr = inet_addr(ip);

	if(-1 ==connect(fd,(struct sockaddr *)&addrsrv,sizeof(struct sockaddr))){
                debug_printf(LOG_LEVEL_ERROR,"build_tcp_connection:build_tcp_connection:connect  failed,port=%d,ip=%s\n",port,ip);
                return ERROR;
        }

	return NO_ERROR;
}

int judge_http_status_code(int code)
{
	if(code/100 == 2 || code/100 == 3)
		return NO_ERROR;
	else
		return ERROR;
}

int
get_db_param_from_url(char *ip,char *url,char *host,int *port,char *db_name,char *usr,char *pwd)
{
	char *pstart = NULL;
	char *phost = host;
	char pport[10] = {0};
	int i = 0;
	
	if(NULL == url) {
		debug_printf(LOG_LEVEL_ERROR,"get_db_param_from_url:invalid parameter,db url is NULL\n");	
		return ERROR;
	}
	
	pstart = strstr(url,"mysql://");
	if(NULL == pstart) {
		debug_printf(LOG_LEVEL_ERROR,"get_db_param_from_url:invalid parameter,db url format error\n");
		return ERROR;	
	}else {
		pstart += strlen("mysql://");
		
	}

	//提取host
	while(*pstart != '\0') {
		if(':' == *pstart || '/' == *pstart) {
			break;
		}else {
			*host++ = *pstart++;
		}
	}
	if(*pstart == '\0') {
		debug_printf(LOG_LEVEL_ERROR,"get_db_param_from_url:db url format error,get host failed!\n");
		return ERROR;	
	}
	
	if(!strcmp("$domain",phost))
		strcpy(phost,ip);

	if(':' == *pstart) {
		pstart++;
		//提取port
		while(*pstart != '\0') {
			if('/' == *pstart)
				break;
			else
				pport[i++] = *pstart++;
		}	
		if(*pstart == '\0') {
			debug_printf(LOG_LEVEL_ERROR,"get_db_param_from_url:db url format error,get port failed!\n");
			return ERROR;	
		}

		*port = atoi(pport);
	}else {
		*port = 3306;//mysql
	}
	
	pstart++;
	//提取db name
	while(*pstart != '\0') {
		if('?' == *pstart) {
			break;
		}else {
			*db_name++ = *pstart++;
		}
	}
	if(*pstart == '\0') {
		debug_printf(LOG_LEVEL_ERROR,"get_db_param_from_url:db url format error,get db_name failed!\n");
		return ERROR;	
	}

	pstart++;
	//提取user
	pstart = strstr(pstart,"user=");
	if(NULL == pstart) {
		debug_printf(LOG_LEVEL_ERROR,"get_db_param_from_url:invalid parameter,can not find user name\n");
		return ERROR;
	} else {
		pstart += strlen("user=");
		
		while(*pstart != '\0') {
			if('&' == *pstart) {
				break;
			}else {
				*usr++ = *pstart++;
			}
		}	
		if(*pstart == '\0') {
			debug_printf(LOG_LEVEL_ERROR,"get_db_param_from_url:db url format error,get user failed!\n");
			return ERROR;	
		}
	}

	pstart++;
	//提取pwd
	pstart = strstr(pstart,"password=");
	if(NULL == pstart) {
		debug_printf(LOG_LEVEL_ERROR,"get_db_param_from_url:invalid parameter,can not find password\n");
		return ERROR;
	} else {
		pstart += strlen("password=");
		
		while(*pstart != '\0') {
			if('&' == *pstart) {
				break;
			}else {
				*pwd++ = *pstart++;
			}
		}		
		if(*pstart == '\0') {
			debug_printf(LOG_LEVEL_ERROR,"get_db_param_from_url:db url format error,get pwd failed!\n");
			return ERROR;	
		}
	}

	return NO_ERROR;
}

int
check_httpget_result(dial_option_t *option,char *src,char *pattern,unsigned int code)
{
	int rtn;
	match_code_t *match_code;
	DIAL_LIST_NODE *cur,*head;
	 

	rtn = check_pattern(src,pattern);
	if(NO_ERROR != rtn)
		return ERROR;

	head = &option->code_head.head;

	if(option->code_head.num > 0) {
		list_for_each(cur,head) {		
			match_code = (match_code_t *)cur;
			if(match_code->code == code)
				break;
		}	
		if(cur != head) {
			debug_printf(LOG_LEVEL_DEBUG,"check_httpget_result:code matched!status_code = %d\n",match_code->code);
			return NO_ERROR;
		}else {
			debug_printf(LOG_LEVEL_DEBUG,"check_httpget_result:code not matched!status_code = %d\n",code);
			return ERROR;
		}
			
	}else{
		debug_printf(LOG_LEVEL_DEBUG,"check_httpget_result:no match codes,pass directly!status_code=%d\n",code);
	}


	return NO_ERROR;
}

const char *httpget_buf = 
"GET %s HTTP/1.1\r\n"
"Host:%s\r\n"
"User-Agent: Mozilla/5.0 (Windows NT 6.2; WOW64; rv:40.0) Gecko/20100101 Firefox/40.0\r\n"
"Accept:*/*\r\n"
"Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3\r\n"
"Accept-Encoding: gzip, deflate\r\n"
"\r\n";

int
do_exthttps_get(int fd,
					int port,
					char *ip,
					char *pattern,
					char *snd_buf,
					char *rcv_buf,
					int snd_buf_len,
					int rcv_buf_len,
					dial_option_t *option)
{

	int rtn;
	int rcv_times = 0;
	long size = 0;
	int status_code;
	extern dial_cfg_t g_cfg;


	status_code = https_get_check_result(rcv_buf,snd_buf,snd_buf_len,rcv_buf_len,g_cfg.certificate_file,ip,port);

	//bug_printf(LOG_LEVEL_DEBUG,"do_exthttps_get:rcv_buf = %s\n",rcv_buf);

	rtn = check_httpget_result(option,rcv_buf,pattern,(unsigned int)status_code);
	
	if(NO_ERROR== rtn)
		debug_printf(LOG_LEVEL_DEBUG,"do_exthttps_get:__success!!!\n");	
	else
		debug_printf(LOG_LEVEL_DEBUG,"do_exthttps_get:___failed!!!\n");	

	return rtn;

}

int
do_exthttp_get(int fd,
					char *pattern,
					char *snd_buf,
					char *rcv_buf,
					int snd_buf_len,
					dial_option_t *option)
{
	int rtn;
	int rcv_times = 0;
	long size = 0;
	int status_code;

   	rtn = send(fd, snd_buf,snd_buf_len, 0);
    if(rtn < 0) {
		debug_printf(LOG_LEVEL_ERROR,"do_exthttps_get: send falied,rtn = %d\n",rtn);
		goto EXIT;
    }
	
	do{
		rcv_times++;
		rtn = recv(fd,rcv_buf+size,1024,0);
	    if(rtn<=0){
			debug_printf(LOG_LEVEL_ERROR,"do_exthttps_get: recv falied rtn = %d\n",rtn);
	   	}
		size += rtn;
		
	}while(rtn > 0);
	
	if(size <= 0) {		
		debug_printf(LOG_LEVEL_DEBUG,"do_exthttp_get:bad page,recv failed!!rtn = %d\n",rtn);
		rtn = ERROR;
		goto EXIT;
	} else {
		status_code = atoi(rcv_buf + 9);
		
		rtn = check_httpget_result(option,rcv_buf,pattern,(unsigned int)status_code);
	
	}

EXIT:

	if(NO_ERROR== rtn)
		debug_printf(LOG_LEVEL_DEBUG,"do_exthttp_get:success!!!\n");	
	else
		debug_printf(LOG_LEVEL_DEBUG,"do_exthttp_get:failed!!!\n");	

	return rtn;
}


int
handle_exthttpget_dialing(char *ip,
								char *host,
								dial_option_t *option)
{
	int rtn = 0;
	int fd = 0;
	int http_len = 0;
	char http_buf[300] = {0};
	char *rcv_buf = NULL;
	char pattern[1024] = {0};
	char resource[1024] = {0};
	char url_ip[26] = {0};
	int port = 0;
	bool https_flag;
	extern dial_cfg_t g_cfg;



	rtn = create_pattern(option->expect_match,pattern);
	if(NO_ERROR != rtn)
		return ERROR;

	debug_printf(LOG_LEVEL_DEBUG,"handle_exthttpget_dialing:option->expect_match=%s,option->dest_url=%s,pattern=%s\n",
								option->expect_match,option->dest_url,pattern);

	rtn = get_parameters_from_url(option->dest_url,resource,url_ip,&port,&https_flag);
	if(NO_ERROR != rtn)
		return ERROR;
	if(!strcmp(url_ip,"$domain")) {
		memset(url_ip,0,sizeof(url_ip));
		strcpy(url_ip,ip);
	}
	
	debug_printf(LOG_LEVEL_DEBUG,"handle_exthttpget_dialing:000000000,pid=%d\n",pthread_self());

	if(false == https_flag) {
		
		fd = create_tcp_client_socket_fd();
		if(fd <= 0) {
			debug_printf(LOG_LEVEL_ERROR,"handle_exthttpget_dialing:create fd failed,fd = %d\n",fd);
			return ERROR;
		}

		if(build_tcp_connection(fd,port,ip) < 0) {
			close(fd);
			return ERROR;
		}
	}
	//debug_printf(LOG_LEVEL_DEBUG,"handle_exthttpget_dialing:22222222222222222\n");

	sprintf(http_buf,httpget_buf,resource,host);
	
	http_len = strlen(http_buf);

	rcv_buf = (char *)malloc(1024*1024*10);
	if(NULL == rcv_buf) {
		debug_printf(LOG_LEVEL_ERROR,"handle_exthttpget_dialing: calloc falied!!\n");
		if(true == https_flag)
			close(fd);
		return ERROR;
	}
	memset(rcv_buf,0,1024*1024*10);
	
	debug_printf(LOG_LEVEL_DEBUG,"handle_exthttpget_dialing:3333333333333333333\n");

	if(true == https_flag)
		rtn = do_exthttps_get(fd,port,url_ip,pattern,http_buf,rcv_buf,http_len,1024*1024*10,option);
	else {
		rtn = do_exthttp_get(fd,pattern,http_buf,rcv_buf,http_len,option);
		close(fd);
	}

	
	free(rcv_buf);
	
	//debug_printf(LOG_LEVEL_DEBUG,"handle_exthttpget_dialing:rtn=%d,ip=%s,host=%s\n",rtn,ip,host);
	
	return rtn;
}

int
handle_httpget_dialing(char *ip,
								int port,
								char *resource,
								char *host)

{
	int rtn = 0;
	int fd;
	int http_len = 0;
	int status_code;
	char http_buf[300] = {0};
	char rcv_buf[1024] = {0};

	fd = create_tcp_client_socket_fd();
	if(fd <= 0) {
		debug_printf(LOG_LEVEL_ERROR,"handle_httpget_dialing:create fd failed,fd = %d\n",fd);
		return ERROR;
	}

	if(build_tcp_connection(fd,port,ip) < 0) {
		close(fd);
		return ERROR;
	}

	sprintf(http_buf,httpget_buf,resource,host);

	http_len = strlen(http_buf);
   	rtn = send(fd, http_buf,http_len, 0);
        if(rtn < 0) {
		debug_printf(LOG_LEVEL_ERROR,"handle_httpget_dialing: send falied,rtn = %d\n",rtn);
		close(fd);
		return ERROR;
        }

	rtn = recv(fd,rcv_buf,sizeof(rcv_buf),0);
        if(rtn<=0){
		debug_printf(LOG_LEVEL_ERROR,"handle_httpget_dialing: recv falied rtn = %d\n",rtn);
		close(fd);
		return ERROR;
        }
	close(fd);
	
	status_code = atoi(rcv_buf + 9);

	rtn = judge_http_status_code(status_code);
	debug_printf(LOG_LEVEL_DEBUG,"handle_httpget_dialing:rtn=%d,status_code=%d,ip=%s,host=%s\n",rtn,status_code,ip,host);
	return rtn;
}

int
handle_httpsget_dialing(char *ip,
								int port,
								char *resource,
								char *host)
{
	int rtn = 0;
	//int fd;
	int http_len = 0;
	int status_code;
	char http_buf[300] = {0};
	char rcv_buf[1024] = {0};
	extern dial_cfg_t g_cfg;
	
	/*fd = create_tcp_client_socket_fd();
	if(fd <= 0) {
		debug_printf(LOG_LEVEL_ERROR,"handle_httpsget_dialing:create fd failed,fd = %d\n",fd);
		return ERROR;
	}

	if(build_tcp_connection(fd,port,ip) < 0) {
		close(fd);
		return ERROR;
	}*/

	sprintf(http_buf,httpget_buf,resource,host);

	http_len = strlen(http_buf);
	status_code = https_get_status_code(http_buf,http_len,g_cfg.certificate_file,ip,port);
	//close(fd);
	

	rtn = judge_http_status_code(status_code);
	debug_printf(LOG_LEVEL_DEBUG,"handle_httpget_dialing:rtn=%d,status_code=%d,ip=%s,host=%s\n",rtn,status_code,ip,host);
	return rtn;
}


int
handle_tcp_and_port_dialing(char *ip,
								int port)
{
	int rtn = 0;
	int fd;
	
	fd = create_tcp_client_socket_fd();
	if(fd <= 0) {
		debug_printf(LOG_LEVEL_ERROR,"handle_tcp_and_port_dialing:create fd failed,fd = %d\n",fd);
		return ERROR;
	}

	if(build_tcp_connection(fd,port,ip) < 0) {
		close(fd);
		return ERROR;
	}

	close(fd);
	debug_printf(LOG_LEVEL_DEBUG,"handle_tcp_and_port_dialing:tcp+port dialing success!port=%d,ip=%s\n",port,ip);
	return NO_ERROR;
}


int
handle_icmp_dialing(char *dip)
{
	int rtn = 0;
	int fd;
	//extern int	raw_fd;
	fd = create_client_raw_socket();
	if(fd <= 0) {
		debug_printf(LOG_LEVEL_ERROR,"handle_icmp_dialing:create raw fd failed,fd = %d,errno=%d\n",fd,errno);
		return ERROR;
	}
	

	rtn = sendIcmp(fd,dip);
	if(rtn < 0) {
		debug_printf(LOG_LEVEL_ERROR,"handle_icmp_dialing:send icmp pakect failed,dip=%s\n",dip);
		close(fd);
		return ERROR;
	}

	close(fd);
	return NO_ERROR;
}

int
handle_db_dialing(char *ip,char *url,char *db_cmd)
{
	char host[100] = {0};
	char db_name[50] = {0};
	char usr[50] = {0};
	char pwd[50] = {0};
	int port = 0;
	int rtn = 0;
	MYSQL *conn;

	if(NULL == ip || NULL == url || NULL == db_cmd) {
		debug_printf(LOG_LEVEL_ERROR,"handle_db_dialing:invalid parameters\n");
		goto EXIT;
	}else{
		debug_printf(LOG_LEVEL_BASIC,"handle_db_dialing:ip=%s,url=%s,cmd=%s\n",ip,url,db_cmd);
	}

	rtn = get_db_param_from_url(ip,url,host,&port,db_name,usr,pwd);
	if(NO_ERROR != rtn)
		goto EXIT;

	conn = mysql_init(NULL);
	if(NULL == conn) {
		debug_printf(LOG_LEVEL_BASIC,"handle_db_dialing:mysql_init failed!!\n");
		goto EXIT;
	}
	
	rtn = open_db(conn,host,port,db_name,usr,pwd);
	if(NO_ERROR != rtn) {
		goto EXIT;
	}

	rtn = result_mysql_query(conn,db_cmd);
	mysql_close(conn);

EXIT:	
	if(NO_ERROR== rtn)
		debug_printf(LOG_LEVEL_DEBUG,"handle_db_dialing:success!!!host=%s,usr=%s,pwd=%s,db_name=%s,port=%d\n",host,usr,pwd,db_name,port);
	else
		debug_printf(LOG_LEVEL_DEBUG,"handle_db_dialing:failed!!!host=%s,usr=%s,pwd=%s,db_name=%s,port=%d\n",host,usr,pwd,db_name,port);
	
	return rtn;
}

