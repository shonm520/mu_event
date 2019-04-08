#pragma once 


struct timer_t  {
    int time_out;
};

typedef struct timer_t timer;

struct timer_manager_t  {
    timer* top;
    timer** queue0;
    timer** queue1;
    int index;
    int size;
    int cap;
    int top_index;
};


typedef struct timer_manager_t timer_manager;

timer_manager* timer_manager_create();

timer* timer_create(int ut);

void timer_manager_push(timer_manager* manager, timer ti);

timer timer_manager_pop(timer_manager* manager);

timer timer_manager_get_top(timer_manager* manager);