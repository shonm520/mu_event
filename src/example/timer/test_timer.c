#include<stdio.h>
#include "config.h"
#include "connection.h"
#include "servermanager.h"
#include "listener.h"
#include "timer.h"



static void timer_cb(void* arg)
{
    printf("timer tick 2 seconds!!!!\n\n");
}

static void timer_cb1(void* arg)
{
    printf("timer tick 1500 miseconds!!!!\n\n");
}

static void timer_cb_once(void* arg)
{
    printf("timer tick once!!!!\n\n");
}


int main(int argc, char* argv[])  
{
    
    int port = DEFAULT_PORT;
    int thread_num = MAX_LOOP;
    if (argc >= 2)
        port = atoi(argv[1]);
    if (argc >= 3)
        thread_num = atoi(argv[2]);


	server_manager *manager = server_manager_create(port, thread_num);
	inet_address addr = addr_create("any", port);
	listener_create(manager, addr, NULL, NULL);

    timer ti;
    ti.type = TIMER_OPT_REPEAT;
    ti.time_out = 2000;
    ti.callback = timer_cb;
    ti.arg = NULL;

    server_manager_add_timer(manager, ti);

    ti.type = TIMER_OPT_REPEAT;
    ti.time_out = 1500;
    ti.callback = timer_cb1;
    server_manager_add_timer(manager, ti);
    ti.type = TIMER_OPT_ONCE;
    ti.time_out = 5000;
    ti.callback = timer_cb_once;
    server_manager_add_timer(manager, ti);
	server_manager_run(manager);

	return 0;
}