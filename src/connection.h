
#pragma once

typedef struct connection_t connection;

typedef void (*message_callback_pt) (connection* conn);

typedef struct event_t event;

typedef struct event_loop_t event_loop; 

typedef struct socket_buffer_t socket_buffer;
typedef struct buffer_pool_t   buffer_pool;

struct connection_t  {
    int fd;
    event* conn_event;    //清理阶段和改变事件时用到
    message_callback_pt message_callback;

    socket_buffer*  buf_socket_read;
    socket_buffer*  buf_socket_write;

};


connection* connection_create(event_loop* loop, int fd, message_callback_pt msg_cb);
void connection_free(connection* conn);
void connection_send(connection *conn, char *buf, size_t len);


