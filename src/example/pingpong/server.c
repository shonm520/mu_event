#include<stdio.h>
#include "config.h"
#include "connection.h"
#include "servermanager.h"
#include "listener.h"
#include "buffer.h"
#include "timer.h"

static void onMessage(connection *conn)
{
    //int size = 163840;
    //char* msg = buffer_read_all(conn->buf_socket_read, &size);
    //char* msg = mu_malloc(size);
    //printf("read all : %s, %d\n", msg, size);

    //printf("svr read all : %d\n", size);

    connection_send_echo_buffer(conn);
    
}

static void ondisconnect(connection* conn)
{
    printf("svr disconnected\n\n");
}

static void onConnection(connection* conn)
{
    printf("connected!!!!\n");
    conn->disconnected_cb = ondisconnect;
}

static void timer_cb(void* arg)
{
    printf("timer tick!!!!\n");
}


int main(int argc, char* argv[])  
{
    int port = DEFAULT_PORT;
    int thread_num = MAX_LOOP;
    if (argc >= 2)
        port = atoi(argv[1]);
    if (argc >= 3)
        thread_num = atoi(argv[2]);

    printf("port, thread_num is %d, %d \n", port, thread_num);

	server_manager *manager = server_manager_create(port, thread_num);
	inet_address addr = addr_create("any", port);
	listener_create(manager, addr, onMessage, onConnection);

	server_manager_run(manager);

	return 0;
}