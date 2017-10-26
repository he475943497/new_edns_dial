#ifndef DIAL_COMMON_H
#define DIAL_COMMON_H

//#include <syslog.h>
#include "Dial_log.h"




#define NO_ERROR	0
#define ERROR			-1
/*

#ifdef		ALL
#define	debug_printf(fmt,...)	log_printf(DEBUG_LOG_FILE_DIAL,fmt,##__VA_ARGS__)	
#define	config_printf(fmt,...)	log_printf(CONFIG_LOG_FILE_DIAL,fmt,##__VA_ARGS__)	
#elif defined(DEBUG)
#define	debug_printf(fmt,...)	log_printf(DEBUG_LOG_FILE_DIAL,fmt,##__VA_ARGS__)	
#define	config_printf(fmt,...) 
#elif defined(CONF)
#define	config_printf(fmt,...)	log_printf(CONFIG_LOG_FILE_DIAL,fmt,##__VA_ARGS__)	
#define	debug_printf(fmt,...)
#else
#define	debug_printf(fmt,...)
#define	config_printf(fmt,...)
#endif
*/

#define	debug_printf(level,fmt,...)  log_printf(level,LOG_RUN,DEBUG_LOG_FILE_DIAL,fmt,##__VA_ARGS__)

#define	cfg_debug_printf(level,fmt,...)  log_printf(level,LOG_CONFIG,CONFIG_LOG_FILE_DIAL,fmt,##__VA_ARGS__)


#endif
