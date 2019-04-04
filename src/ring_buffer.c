#include <string.h>
#include <stdlib.h>
#include "ring_buffer.h"
#include "config.h"

ring_buffer* ring_buffer_new()
{
    ring_buffer* rb = mu_malloc(sizeof(ring_buffer));
    memset(rb, 0, sizeof(ring_buffer));
    return rb;
}


void ring_buffer_free(ring_buffer* rb)
{
    if (rb->msg)  {
        mu_free(rb->msg);
    }
}



void ring_buffer_push_data(ring_buffer* rb, char* msg, int size)
{
    int used = rb->end - rb->start;
    int available = rb->cap - used;
    if (available < size)  {          //可用数据不足了
        if (rb->start > size)  {      //可以塞前面
            memmove(rb->msg, rb->msg + rb->start, used);
            rb->start = 0;
            memcpy(rb->msg + used, msg, size);
            rb->end = used + size;
        }
        else  {
            rb->cap = rb->cap * 2 + size;
            char* new_msg = mu_malloc(rb->cap);
            memcpy(new_msg, rb->msg + rb->start, used);
            memcpy(new_msg + used, msg, size);
            if (rb->msg)  {    //刚开始非空
                mu_free(rb->msg);
            }
            rb->msg = new_msg;
            rb->start = 0;
            rb->end = size + used;
        }
        
    }
    else  {
        memcpy(rb->msg + rb->end, msg, size);
        rb->end += size;
    }
}




char* ring_buffer_readable_start(ring_buffer* rb)
{
    return rb->msg + rb->start;
}


int ring_buffer_readable_bytes(ring_buffer* rb)
{
    int ocuSize = rb->end - rb->start;
    return ocuSize;
}

int ring_buffer_available_bytes(ring_buffer* rb)
{
    return rb->cap - rb->end;
}

void ring_buffer_release_bytes(ring_buffer* rb, int size)
{
    rb->start += size;
    if (rb->start == rb->end)  {
        rb->start = rb->end = 0;
    }
}