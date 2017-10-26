#ifndef DIAL_SQL_H
#define DIAL_SQL_H

#include <mysql/mysql.h>


int
open_db( MYSQL *conn,char *host,int port,char *db_name,char *usr,char *pwd);

int
my_mysql_query(MYSQL *conn,char *query_cmd);

int
result_mysql_query(MYSQL *conn,char *cmd);

#endif
