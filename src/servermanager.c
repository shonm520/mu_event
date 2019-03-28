#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include "logger.h"
#include "servermanager.h"
#include "event_loop.h"
#include "epoll.h"


#define MAX_LOOP 4

event_loop *g_loops[MAX_LOOP];



void* spawn_thread(void *arg)
{
	int i = (long)arg;
	g_loops[i] = event_loop_create();
	event_loop_run(g_loops[i]);
}


server_manager* server_manager_create()
{
    server_manager* manager = (server_manager*)malloc(sizeof(server_manager));

    if (manager == NULL)  {
		debug_ret("file: %s, line: %d", __FILE__, __LINE__);
		return NULL;
	}
	
	manager->epoll_fd = epoller_create();
	if (manager->epoll_fd == -1)  {
		debug_ret("file: %s, line: %d", __FILE__, __LINE__);
		free(manager);
		return NULL;
	}

    signal(SIGPIPE, SIG_IGN);

    int i = 0;
    pthread_t tid;
	for (i = 0; i < MAX_LOOP; i++)  {
		pthread_create(&tid, NULL, spawn_thread, (void *)i);
	}
	
	return manager;
}



void server_manager_run(server_manager* manager)
{
    int timeout = -1;    //阻塞等待
    while(1)  {
        epoller_dispatch(manager->epoll_fd, timeout);
    }
}