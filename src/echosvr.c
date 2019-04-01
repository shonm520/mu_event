

#include <stdio.h>

#include "servermanager.h"
#include "connection.h"
#include "listner.h"
#include "buffer.h"
#include "config.h"

void request_handler(connection *conn)
{
	//static char buf[] = "HTTP/1.1 200 0K\r\nConnection:keep-alive\r\nContent-length:4\r\n\r\n"
	//					"this is a test";

	//static char buf[] = "HTTP/1.0 200 OK\r\n";
	
	/* 发送HTTP响应 */
	//connection_send(conn, buf, sizeof(buf) - 1);

	/* 清空输入缓冲区 */
	//array_clear(conn->input_buffer);

	//connection_free(conn);
    int size = 0;
    char* msg = readall(conn->buf_socket_read, &size);
    printf("read all : %s, %d\n", msg, size);

    char buf[] = "123456";
    connection_send(conn, buf, sizeof(buf) - 1);
}

void new_conn_cb(connection* conn)
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
    if (thread_num <= 0)  {
        thread_num = MAX_LOOP;
    }
    printf("port, thread_num is %d, %d \n", port, thread_num);

	server_manager *manager = server_manager_create(port, thread_num);
	inet_address addr = addr_create("any", port);
	listener_create(manager, addr, request_handler, new_conn_cb);
	server_manager_run(manager);

	return 0;
}


