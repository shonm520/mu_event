#include <unistd.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include "connection.h"
#include "event_loop.h"
#include "logger.h"
#include "event.h"
#include "buffer.h"
#include "config.h"
#include "ring_buffer.h"


static void handle_close(connection* conn);
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

    event* ev = (event*)event_create(connfd,  EPOLLIN | EPOLLPRI, event_readable_callback, 
                            conn, event_writable_callback, conn);
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


static int read_buffer1(int fd, connection* conn)       //使用了readv但是好像并没有提高效率，不过使用了栈上数据，避免了malloc，free
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
            debug_ret("read n < 0, file: %s, line: %d", __FILE__, __LINE__);
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

static int read_buffer2(int fd, connection* conn)         //另一种读数据方式
{
    int size = 0;
    if (ioctl(fd, FIONREAD, &size) < 0)  {
        debug_ret("ioctl failed, file: %s, line: %d", __FILE__, __LINE__);	
    }
    int ret = size;
    char* buf = (char*)mu_malloc(size);
    do  {
        ssize_t	real_n = read(fd, buf, size);
        if (real_n == 0)  { 
            ret = 0;
            break;
        }
        else if (real_n < 0)  {
            if (errno == EAGAIN || errno == EWOULDBLOCK)  {
                ret = -1;
                break;
            }
            else  {
                ret = -1;
                debug_ret("read n < 0, file: %s, line: %d", __FILE__, __LINE__);
                break;
            }
        }
        buffer_push_data(conn->buf_socket_read, buf, real_n);
        size -= real_n;
    } 
    while(size > 0);
    mu_free(buf);
    return ret;
}

static int read_buffer3(int fd, connection* conn)         //另一种读数据方式
{
    int size = 0;
    if (ioctl(fd, FIONREAD, &size) < 0)  {
        debug_ret("ioctl failed, file: %s, line: %d", __FILE__, __LINE__);	
    }
    int ret = size;
    char* buf = (char*)mu_malloc(size);
    do  {
        ssize_t	real_n = read(fd, buf, size);
        if (real_n == 0)  { 
            ret = 0;
            break;
        }
        else if (real_n < 0)  {
            if (errno == EAGAIN || errno == EWOULDBLOCK)  {
                ret = -1;
                break;
            }
            else  {
                ret = -1;
                debug_ret("read n < 0, file: %s, line: %d", __FILE__, __LINE__);
                break;
            }
        }
        ring_buffer_push_data(conn->ring_buffer_read, buf, real_n);
        size -= real_n;
    } 
    while(size > 0);
    mu_free(buf);
    return ret;
}


static void event_readable_callback(int fd, event* ev, void* arg)
{
    connection* conn = (connection*)arg;
    int nread = read_buffer1(fd, conn);

    if (nread > 0 && conn->message_callback)  {
        conn->message_callback(conn);
    }
    else if(nread == 0)  {
        handle_close(conn);
    }
}

static void event_writable_callback(int fd, event* ev, void* arg)
{
    connection* conn = (connection*)arg;
    int size = 0;
    char* msg = buffer_read_all(conn->buf_socket_write, &size);
    if (size > 0)  {
        int n = send(conn->connfd, msg, size, 0);
        mu_free(msg);  
    }
    int left = buffer_get_size(conn->buf_socket_write);
    if (left == 0)  {        //缓存区数据已全部发送，则关闭发送消息
        event_disable_writing(conn->conn_event);
        if (conn->state == State_Closing)  {
            connection_free(conn);    //如不关闭一直会触发
            conn->state = State_Closed;
        }
    }
    //printf("write buf is %d !!! \n", size);
}

static void handle_close(connection* conn)
{
    printf("handle_close!!! \n");
    connection_disconnected(conn);
}

void connection_established(connection* conn)
{
    if (conn->connected_cb)
        conn->connected_cb(conn);
}

void connection_disconnected(connection* conn)
{
    if (conn->disconnected_cb)  {
        conn->disconnected_cb(conn);
    }

    conn->state = State_Closing;
    if (buffer_get_size(conn->buf_socket_write) > 0)   {     //收到对方关闭写的通知时，如果缓冲区还有数据要发送则等数据发送完毕后再关闭socket
        event_enable_writing(conn->conn_event); 
    }
    else  {
         connection_free(conn);    //如不关闭一直会触发
    }
}

void connection_free(connection* conn)
{
    event_free(conn->conn_event);
    if (conn->buf_socket_read)  {
        socket_buffer_free(conn->buf_socket_read);
    }
    if (conn->buf_socket_write)  {
        socket_buffer_free(conn->buf_socket_write);
    }

    if (conn->ring_buffer_read)  {
        ring_buffer_free(conn->ring_buffer_read);
    }
    if (conn->ring_buffer_write)  {
        ring_buffer_free(conn->ring_buffer_write);
    }
    
    
    mu_free(conn);
}


void connection_send(connection *conn, char *buf, size_t len)
{
    if (conn->buf_socket_write->size == 0)  {                //缓冲区为空直接发送
        int ret = send(conn->connfd, buf, len, 0);
    }
    else  {
        printf("connection_send %d\n", len);
        buffer_push_data(conn->buf_socket_write, buf, len);
        event_enable_writing(conn->conn_event);              //须开启才能发送
    }
}

int connection_send_echo_buffer(connection *conn)
{
    if (ring_buffer_readable_bytes(conn->ring_buffer_write) == 0)  {                //缓冲区为空直接发送
        int len = ring_buffer_readable_bytes(conn->ring_buffer_read);
        char* msg = ring_buffer_readable_start(conn->ring_buffer_read);
        int ret = send(conn->connfd, msg, len, 0);
        ring_buffer_release_bytes(conn->ring_buffer_read, len);
        return len;
    }
    else  {
        int len = ring_buffer_readable_bytes(conn->ring_buffer_read);
        char* msg = ring_buffer_readable_start(conn->ring_buffer_read);
        ring_buffer_push_data(conn->ring_buffer_write, msg, len);
        ring_buffer_release_bytes(conn->ring_buffer_read, len);
        event_enable_writing(conn->conn_event);              //须开启才能发送

        printf("connection_send %d\n", len);
        return len;
    }
}


int connection_send_buffer(connection *conn)
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
                event_enable_writing(conn->conn_event);              //须开启才能发送
                return 1;
            }
            else  {
                return 0;
            }
        }
    }
    return -1;
}