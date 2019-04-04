#pragma once

typedef struct event_loop_t event_loop;
typedef struct connection_t connection;
typedef void (*message_callback_pt)(connection *conn);

struct tcpclient_t  {
    int connfd;
    char* ip;
    short port;

    event_loop* loop;
    //message_callback_pt msg_callback;
};


typedef struct tcpclient_t tcpclient;

tcpclient* tcpclient_create(const char* ip, short port);


typedef void (*connected_callback_pt)(connection *conn);

int tcpclient_connect(tcpclient* cli, connected_callback_pt connected_cb, message_callback_pt msg_cb);

int tcpclient_send(tcpclient* cli);

void tcpclient_run(event_loop* loop);