#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include "logger.h"
#include "servermanager.h"
#include "event_loop.h"
#include "epoll.h"
#include "config.h"



event_loop *g_loops[MAX_LOOP];


void* spawn_thread(void *arg)
{
	int i = (long)arg;
	g_loops[i] = event_loop_create();
	event_loop_run(g_loops[i]);
}


server_manager* server_manager_create(int port, int thread_num)
{
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
    int i = 0;
	for (i = 0; i < thread_num; i++)  {
		pthread_create(&tid, NULL, spawn_thread, (void *)i);
	}
	
	return manager;
}



void server_manager_run(server_manager* manager)
{
    event_loop_run(manager->loop);
}