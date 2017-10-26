/**
 *Author:           wuyangchun
 *Date:             2012-06-04
 *Description:      epoll的简单封装，只适用于特定的场合, 性能不是最优
 *
 *Notice:           目前只有xforward项目用
 **/


#ifndef CLIB_EPOLL_H_
#define CLIB_EPOLL_H_

#include <clib/types.h>
#include <sys/epoll.h>

typedef struct _epoll epoll_t;

/*
 *@brief        发送tcp数据
 *@param in     ip_version    IPV4或者IPV6
 *@param in     socket        socket描述符
 *@param in     user_data     epoll_add传入的数据
 *@return       -1表示失败
 *
 @notice        如果callbacke函数返回<0，epoll会自动将该socket移除
 */
typedef int (*epoll_callback_func)(uint8_t ip_version, int socket, void *user_data);

/*
 *@brief        新建一个epoll对象
 *@param in     socket_count    支持的最大socket数量
 *@return       NULL 表示失败
 */
epoll_t* epoll_new(int socket_count);

/*
 *@brief        向epoll中添加socket
 *@param in     epoll
 *@param in     ip_version      IPV4或者IPV6
 *@param in     socket          要添加到epoll的socket描述符
 *@param in     event
 *             EPOLLIN
                      The associated file is available for read(2) operations.

               EPOLLOUT
                      The associated file is available for write(2) operations.

               EPOLLRDHUP
                      Stream  socket peer closed connection, or shut down writing half of connection.  (This flag is especially
                      useful for writing simple code to detect peer shutdown when using Edge Triggered monitoring.)

               EPOLLPRI
                      There is urgent data available for read(2) operations.

               EPOLLERR
                      Error condition happened on the associated file descriptor.  epoll_wait(2)  will  always  wait  for  this
                      event; it is not necessary to set it in events.

               EPOLLHUP
                      Hang up happened on the associated file descriptor.  epoll_wait(2) will always wait for this event; it is
                      not necessary to set it in events.

               EPOLLET
                      Sets the Edge Triggered behaviour for the associated file descriptor.  The default behaviour for epoll is
                      Level  Triggered. See epoll(7) for more detailed information about Edge and Level Triggered event distri-
                      bution architectures.

               EPOLLONESHOT (since kernel 2.6.2)
                      Sets the one-shot behaviour for the associated file descriptor.  This means that after an event is pulled
                      out  with epoll_wait(2) the associated file descriptor is internally disabled and no other events will be
                      reported by the epoll interface. The user must call epoll_ctl(2) with EPOLL_CTL_MOD to re-enable the file
                      descriptor with a new event mask.
 *@param in     callback        socket可读时的回调函数
 *@param in     user_data       自定义数据，epoll在调用callback时会传入这个指针
 *@return       -1 表示失败
 */
int epoll_add(epoll_t* epoll, uint8_t ip_version, int socket, int event,
        epoll_callback_func callback, void *user_data);

/*
 *@brief        从epoll中删除一个socket
 *@param in     socket      要移除的socket
 *@return       -1 表示失败
 */
int epoll_remove(epoll_t* epoll, int socket);

/*
 *@brief        启动epoll循环
 *@param in     epoll
 *@param in     running     控制循环终止的变量
 *@return       -1 表示失败
 */
int epoll_start(epoll_t* epoll, volatile bool* running);


/*
 *@brief        释放epoll
 *@param in     epoll       要释放的epoll
 *@return       -1 表示失败
 */
int epoll_free(epoll_t *epoll);


#endif /* EPOLL_H_ */
