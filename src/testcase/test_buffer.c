#include<stdio.h>
#include<stdlib.h>

#include "../buffer.h"

int main()  
{
    socket_buffer* sb = socket_buffer_new();

    buffer_push_data(sb, "123456", 6);
    buffer_push_data(sb, "abcdef", 6);
    buffer_push_data(sb, "xyzmnl", 6);
    
    int size = 0;
    char* msg = buffer_read_spec(sb, 28, NULL);
    printf("read %s, %d\n", msg, size);
    printf("aft read sb->size is %d\n\n", sb->size);

    msg = buffer_read_spec(sb, 18, &size);
    printf("read %s, %d\n", msg, size);
    printf("aft read sb->size is %d\n\n", sb->size);

    msg = buffer_read_all(sb, &size);
    printf("read %s, %d\n", msg, size);
    printf("aft read sb->size is %d\n\n", sb->size);

    socket_buffer_free(sb);
    return 0;
}