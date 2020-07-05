
#include <stdio.h>
#include "servermanager.h"
#include "connection.h"
#include "listener.h"
#include "ring_buffer.h"
#include "config.h"

void onMessage(connection *conn)
{
    int size = 0;
    char* msg = ring_buffer_get_msg(conn->ring_buffer_read, &size);
    printf("read all : %s, %d\n", msg, size);

    char buf[] = "abcd";
    connection_send(conn, buf, sizeof(buf) - 1);

    connection_send(conn, msg, size);
}

void onConnection(connection* conn)
{
    printf("connected!!! ip:port %s:%d\n", conn->client_ip, conn->client_port);
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


