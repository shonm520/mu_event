#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "listener.h"
#include "servermanager.h"
#include "event.h"
#include "logger.h"
#include "connection.h"
#include "event_loop.h"



extern event_loop *g_loops[];


inet_address addr_create(const char *ip, int port)
{
	if (ip == NULL)
		debug_quit("addr_create failed, ip is null, file: %s, line: %d", __FILE__, __LINE__);
	
	inet_address new_addr;

	memset(&new_addr.addr, 0, sizeof(new_addr.addr));
	new_addr.addr.sin_family = AF_INET;
	
	if (strcmp(ip, "any") == 0)
		new_addr.addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else if(inet_pton(AF_INET, ip, &new_addr.addr.sin_addr) <= 0)
		debug_quit("inet_pton failed, file: %s, line: %d", __FILE__, __LINE__);
	
	new_addr.addr.sin_port = htons(port);
	return new_addr;
}

void disconnected_callback(connection* conn);

static void event_accept_callback(int listenfd, event* ev, void* arg)
{
    server_manager *manager = (server_manager *)arg;
	inet_address client_addr;
	socklen_t clilen = sizeof(client_addr.addr);
	
	int connfd = accept(listenfd, (struct sockaddr *)&client_addr.addr,	&clilen);
	if (connfd < 0)  {
		int save = errno;
		if (save == EAGAIN || save == ECONNABORTED || save == EINTR
			|| save == EPROTO || save == EPERM || save == EMFILE)
		{
			return;
		}
		else  {
			debug_sys("accept failed, file: %s, line: %d", __FILE__, __LINE__);
		}
	}

	
	//char buff[50];
	//printf("connection from %s, port %d\n",
	//		inet_ntop(AF_INET, &client_addr.addr.sin_addr, buff, sizeof(buff)),
	//		ntohs(client_addr.addr.sin_port));

	fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL) | O_NONBLOCK);

    static int i = 0;
	if (i == manager->loop_num)
		i = 0;
    
    event_loop* loop = g_loops[i++];

    if (manager->loop_num == 0)  {     //如果没有开启线程则用主线程的
        loop = manager->loop;
    }
	
	connection *conn = connection_create(loop, connfd, manager->msg_callback);      //后面的参数是指有消息时的用户回调
	if (conn == NULL)  {
		debug_quit("create connection failed, file: %s, line: %d", __FILE__, __LINE__);
	}
	
	if (manager->new_connection_callback)
        conn->connected_cb = manager->new_connection_callback;
        connection_established(conn);

    conn->disconnected_cb = disconnected_callback;
}


#define ERR_SOCKET 1
#define ERR_BIND   2
#define ERR_LISTEN 3
#define ERR_EVENT  4

listener* listener_create(server_manager* manager, inet_address ls_addr,
                         message_callback_pt msg_cb, connection_callback_pt new_con_cb)
{
    listener* ls = (listener*)malloc(sizeof(listener));
    if (ls == NULL)  {
        debug_ret("create listener failed, file: %s, line: %d", __FILE__, __LINE__);
        return NULL;
    }

    ls->listen_addr = ls_addr;

    manager->msg_callback = msg_cb;
    manager->new_connection_callback = new_con_cb;

    int bOk = -1;
    event* ls_event = NULL;
    int listen_fd;
    do {
        listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);     //创建非阻塞套接字 
        if (listen_fd < 0)  {
            bOk = ERR_SOCKET;
            break;
        }

        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        int ret = bind(listen_fd, (struct sockaddr *)&ls_addr.addr, sizeof(ls_addr.addr));
        if (ret < 0)  {
            bOk = ERR_BIND;
            break;
        }

        ret = listen(listen_fd, SOMAXCONN);
        if (ret < 0)  {
            bOk = ERR_LISTEN;
            break;
        }

        ls_event = event_create(listen_fd, EPOLLIN | EPOLLPRI,
                                    event_accept_callback, manager, NULL, NULL);       //后面参数是读写回调及其参数
        if (ls_event == NULL)  {
            bOk = ERR_EVENT;
            break;
        }

        bOk = 0;
    } while(0);

    if (bOk != 0)  {
        debug_ret("create listener failed, error code is %d, file: %s, line: %d", bOk, __FILE__, __LINE__);
        if (listen_fd > 0)  {
            close(listen_fd);
        }
        free(ls);
        return NULL;
    }  
    else  {
        event_add_io(manager->loop->epoll_fd, ls_event);
    }
	
}

void disconnected_callback(connection* conn)
{
    
}