#pragma once 


struct server_manager_t {
    int epoll_fd;
    int listen_port;
};


typedef struct server_manager_t server_manager;


server_manager* server_manager_create();
void server_manager_run(server_manager* manager);

 