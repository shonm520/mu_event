#pragma once


typedef struct event_t event;

int epoller_create();
void epoller_free();

void epoller_add(int fd, event* ev);
void epoller_del(int fd, event* ev);
void epoller_modify(int fd, event* ev);

struct timeval epoller_dispatch(int fd, int timeout);