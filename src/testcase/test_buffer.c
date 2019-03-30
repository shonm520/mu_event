#include<stdio.h>
#include<stdlib.h>

#include "../buffer.h"

int main()  
{
    socket_buffer* sb = malloc(sizeof(socket_buffer));
    buffer_pool* pool = malloc(sizeof(buffer_pool));

    push_buffer(pool, sb, "123456", 6);
    push_buffer(pool, sb, "abcdef", 6);
    push_buffer(pool, sb, "abcdef", 6);
    
    int size = 0;
    char* msg = read_buffer(pool, sb, 8);
    printf("read %s, %d\n\n", msg, size);
    printf("aft read sb->size is %d\n", sb->size);
    msg = read_buffer(pool, sb, 8);
    printf("read %s, %d\n\n", msg, size);
    printf("aft read sb->size is %d\n", sb->size);

    msg = read_buffer(pool, sb, 8);
    printf("read %s, %d\n\n", msg, size);
    printf("aft read sb->size is %d\n", sb->size);
    return 0;
}