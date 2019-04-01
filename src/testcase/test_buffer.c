#include<stdio.h>
#include<stdlib.h>

#include "../buffer.h"

int main()  
{
    socket_buffer* sb = socket_buffer_new();

    push_buffer(sb, "123456", 6);
    push_buffer(sb, "abcdef", 6);
    push_buffer(sb, "xyzmnl", 6);
    
    int size = 0;
    char* msg = read_buffer(sb, 28, NULL);
    printf("read %s, %d\n", msg, size);
    printf("aft read sb->size is %d\n\n", sb->size);

    msg = read_buffer(sb, 18, &size);
    printf("read %s, %d\n", msg, size);
    printf("aft read sb->size is %d\n\n", sb->size);

    msg = readall(sb, &size);
    printf("read %s, %d\n", msg, size);
    printf("aft read sb->size is %d\n\n", sb->size);

    socket_buffer_free(sb);
    return 0;
}