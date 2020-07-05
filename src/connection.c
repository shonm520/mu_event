#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include "connection.h"
#include "event_loop.h"
#include "event.h"
#include "config.h"
#include "ring_buffer.h"
#include "logger.h"


/*
    这里的connection仅仅指客户端连接后建立的socket connnection
*/


static void connection_passive_close(connection* conn);
static void connection_disconnect(connection* conn);
static void event_readable_callback(int fd, event* ev, void* arg);
static void event_writable_callback(int fd, event* ev, void* arg);

connection* connection_create(int connfd, message_callback_pt msg_cb)
{
    connection* conn = (connection* )mu_malloc(sizeof(connection));
    if (conn == NULL)  {
        debug_ret("create connection failed, file: %s, line: %d", __FILE__, __LINE__);
        return NULL;
    }

    memset(conn, 0, sizeof(connection));
    conn->connfd = connfd;
    conn->message_callback = msg_cb;

    //创建连接的时候没有指定EPOLLOUT,是因为在epoll水平触发的情形下指定EPOLLOUT会导致写事件一直被触发(通常写缓冲区是不满的)
    //这里的高效实现方式是有数据时直接调用发送,如果发现发送函数send调用之后缓冲区还有数据,则开启EPOLLOUT事件发送完毕后又关闭事件
    event* ev = (event*)event_create(connfd,  EPOLLIN | EPOLLPRI, event_readable_callback, conn, event_writable_callback, conn);
    if (ev == NULL)  {
        debug_ret("create event failed, file: %s, line: %d", __FILE__, __LINE__);
        mu_free(conn);
        return NULL;
    }

    conn->conn_event = ev;
    
    return conn;    
}


void connection_start(connection* conn, event_loop* loop)
{
    if (! conn->ring_buffer_read)  {
        conn->ring_buffer_read = ring_buffer_new();
    }

    if (! conn->ring_buffer_write)  {
        conn->ring_buffer_write = ring_buffer_new();
    }
    event_add_io(loop->epoll_fd, conn->conn_event);
}


static int read_buffer(int fd, connection* conn)       //使用了readv但是好像并没有提高效率，不过使用了栈上数据，避免了malloc，free
{
    int nread2 = 65536;
    char extrabuf2[nread2];
    struct iovec vec[2];

    char* start = ring_buffer_readable_start(conn->ring_buffer_read);
    int available_bytes = ring_buffer_available_bytes(conn->ring_buffer_read);
    vec[0].iov_base = start;
    vec[0].iov_len = available_bytes;        //一开始时为0，并不读到ring_buffer中去
    vec[1].iov_base = extrabuf2;
    vec[1].iov_len = nread2;

    ssize_t nread = readv(fd, vec, 2);      //最多读iovec结构体指定的长度,而且fd一定是非阻塞的
    if (nread == 0)  {
        return 0;
    }
    else if (nread < 0)  {
        if (errno == EAGAIN || errno == EWOULDBLOCK)  {
            return -1;
        }
        else  {
            debug_msg("read n < 0, fd : %d, file: %s, line: %d", conn->connfd, __FILE__, __LINE__);
            return -1;
        }
    }
    else if (nread <= available_bytes)  {
        conn->ring_buffer_read->end += nread;
        return nread;
    }
    else  {
        conn->ring_buffer_read->end += available_bytes;
        ring_buffer_push_data(conn->ring_buffer_read, extrabuf2, nread - available_bytes);
        return nread;
    }
    return -1;
}


static void event_readable_callback(int fd, event* ev, void* arg)
{
    connection* conn = (connection*)arg;
    int nread = read_buffer(fd, conn);
    if (nread > 0 && conn->message_callback)  {
        conn->message_callback(conn);
    }
    else if(nread <= 0)  {
        connection_passive_close(conn);
    }
}

static void event_writable_callback(int fd, event* ev, void* arg)
{
    int len = 0;
    connection* conn = (connection*)arg;
    char* msg = ring_buffer_get_msg(conn->ring_buffer_write, &len);
    if (msg && len > 0)  {
        int n = send(conn->connfd, msg, len, 0);
        if (n > 0)  {
            ring_buffer_release_bytes(conn->ring_buffer_write, n);
            len = ring_buffer_readable_bytes(conn->ring_buffer_write);
            if (len == 0)  {                  //发送完所有缓冲区数据后要关闭写触发
                event_disable_writing(conn->conn_event);
                if (conn->state == State_Closing)  {
                    connection_free(conn);    //如不关闭一直会触发
                    conn->state = State_Closed;
                }
            }
        }
    }
}

static void connection_passive_close(connection* conn)
{
    //printf("connection_passive_close!!! %d, life time is %d \n", conn->connfd, (int)time(NULL) - conn->time_on_connect);
    connection_disconnect(conn);
}

void connection_established(connection* conn)
{
    if (conn->connected_cb)
        conn->connected_cb(conn);
}

void connection_active_close(connection* conn)
{   
    //printf("active close %d\n", conn->connfd);
    connection_disconnect(conn);
}

void connection_set_disconnect_callback(connection* conn, connection_callback_pt cb)
{
    conn->disconnected_cb = cb;
}

static void connection_disconnect(connection* conn)
{
    conn->state = State_Closing;
    if (ring_buffer_readable_bytes(conn->ring_buffer_write) > 0)   {     //收到对方关闭写的通知时，如果缓冲区还有数据要发送则等数据发送完毕后再关闭socket
        event_enable_writing(conn->conn_event); 
    }
    else  {
        connection_free(conn);    //如不关闭一直会触发
        conn->state = State_Closed;
    }
}

void connection_free(connection* conn)
{
    if (conn->disconnected_cb)  {
        conn->disconnected_cb(conn);
    }

    event_free(conn->conn_event);

    if (conn->ring_buffer_read)  {
        ring_buffer_free(conn->ring_buffer_read);
    }
    if (conn->ring_buffer_write)  {
        ring_buffer_free(conn->ring_buffer_write);
    }
    
    mu_free(conn);
}


void connection_send(connection *conn, char *buf, size_t len)  //外部调用来发送数据
{
    if (ring_buffer_readable_bytes(conn->ring_buffer_write) == 0)  {         //缓冲区为空直接发送
        int ret = send(conn->connfd, buf, len, 0);
    }
    else  {
        printf("connection_send %d\n", len);
        ring_buffer_push_data(conn->ring_buffer_write, buf, len);
        event_enable_writing(conn->conn_event);              //须开启才能发送
    }
}


int connection_send_buffer(connection *conn)                 //外部调用来发送数据
{
    int len = 0;
    char* msg = ring_buffer_get_msg(conn->ring_buffer_write, &len);
    if (msg && len > 0)  {
        int n = send(conn->connfd, msg, len, 0);
        if (n == -1)  {
            return -1;
        }
        if (n > 0)  {
            ring_buffer_release_bytes(conn->ring_buffer_write, n);
            if (n < len)  {       //没有发完全
                event_enable_writing(conn->conn_event);      //须开启才能发送
                return 1;
            }
            else  {
                return 0;
            }
        }
    }
    return -1;
}