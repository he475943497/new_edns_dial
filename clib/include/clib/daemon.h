/*
 * daemonize.h
 *
 *  Created on: 2012-12-6
 *      Author: ry
 */

#ifndef DAEMONIZE_H_
#define DAEMONIZE_H_

#include	<stdlib.h>

#include <clib/array.h>
#ifdef __cplusplus
extern "C" {
#endif



/*
 * @max_process babysit count
  */
extern void daemon_start(uint process_count);


extern void daemon_stop();
#ifdef __cplusplus
}
#endif

#endif /* DAEMONIZE_H_ */
