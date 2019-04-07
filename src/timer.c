
#include <string.h>
#include "timer.h"
#include "config.h"


timer_manager* timer_manager_create()
{
    int size = sizeof(timer_manager);
    timer_manager* m = (timer_manager*)mu_malloc(size);
    if (NULL == m)  {
        return NULL;
    }

    memset(m, 0, size);
    m->cap = MAX_TIMER_EVENT;

    m->queue0 = (timer**)mu_malloc(size * m->cap);
    m->queue1 = (timer**)mu_malloc(size * m->cap);
    memset(m->queue0, 0, size * m->cap);
    memset(m->queue1, 0, size * m->cap);
    return m;
}

void timer_manager_free(timer_manager* m)
{
    if (m)  {
        if(m->queue0) mu_free(m->queue0);
        if(m->queue1) mu_free(m->queue1);
    }
}


timer* timer_create(int utime)
{
    timer* t = malloc(sizeof(timer));
    memset(t, 0, sizeof(timer));
    t->time_out = utime;
    return t;
}


void timer_manager_push(timer_manager* manager, timer* ti)
{
    int size = manager->size;
    if (size >= manager->cap)  {
        timer** temp0 = manager->queue0;
        timer** temp1 = manager->queue1;
        manager->queue0 = mu_malloc(size * manager->cap * 2);
        manager->queue1 = mu_malloc(size * manager->cap * 2);

        memset(manager->queue0, 0, size * manager->cap * 2);
        memset(manager->queue1, 0, size * manager->cap * 2);

        memcpy(manager->queue0, temp0, manager->cap);
        memcpy(manager->queue1, temp1, manager->cap);
        mu_free(temp0);
        mu_free(temp1);
        manager->cap *= 2;
    }
    if (manager->top)  {   
        if (ti->time_out < manager->top->time_out)  {   //替换最小的
            manager->top = ti;
            manager->top_index = size;
        }
    }
    else  {
        manager->top = ti;
    }
    
    manager->queue0[manager->size++] = ti;
}


timer* timer_manager_pop(timer_manager* manager)
{
    if (manager == NULL)  {
        return NULL;
    }
    if (manager->size <= 0)  {
        return NULL;
    }
    if (manager->size == 1)  {
        manager->size = 0;
        return manager->top;
    }
    timer* t = manager->top;
    int top_index = manager->top_index;
    if (manager->size > 1) {
        timer* top = NULL;
        timer** queue0;
        timer** queue1;
        if (manager->index == 0)  {
            queue0 = manager->queue0;
            queue1 = manager->queue1;
        }
        else  {
            queue0 = manager->queue1;
            queue1 = manager->queue0;
        }
        manager->index = (manager->index + 1) % 2; 
        int i = 0;
        int j = 0;
        for (; i < manager->size; i++)  {
            if (i == top_index)  {
                continue;
            }
            queue1[j] = queue0[i];
            if (top == NULL)  {
                top = queue0[i];
                manager->top_index = 0;
            }
            else  {
                if (queue0[i]->time_out < top->time_out)  {
                    top = queue0[i];
                    manager->top_index = j;
                }
            }
            j++; 
        }
        manager->top = top;
    }
    manager->size -= 1;
    return t;
}

timer* timer_manager_get_top(timer_manager* manager)
{
    return manager->top;
}
