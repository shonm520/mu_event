#include <stdio.h>

#include "../timer.h"



int main()
{
    timer_manager* t = timer_manager_create();

    timer* t1 = timer_create(500);
    timer* t2 = timer_create(250);
    timer* t3 = timer_create(360);
    timer* t4 = timer_create(180);
    timer* t5 = timer_create(300);
    timer* t6 = timer_create(180);
    timer_manager_push(t, t1);
    timer_manager_push(t, t2);
    timer_manager_push(t, t3);
    timer_manager_push(t, t4);
    timer_manager_push(t, t5);
    timer_manager_push(t, t6);

    //timer* top = timer_manager_get_top(t);

    //printf("t top is %d\n", top->time_out);

    timer* top = timer_manager_pop(t);
    printf("t top is %d\n", top->time_out);
    top = timer_manager_pop(t);
    printf("t top is %d\n", top->time_out);
    top = timer_manager_pop(t);
    printf("t top is %d\n", top->time_out);
    top = timer_manager_pop(t);
    printf("t top is %d\n", top->time_out);
    top = timer_manager_pop(t);
    printf("t top is %d\n", top->time_out);
    top = timer_manager_pop(t);
    printf("t top is %d\n", top->time_out);


    timer_manager_push(t, t3);
    timer_manager_push(t, t3);
    timer_manager_push(t, t3);
    top = timer_manager_pop(t);
    printf("t top is %d\n", top->time_out);

    timer_manager_free(t);

    return 0;
}