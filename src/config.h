#pragma once 
#include <stdlib.h>

#ifndef bool
    #define bool int 
#endif

#ifndef true
    #define true 1
#endif 

#ifndef false
    #define false 0
#endif 


#ifndef NULL
    #define NULL 0
#endif 


#define mu_malloc malloc
#define mu_free   free

#define LARGE_PAGE_NODE 12   //buffer node 的对其

#define MAX_EVENTS  32       //epoll_wait一次性监听最大的事件数量

#define DEFAULT_PORT  2019   //默认端口
#define MAX_LOOP 4           //线程数量



#define MAX_TIMER_EVENT 100 //最大定时器个数   