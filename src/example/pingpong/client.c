#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h> 
#include <netinet/tcp.h>
#include "config.h"
#include "connection.h"
#include "servermanager.h"
#include "listener.h"
#include "buffer.h"
#include "tcpclient.h"
#include "event_loop.h"

int64_t g_totalBytesRead = 0;
int64_t g_totalMessagesRead = 0;


int64_t g_startTime = 0;
int64_t g_endTime = 0;

int g_timeout = 0;

static void onMessage(connection *conn)
{
    int size = connection_send_buffer(conn);
    g_totalBytesRead += size;
    g_totalMessagesRead++;


    {
        int64_t cur_time = 0;
        struct timespec spec;
    
        clock_gettime(CLOCK_MONOTONIC, &spec);
        cur_time = (int64_t)spec.tv_sec * 1000000 + spec.tv_nsec * 0.001;

        g_endTime = cur_time; 

        double total_time = (double)(g_endTime - g_startTime);
        if (total_time > g_timeout * 1000 * 1000)  {
            //printf("total_time is %f\n", total_time);
            int ret = shutdown(conn->connfd, SHUT_WR);
        }


        /*if (g_totalMessagesRead > 271545)  {
            int64_t cur_time = 0;
            struct timespec spec;
        
            clock_gettime(CLOCK_MONOTONIC, &spec);
            cur_time = (int64_t)spec.tv_sec * 1000000 + spec.tv_nsec / 1000;

            g_endTime = cur_time;

            double total_time = (double)(g_endTime - g_startTime) / 1000 / 1000;
            if (total_time > g_timeout)  {
                printf("total_time is %f\n", total_time);
                int ret = -1;
                ret = shutdown(conn->connfd, SHUT_WR);
            }
        }*/
    }
}

static int g_blockSize = 0;



static void ondisconnect(connection* conn)
{

    int64_t cur_time = 0;
	struct timespec spec;
 
	clock_gettime(CLOCK_MONOTONIC, &spec);
	cur_time = (int64_t)spec.tv_sec * 1000000 + spec.tv_nsec / 1000;

    g_endTime = cur_time;

    double total_time = (double)(g_endTime - g_startTime) / 1000 / 1000;

    printf("total time is %f\n", total_time);


    printf("totalBytesRead is %ld\n", g_totalBytesRead);
    printf("totalMessagesRead is %ld\n", g_totalMessagesRead);
    
    printf("average message size is %f\n", ((double)g_totalBytesRead / (double)g_totalMessagesRead));

    printf("MiB/s throughput is %f\n", ((double)g_totalBytesRead / (total_time * 1024 * 1024)));

    exit(0);
}

static void onConnected(connection* conn)
{
    fcntl(conn->connfd, F_SETFL, fcntl(conn->connfd, F_GETFL) | O_NONBLOCK);

    int optval = 1;
    setsockopt(conn->connfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));

    if (g_blockSize > 64 * 1024)  {
        char* buf = mu_malloc(g_blockSize);
        if (buf)  {
            send(conn->connfd, buf, g_blockSize, 0);
            mu_free(buf);
        }
        else  {
            printf("malloc failed!\n");
        }
    }
    else  {
        char buf[1024 * 64] = {0};
        int i = 0;
        for (; i < g_blockSize; ++i)  {
            buf[i] = (i % 128);
        }
        send(conn->connfd, buf, g_blockSize, 0);
    }
    conn->disconnected_cb = ondisconnect;   
}


int main(int argc, char* argv[])  
{
    if (argc != 7 )  {
        fprintf(stderr, "Usage: client <host_ip> <port> <threads> <blocksize> ");
        fprintf(stderr, "<sessions> <time>\n");
        return -1;
    }

    const char* ip = argv[1];
    short port = (atoi(argv[2]));
    int threadCount = atoi(argv[3]);
    int blockSize = atoi(argv[4]);
    int sessionCount = atoi(argv[5]);
    int timeout = atoi(argv[6]);

    g_blockSize = blockSize;

    g_timeout = timeout;

    printf("port, thread_num, blockSize, sessionCount, timeout is %d, %d, %d, %d, %d \n", port, threadCount, blockSize, sessionCount, timeout);


    printf("pid = %d\n", getpid());

    int64_t cur_time = 0;
	struct timespec spec;
 
	clock_gettime(CLOCK_MONOTONIC, &spec);
	cur_time = (int64_t)spec.tv_sec * 1000000 + spec.tv_nsec / 1000;

    g_startTime = cur_time;

    signal(SIGPIPE, SIG_IGN);

    int i = 0;
    event_loop* loop = event_loop_create();
    for (; i < sessionCount; i++)  {
        tcpclient* cli = tcpclient_create(ip, port);
        if (cli)  {
            cli->loop = loop;
            tcpclient_connect(cli, onConnected, onMessage);
        }
        else  {
            printf("create cli failed: i = %d \n", i);
            break;
        }
    }
    tcpclient_run(loop);

	return 0;
}