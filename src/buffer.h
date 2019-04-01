#pragma once

struct buffer_node_t {
    char* msg;
    int sz;
    struct buffer_node_t* next;
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
int   push_buffer(socket_buffer* sb, char* msg, int sz);
void  release_free_node(socket_buffer* sb);
char* read_buffer(socket_buffer* sb, int sz);
char* readall(socket_buffer* sb, int* retNum);   

int get_buffer_size(socket_buffer* sb);