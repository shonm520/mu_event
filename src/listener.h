#pragma once
#include <sys/socket.h>
#include <netinet/in.h>



/* 封装的IPv4地址和port */
typedef struct inet_addr_t {
	struct sockaddr_in addr;
} inet_address;

inet_address addr_create(const char *ip, int port);

typedef struct connection_t connection;

typedef void (*connection_callback_pt)(connection *conn);
typedef void (*message_callback_pt)(connection *conn);


typedef struct listener_t listener;

struct listener_t  {
    inet_address listen_addr;
    //event* ls_event;
};

typedef struct server_manager_t server_manager;

listener* listener_create(server_manager* manager, inet_address ls_addr,
                         message_callback_pt msg_cb, connection_callback_pt new_con_cb);

void listener_free(listener* ls);