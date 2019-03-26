#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "logger.h"
#include "servermanager.h"
#include "event_loop.h"

#include <stdlib.h>
#include "epoll.h"


#define MAX_LOOP 4

event_loop *g_loops[MAX_LOOP];


int main()
{
    return 0;
}


void* spawn_thread(void *arg)
{
	int i = (int)(*(int*)arg);
	
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
    int timeout = -1;
    while(1)  {
        
    }
}