#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "config.h"


socket_buffer* socket_buffer_new()
{
    socket_buffer* sb = (socket_buffer*)mu_malloc(sizeof(socket_buffer));
    sb->size = 0;
    sb->offset = 0;
    sb->head = sb->tail = NULL;
    sb->pool = (buffer_pool*)mu_malloc(sizeof(buffer_pool));
    sb->pool->head = NULL;
    sb->pool->len = 0;
    return sb;
}

void socket_buffer_free(socket_buffer* sb)
{
    buffer_node* head = sb->pool->head;
    for(; head != NULL; head = head->next)  {
        if (head->msg)  {
            mu_free(head->msg);
            head->msg = NULL;
        }
    }
}

buffer_node* new_buffer_node(int size)
{
    buffer_node** pool =  (buffer_node**)mu_malloc(sizeof(buffer_node*) * size);
    int i = 0;
    for (i = 0; i < size; i++)  {
        pool[i] = (buffer_node*)mu_malloc(sizeof(buffer_node));
    }
    for (i = 0; i < size; i++)  {
        pool[i]->msg = NULL;
        pool[i]->sz = 0;
        pool[i]->next = pool[i + 1];
    }
    pool[size - 1]->next = 0;
    return pool[0];
}


int buffer_push_data(socket_buffer* sb, char* msg, int sz)     //写数据到缓冲池中的新结点，然后sb指向新结点
{
    if (msg == NULL || sz == 0)  {
        return 0;
    }
    buffer_pool* pool = sb->pool;
    buffer_node* free_node = NULL;
    if (pool->head == NULL)  {                //缓冲池已空
        int len = pool->len + 1;
        int size = 8;
        if (len <= LARGE_PAGE_NODE - 3 )  {
            size <= len;
        }  else  {
            size <= LARGE_PAGE_NODE - 3;
        }
        free_node = new_buffer_node(size);
        pool->len = size;
    }
    else  {
        free_node = pool->head;
    }   
    pool->head = free_node->next;            //取pool的头结点作为free_node,然后头结点指向next
    char* msgt = (char*)mu_malloc(sz);
    memcpy(msgt, msg, sz);
    free_node->msg = msgt;
    free_node->sz = sz;
    free_node->next = NULL;

    if (sb->head == NULL)  {
        sb->head = sb->tail = free_node;
    }  else  {
        sb->tail->next = free_node;
        sb->tail = free_node;
    }
    sb->size += sz;
    return sb->size;
}


void buffer_node_release(socket_buffer* sb)    //返回给缓冲池
{
    buffer_pool* pool = sb->pool;
    buffer_node* free_node = sb->head;     //把头结点释放
    sb->offset = 0;
    sb->head = free_node->next;            //重置数据区
    if (sb->head == NULL)  {
        sb->tail = NULL;
    }
    free_node->next = pool->head;          //把该结点加入到缓冲池
    mu_free(free_node->msg);               //这里放心free因为结点的指针有自己的空间
    free_node->msg = NULL;
    free_node->sz = 0;
    pool->head = free_node;                //重置缓冲池头结点
}


char* buffer_read_spec(socket_buffer* sb, int sz, int* realSz)    //读取缓冲区sz个字节
{
    if (sz == 0)  {
        return NULL;
    }
    if (sb->size < sz)  {     //缓冲区数据不够
        if (realSz)  {
            return buffer_read_all(sb, realSz);
        }
        else {
            return NULL;
        }
    }

    buffer_pool* pool = sb->pool;   
    sb->size -= sz;
    buffer_node* cur = sb->head;
    char* msg = (char*)mu_malloc(sz);
    int curLen = cur->sz - sb->offset;
    if (sz <= curLen)  {            //要读取的数据小于等于当前结点剩余的数据
        memcpy(msg, cur->msg + sb->offset, sz);
        sb->offset += sz;
        if (sz == curLen)  {        //刚好读完就释放该结点
            buffer_node_release(sb);    
        }
        return msg;
    }
    else  {    //要读取的数据大于当前结点的数据，其余结点又有
        int offset = 0;
        for ( ;; )  {
            int curLen = cur->sz - sb->offset;
            if (curLen >= sz)  {          //第二次读
                memcpy(msg + offset, cur->msg + sb->offset, sz);
                offset += sz;
                sb->offset += sz;
                if (curLen == sz)  {
                    buffer_node_release(sb);
                }
                break;
            }
            int real_sz = (sz < curLen) ? sz : curLen;
            memcpy(msg + offset, cur->msg + sb->offset, real_sz);   //先把当前结点剩余的的数据给读取了
            offset += real_sz;
            buffer_node_release(sb);
            sz -= curLen;
            if (sz == 0)  {
                break;
            }
            cur = sb->head;    //由于上面释放过，这里是新的结点
            if (!cur)   { 
                break;
            }
        }
        return msg;
    }
    return NULL;
}

char* buffer_read_all(socket_buffer* sb, int* retNum)      //读取缓冲区所有的字节
{
    int total_size = sb->size;
    if (total_size <= 0)  {
        return NULL;
    }
    char* msg = (char*)mu_malloc(total_size + 1); 
    int offset = 0;
    while(sb->head)  {
        buffer_node* cur = sb->head;
        int curLen = cur->sz - sb->offset;

        memcpy(msg + offset, cur->msg + sb->offset, curLen);
        offset += curLen; 
        buffer_node_release(sb);       //这里会重置sb->offset并重新设置头
    }
     if (retNum)  {
        *retNum = offset;
    }
    sb->size = 0;
    return msg;
}

int buffer_get_size(socket_buffer* sb)
{
    if (sb)
        return sb->size;
    return 0;
}

