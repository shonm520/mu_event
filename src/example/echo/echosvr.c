
#include <stdio.h>
#include "servermanager.h"
#include "connection.h"
#include "listener.h"
#include "buffer.h"
#include "config.h"

void onMessage(connection *conn)
{
    int size = 0;
    char* msg = buffer_read_all(conn->buf_socket_read, &size);
    printf("read all : %s, %d\n", msg, size);

    //char buf[] = "123456";
    //connection_send(conn, buf, sizeof(buf) - 1);

    connection_send(conn, msg, size);
}

void onConnection(connection* conn)
{
    printf("connected!!!!\n");
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


