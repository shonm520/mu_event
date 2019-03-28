#include <unistd.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "connection.h"
#include "event_loop.h"
#include "logger.h"
#include "event.h"

#define mu_malloc malloc
#define mu_free   free

static void event_readable_callback(int fd, event* ev, void* arg)
{
    connection* conn = (connection*)arg;
    int size = 0;
    if (ioctl(fd, FIONREAD, &size) < 0)
		debug_sys("file: %s, line: %d", __FILE__, __LINE__);	/* exit */

    char buf[1024] = {0};
	ssize_t	n = read(fd, buf, size);

    if (n > 0)  {
        if (conn->readable_callback)  {
            conn->readable_callback(conn);
        }
    }
    else if (n == 0)  {        
        printf("closed!!! \n");
        connection_free(conn);    //如不关闭一直会触发
    }
    else  {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		debug_sys("file: %s, line: %d", __FILE__, __LINE__);
    }

    printf("recv %s\n", buf);
}

static void event_writable_callback(int fd, event* ev, void* arg)
{
    connection* conn = (connection*)arg;
}

connection* connection_create(event_loop* loop, int connfd, connection_callback_pt read_cb)
{
    connection* conn = (connection* )mu_malloc(sizeof(connection));
    if (conn == NULL)  {
        debug_ret("file: %s, line: %d", __FILE__, __LINE__);
        return NULL;
    }

    conn->fd = connfd;
    conn->readable_callback = read_cb;

    event* ev = (event*)event_create(connfd,  EPOLLIN | EPOLLPRI, event_readable_callback, 
                            conn, event_writable_callback, conn);
    if (ev == NULL)  {
        debug_ret("file: %s, line: %d", __FILE__, __LINE__);
        mu_free(conn);
        return NULL;
    }

    conn->conn_event = ev;
    event_add_io(loop->epoll_fd, ev);

    return conn;    
}

void connection_free(connection* conn)
{
    event_free(conn->conn_event);
    mu_free(conn);
}


void connection_send(connection *conn, char *buf, size_t len)
{
    send(conn->fd, buf, len, 0);
}