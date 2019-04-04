#pragma once

struct buffer_node_t {
    char* msg;
    int sz;
    struct buffer_node_t* next;

    char* buf;
    int cap;
};
 
 
struct buffer_pool_t {
    struct buffer_node_t* head;  
    int len;
};


struct socket_buffer_t {
    int size;
    int offset;
    struct buffer_node_t* head;
    struct buffer_node_t* tail;

    struct buffer_pool_t*  pool;
};



typedef struct buffer_node_t   buffer_node;
typedef struct socket_buffer_t socket_buffer;
typedef struct buffer_pool_t   buffer_pool;

socket_buffer* socket_buffer_new();
void socket_buffer_free(socket_buffer* sb);
int   buffer_push_data(socket_buffer* sb, char* msg, int sz);
void  buffer_node_release(socket_buffer* sb);
char* buffer_read_spec(socket_buffer* sb, int sz, int* realSz);
char* buffer_read_all(socket_buffer* sb, int* retNum);   

int buffer_get_size(socket_buffer* sb);