#pragma once 


typedef void (*timeout_callback_pt)(void *arg);


enum TimerOptions {
    TIMER_OPT_NONE,		// 超时不处理 
	TIMER_OPT_ONCE,		// 超时处理一次 
	TIMER_OPT_REPEAT	// 超时重复处理 
};


struct timer_  {
    int time_out;        //单位毫秒
    int time_left;
    enum TimerOptions type;
    timeout_callback_pt callback;
    void* arg;
};

typedef struct timer_ timer;

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

void timer_manager_free(timer_manager* m);

timer* timer_new(int utime);

void timer_manager_push(timer_manager* manager, timer ti);

timer timer_manager_pop(timer_manager* manager);

timer timer_manager_get_top(timer_manager* manager);

void timer_manager_update(timer_manager* manager, int timeout);