#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include "listner.h"
#include "servermanager.h"
#include "event.h"
#include "logger.h"
#include "connection.h"
#include "event_loop.h"



extern event_loop *g_loops[4];


inet_address addr_create(const char *ip, int port)
{
	if (ip == NULL)
		debug_quit("file: %s, line: %d", __FILE__, __LINE__);
	
	inet_address new_addr;

	memset(&new_addr.addr, 0, sizeof(new_addr.addr));
	new_addr.addr.sin_family = AF_INET;
	
	if (strcmp(ip, "any") == 0)
		new_addr.addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else if(inet_pton(AF_INET, ip, &new_addr.addr.sin_addr) <= 0)
		debug_quit("file: %s, line: %d", __FILE__, __LINE__);
	
	new_addr.addr.sin_port = htons(port);
	return new_addr;
}


static void event_accept_callback(int listenfd, event* ev, void* arg)
{
    listener *ls = (listener *)arg;
	inet_address client_addr;
	socklen_t clilen = sizeof(client_addr.addr);
	
	int connfd = accept(listenfd, (struct sockaddr *)&client_addr.addr,	&clilen);
	if (connfd < 0)  {
		debug_ret("file: %s, line: %d", __FILE__, __LINE__);
		
		int save = errno;
		if (save == EAGAIN || save == ECONNABORTED || save == EINTR
			|| save == EPROTO || save == EPERM || save == EMFILE)
		{
			return;
		}
		else  {
			debug_sys("file: %s, line: %d", __FILE__, __LINE__);
		}
	}

	/* 打印客户端地址信息 */
	char buff[50];
	printf("connection from %s, port %d\n",
			inet_ntop(AF_INET, &client_addr.addr.sin_addr, buff, sizeof(buff)),
			ntohs(client_addr.addr.sin_port));

	fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL) | O_NONBLOCK);

	/* epoll是线程安全的 */
    static int i = 0;
	if (i == 4)
		i = 0;
	
	connection *conn = connection_create(g_loops[i], connfd, ls->readable_callback);
	if (conn == NULL)  {
		debug_quit("file: %s, line: %d", __FILE__, __LINE__);
	}
	i++;
	
	/* 用户回调函数 */
	if (ls->new_connection_callback)
		ls->new_connection_callback(conn);
}


listener* listener_create(server_manager* manager, inet_address ls_addr,
                         connection_callback_pt read_cb, connection_callback_pt new_con_cb)
{
    listener* ls = (listener*)malloc(sizeof(listener));
    if (ls == NULL)  {
        debug_ret("file: %s, line: %d", __FILE__, __LINE__);
        return NULL;
    }

    ls->listen_addr = ls_addr;
    ls->readable_callback = read_cb;
    ls->new_connection_callback = new_con_cb;

    int bOk = -1;
    event* ls_event = NULL;
    int listen_fd;
    do {
        /* 创建非阻塞套接字 */
        listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        if (listen_fd < 0)  {
            break;
        }

        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        int ret = bind(listen_fd, (struct sockaddr *)&ls_addr.addr, sizeof(ls_addr.addr));
        if (ret < 0)  {
            break;
        }

        ret = listen(listen_fd, SOMAXCONN);
        if (ret < 0)  {
            break;
        }

        ls_event = event_create(listen_fd, EPOLLIN | EPOLLPRI,
                                    event_accept_callback, ls, NULL, NULL);       //后面参数是读写回调
        if (ls_event == NULL)  {
            break;
        }

        bOk = 0;
    } while(0);

    if (bOk == -1)  {
        debug_ret("file: %s, line: %d", __FILE__, __LINE__);
        if (listen_fd > 0)  {
            close(listen_fd);
        }
        free(ls);
        return NULL;
    }  
    else  {
        event_add_io(manager->epoll_fd, ls_event);
    }
	
}