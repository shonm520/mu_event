#pragma once

struct event_loop_t  {
    int epoll_fd;
};

typedef struct event_loop_t event_loop;

event_loop* event_loop_create();
void event_loop_run(event_loop* el);