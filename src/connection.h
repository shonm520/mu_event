
#pragma once

typedef struct connection_t connection;

typedef void (*connection_callback_pt) (connection* conn);

typedef struct event_t event;

typedef struct event_loop_t event_loop; 

typedef struct socket_buffer_t socket_buffer;
typedef struct buffer_pool_t   buffer_pool;

struct connection_t  {
    int fd;
    event* conn_event;    //为了在清理阶段一起销毁,处理这个作用，其实完全没有必要包含在这里

    connection_callback_pt readable_callback;

    //buffer_pool*    buf_pool_read;
    socket_buffer*  buf_socket_read;

};


connection* connection_create(event_loop* loop, int fd, connection_callback_pt read_cb);
void connection_free(connection* conn);
void connection_send(connection *conn, char *buf, size_t len);


