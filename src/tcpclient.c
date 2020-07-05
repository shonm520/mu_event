#include <sys/epoll.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tcpclient.h"
#include "config.h"
#include "event_loop.h"
#include "event.h"
#include "connection.h"
#include "logger.h"


tcpclient* tcpclient_create(const char* ip, short port)
{
    tcpclient* cli = (tcpclient*)mu_malloc(sizeof(tcpclient));
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);  
    if (socket_fd < 0)  {
        debug_ret("create socket failed, file: %s, line: %d", __FILE__, __LINE__);
        return NULL;
	}
    cli->connfd = socket_fd;
    cli->ip = (char*)ip;
    cli->port = port;

    signal(SIGPIPE, SIG_IGN);

    return cli;
}

static void default_disconnect_callback(connection* conn)
{
    printf("client disconnected#######\n\n");
}


int tcpclient_connect(tcpclient* cli, connected_callback_pt connected_cb, message_callback_pt msg_cb)
{
    struct sockaddr_in server_address;
    memset( &server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton( AF_INET, cli->ip, &server_address.sin_addr);
    server_address.sin_port = htons(cli->port);

    int ret = connect(cli->connfd, (struct sockaddr*)&server_address, sizeof(server_address));
    if (ret == -1)  {
        debug_sys("connect failed, file: %s, line: %d", __FILE__, __LINE__);
        return -1;
    }

    connection *conn = connection_create(cli->connfd, msg_cb);      //后面的参数是指有消息时的用户回调
	if (conn == NULL)  {
		debug_quit("create connection failed, file: %s, line: %d", __FILE__, __LINE__);
	}
    conn->disconnected_cb = default_disconnect_callback;

    connected_cb(conn);
    return 0;
}



void tcpclient_run(event_loop* loop)
{
    event_loop_run(loop);
}

