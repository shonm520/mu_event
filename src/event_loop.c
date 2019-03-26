#include <stdlib.h>
#include "logger.h"
#include "event_loop.h"

#include "epoll.h"

#define mu_malloc malloc;
#define mu_free free;

event_loop* event_loop_create()
{
    event_loop* loop = (event_loop*)mu_malloc(sizeof(event_loop));
    if (loop == NULL)  {
        debug_ret("file : %s, line : %d", __FILE__, __LINE__);
        return NULL;
    }

    loop->epoll_fd = epoller_create();
    if (loop->epoll_fd == -1)  {
        debug_ret("file : %s, line : %d", __FILE__, __LINE__);
        mu_free(loop);
        return NULL;
    }

    return loop;
}

void event_loop_run(event_loop* loop)
{
    int timeout = -1;
    while(1)  {
        epoller_dispatch(loop->epoll_fd, timeout);
    }
}