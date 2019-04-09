
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
        timer** queue = m->queue0;
        if (m->index == 1)  {
            queue = m->queue1;
        }
        int i = 0;
        for (; i < m->cap ; i++)  {
            if (queue[i])  {
                mu_free(queue[i]);
            }
            else  {
                break;
            }
        }

        if(m->queue0) mu_free(m->queue0);
        if(m->queue1) mu_free(m->queue1);
    }
}


timer* timer_new(int utime)
{
    timer* t = (timer*)malloc(sizeof(timer));
    memset(t, 0, sizeof(timer));
    t->time_out = utime;
    return t;
}


void timer_manager_push(timer_manager* manager, timer ti)
{
    int size = manager->size;
    if (size >= manager->cap)  {
        timer** temp0 = manager->queue0;
        timer** temp1 = manager->queue1;
        int size_of_ptr = sizeof(timer*);
        manager->queue0 = (timer**)mu_malloc(size_of_ptr * manager->cap * 2);
        manager->queue1 = (timer**)mu_malloc(size_of_ptr * manager->cap * 2);

        memset(manager->queue0, 0, size_of_ptr * manager->cap * 2);
        memset(manager->queue1, 0, size_of_ptr * manager->cap * 2);

        //memcpy(manager->queue0, temp0, manager->cap);
        //memcpy(manager->queue1, temp1, manager->cap);

        if (manager->index == 0)  {
            int i = 0;
            for (; i < manager->size; i++)  {
                manager->queue0[i] = temp0[i];
            }
        }
        else  {
            int i = 0;
            for (; i < manager->size; i++)  {
                manager->queue1[i] = temp1[i];
            }
        }

        mu_free(temp0);
        mu_free(temp1);
        manager->cap *= 2;
    }
    ti.time_left = ti.time_out;
    timer* pti = (timer*)mu_malloc(sizeof(timer));
    *pti = ti;
    if (manager->top)  {   
        if (pti->time_left < manager->top->time_left)  {   //替换最小的
            manager->top = pti;
            manager->top_index = size;
        }
    }
    else  {
        manager->top = pti;
    }

    if (manager->index == 0)  {
        manager->queue0[manager->size++] = pti;
    }
    else  {
        manager->queue1[manager->size++] = pti;
    }
}


timer timer_manager_pop(timer_manager* manager)
{
    timer ti;
    ti.time_out = 0;
    if (manager == NULL)  {
        return ti;
    }
    if (manager->size <= 0)  {
        return ti;
    }
    timer ret = *manager->top;
    if (manager->size == 1)  {
        manager->top = NULL;
        manager->size = 0;
        return ret;
    }
    mu_free(manager->top);
    manager->top = NULL;
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
        for (; i < manager->cap; i++)  {
            if (i < manager->size)  {
                if (i == top_index)  {
                    continue;
                }
                queue1[j] = queue0[i];
                if (top == NULL)  {
                    top = queue0[i];
                    manager->top_index = 0;
                }
                else  {
                    if (queue0[i]->time_left < top->time_left)  {
                        top = queue0[i];
                        manager->top_index = j;
                    }
                }
                j++; 
            }
            else  {
                queue0[i] = NULL;
                queue1[i] = NULL;
            }   
        }
        manager->top = top;
    }
    manager->size -= 1;
    return ret;
}

timer timer_manager_get_top(timer_manager* manager)
{
    return *manager->top;
}

void timer_manager_update(timer_manager* manager, int timeout)
{
    timer** queue = NULL;
    if (manager->index == 0)  {
        queue = manager->queue0;
    }
    else  {
        queue = manager->queue1;
    }

    int i = 0;
    for (; i < manager->size; i++)  {
        timer* ti = queue[i];
        ti->time_left -= timeout;
    }
}
