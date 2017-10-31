#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mysql/mysql.h>
#include "Dial_common.h"

int
open_db( MYSQL *conn,char *host,int port,char *db_name,char *usr,char *pwd)
{	

	//mysql_init(conn);

	//连接数据库
	if (NULL == mysql_real_connect(conn, host, usr, pwd, db_name,port,NULL,0)) {

		debug_printf(LOG_LEVEL_ERROR,"open_db:mysql_real_connect failed, %u: %s\n", 
							mysql_errno(conn), mysql_error(conn));
		return ERROR;
	}

	return NO_ERROR;		
}


int
my_mysql_query(MYSQL *conn,char *query_cmd)
{

	if(0 != mysql_real_query(conn,query_cmd,strlen(query_cmd))) {

		debug_printf(LOG_LEVEL_ERROR,"mysql_real_query: %u: %s,<%s>\n", 
								mysql_errno(conn), mysql_error(conn),query_cmd);
		return	 ERROR;
	}


	return	 NO_ERROR;
}

int
result_mysql_query(MYSQL *conn,char *cmd)
{
	int ret;
	int num_rows;
	MYSQL_RES *result;
	
	ret = my_mysql_query(conn,cmd);
	if(NO_ERROR != ret) {
		return ret;
	}

	result = mysql_store_result(conn); 
	num_rows= mysql_num_rows(result);
	mysql_free_result(result);
	//mysql_close(conn);

	if(num_rows <= 0)
		return ERROR;
	else
		return NO_ERROR;

}




