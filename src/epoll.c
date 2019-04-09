#include <sys/time.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "epoll.h"
#include "logger.h"
#include "event.h"
#include "config.h"



int epoller_create()
{
    int epoll_fd = epoll_create(1024);  //大于0就好
    if (epoll_fd == -1)  {
         debug_ret("create epoll failed, file : %s, line : %d", __FILE__, __LINE__);
        return -1;
    }
    return epoll_fd;
}

void epoll_free(int fd)
{
    close(fd);
}

void epoller_add(int epoll_fd, event* e)
{
    struct epoll_event ev;
    ev.events = e->event_flag;
    ev.data.ptr = e;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, e->fd, &ev) == -1)  {
        debug_sys("epoll_ctl add failed, file : %s, line : %d", __FILE__, __LINE__);
    }
}

void epoller_del(int epoll_fd, event* e)
{
    struct epoll_event ev;
    ev.events = e->event_flag;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e->fd, &ev) == -1)  {
        debug_sys("epoll_ctl del failed, file : %s, line : %d", __FILE__, __LINE__);
    }
}

void epoller_modify(int epoll_fd, event* e)
{
    struct epoll_event ev;
    ev.events = e->event_flag;
    ev.data.ptr = e;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, e->fd, &ev) == -1)  {
        debug_sys("epoll_fd modify failed, file : %s, line : %d", __FILE__, __LINE__);
    }
}


struct timeval epoller_dispatch(int epoll_fd, int timeout)
{
    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout);

    if (nfds == -1)  {
        if (errno != EINTR)  {
            debug_sys("epoll_wait failed, file : %s, line : %d", __FILE__, __LINE__);
        }
    }

    struct timeval now;
    gettimeofday(&now, NULL);

    int i;
    event* ev;
    for (i = 0; i < nfds; i++)  {
        ev = (event*)events[i].data.ptr;
        ev->time = now;
        ev->active_event = events[i].events;
        event_handler(ev);
    }

    struct timeval end;
    gettimeofday(&end, NULL);
    return end;
}