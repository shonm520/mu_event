#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include "logger.h"
#include "servermanager.h"
#include "event_loop.h"
#include "epoll.h"
#include "config.h"
#include "timer.h"


event_loop *g_loops[MAX_LOOP];

int started_loop = 0;
pthread_spinlock_t lock;

void* spawn_thread(void *arg)
{
	int i = (long)arg;
	g_loops[i] = event_loop_create();
    pthread_spin_lock(&lock);
    started_loop++;
    pthread_spin_unlock(&lock);
	event_loop_run(g_loops[i]);
}


server_manager* server_manager_create(int port, int thread_num)
{
    pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);
    server_manager* manager = (server_manager*)malloc(sizeof(server_manager));
    if (manager == NULL)  {
		debug_ret("create server_manager failed, file: %s, line: %d", __FILE__, __LINE__);
		return NULL;
	}
	manager->listen_port = port;

    manager->loop = event_loop_create();
    if (manager->loop == NULL)  {
        debug_ret("create epoller failed, file: %s, line: %d", __FILE__, __LINE__);
	 	mu_free(manager);
	 	return NULL;
    }

    signal(SIGPIPE, SIG_IGN);

    if (thread_num < 0) {
        thread_num = MAX_LOOP;
    }
    manager->loop_num = thread_num;
    pthread_t tid;
    long long i = 0;
	for (i = 0; i < thread_num; i++)  {
		pthread_create(&tid, NULL, spawn_thread, (void *)i);
	}

    while(true)  {       //等event_loop全部create完毕
        pthread_spin_lock(&lock);
        if (started_loop == thread_num)  {
            pthread_spin_unlock(&lock);
            break;
        }
        pthread_spin_unlock(&lock);
    }

    pthread_spin_destroy(&lock);
    manager->timer_manager = timer_manager_create();
	
	return manager;
}

void server_manager_time_event(server_manager* manager, int timeout)
{
    timer_manager* tm = manager->timer_manager;
    if (!tm || tm->size <= 0)  {
        return;
    }

    timer_manager_update(tm, timeout);

    timer* top = tm->top;
    while(top->time_left <= 0)  {
        timer ti = timer_manager_pop(tm);
        if (ti.type == TIMER_OPT_ONCE)  {
            ti.callback(ti.arg);
        }
        else if (ti.type == TIMER_OPT_REPEAT)  {
            server_manager_add_timer(manager, ti);
            ti.callback(ti.arg);
        }
        top = tm->top;
    }
}


bool calc_timeout(server_manager* manager, int* timeout)
{
    timer_manager* tm = manager->timer_manager;
    if (tm && tm->size > 0 && tm->top)  {
        *timeout = tm->top->time_left;
        return true;
    }
    *timeout = -1;
    return false;
}


void server_manager_add_timer(server_manager* manager, timer ti)
{
    timer_manager* tm = manager->timer_manager;
    if (!tm)  {
        return ;
    }

    timer_manager_push(tm, ti);
}



void server_manager_run(server_manager* manager)
{
    int timeout = -1;
    while(1)  {
        int temp;
        bool has_timeout = calc_timeout(manager, &timeout);

        struct timeval now;
        gettimeofday(&now, NULL);

        struct timeval trigger_time = epoller_dispatch(manager->loop->epoll_fd, timeout);       //

        int64_t diff = (trigger_time.tv_sec - now.tv_sec) * 1000 * 1000 + (trigger_time.tv_usec - now.tv_usec);
        timeout = diff / 1000;

        if (has_timeout)  {
            server_manager_time_event(manager, timeout);         //已经过去了多少毫秒
        }
    }
}