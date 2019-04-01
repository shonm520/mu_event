#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "event.h"
#include "servermanager.h"
#include "event_loop.h"
#include "logger.h"
#include "epoll.h"


void event_handler(event* ev)
{
    if (ev->active_event & EPOLLRDHUP)  {

    }
    if (ev->active_event & (EPOLLIN | EPOLLPRI))  {
        if (ev->event_read_handler)  {
            ev->event_read_handler(ev->fd, ev, ev->r_arg);
        }
    }
    if (ev->active_event & EPOLLOUT)  {
        if (ev->event_write_handler)  {
            ev->event_write_handler(ev->fd, ev, ev->w_arg);
        }
    }
}

event* event_create(int fd, short event_flag, event_callback_pt read_cb,
                    void* r_arg, event_callback_pt write_cb, void* w_arg)
{
    event* ev = (event*)malloc(sizeof(event));
    if (ev == NULL)  {
        debug_ret("file: %s, line: %d", __FILE__, __LINE__);
        return NULL;
    }

    ev->fd = fd;
    ev->event_flag = event_flag;
    ev->active_event = 0;
    ev->event_read_handler = read_cb;
    ev->r_arg = r_arg;

    ev->event_write_handler = write_cb;
    ev->w_arg = w_arg;

    return ev;
}

void event_free(event* ev)
{
    event_stop(ev);
	close(ev->fd);
	free(ev);
}

void event_add_io(int epoll_fd, event* ev)
{
    epoller_add(epoll_fd, ev);
    ev->epoll_fd = epoll_fd;
    ev->is_working = 1;
}

void event_modify_flag(event* ev, int new_flag)
{
    if (ev->is_working == 0)  {
        debug_msg("epoll_fd cannot modify, because it's not working now! %s, line: %d", __FILE__, __LINE__);
        return;
    }
    ev->event_flag = new_flag;
    epoller_modify(ev->epoll_fd, ev);
}

void event_add_flag(event* ev, int add_flag, int plus)
{
    int flag = ev->event_flag |= add_flag;
    if (plus == 0)  {
        flag = ev->event_flag &= ~add_flag;
    }
    event_modify_flag(ev, flag);
}

void event_enable_writing(event* ev)
{
    event_add_flag(ev, EPOLLOUT, 1);
}

void event_disable_writing(event* ev)
{
    event_add_flag(ev, EPOLLOUT, 0);
}


void event_stop(event *ev)
{
	if (ev->is_working == 0)                   //判断事件ev是否在epoll中,防止重复删除同一事件
		return;

    epoller_del(ev->epoll_fd, ev);

	ev->is_working = 0;
}