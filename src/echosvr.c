

#include <stdio.h>

#include "servermanager.h"
#include "connection.h"
#include "listner.h"
#include "buffer.h"

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

int main()
{
	server_manager *manager = server_manager_create();
	inet_address addr = addr_create("any", 2019);
	listener_create(manager, addr, request_handler, new_conn_cb);
	server_manager_run(manager);

	return 0;
}


