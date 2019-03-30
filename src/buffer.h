#pragma once

#define NULL 0

struct buffer_node_t {
    char* msg;
    int sz;
    struct buffer_node_t* next;
};
 
struct socket_buffer_t {
    int size;
    int offset;
    struct buffer_node_t *head;
    struct buffer_node_t *tail;
};
 
 
struct buffer_pool_t {
    struct buffer_node_t** pool;
    struct buffer_node_t* head;  
    int len;
};

typedef struct buffer_node_t   buffer_node;
typedef struct socket_buffer_t socket_buffer;
typedef struct buffer_pool_t   buffer_pool;

int   push_buffer(buffer_pool* pool, socket_buffer* sb, char* msg, int sz);
void  release_free_node(buffer_pool* pool, socket_buffer* sb);
char* read_buffer(buffer_pool* pool, socket_buffer* sb, int sz);
char* readall(buffer_pool* pool, socket_buffer* sb, int* retNum);   