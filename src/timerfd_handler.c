/*
 * timerfd_handler.c
 *
 *  Created on: May 16, 2022
 *      Author: Luca Castelli
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <pthread.h>

#include "timerfd_handler.h"

/* ------------------------------------------------------------------
 * Local define
 * ------------------------------------------------------------------ */
#define MAX_EVENTS 10
#define PROCESS_PRIORITY 0
/* ------------------------------------------------------------------
 * End local define
 * ------------------------------------------------------------------ */

/* ------------------------------------------------------------------
 * Local type
 * ------------------------------------------------------------------ */
typedef struct {
    int us_interval;         /* configured interval in us */
    int interval_fd;              /* timer fd */
} threadRT_t;
/* ------------------------------------------------------------------
 * End local type
 * ------------------------------------------------------------------ */

/* ------------------------------------------------------------------
 * Local variable
 * ------------------------------------------------------------------ */
/*! \brief Interval file descriptor */
threadRT_t threadRT;

/*! \brief Callback called after timer_fd set */
void (*callback)() = NULL;
/* ------------------------------------------------------------------
 * End local variable
 * ------------------------------------------------------------------ */

/* ------------------------------------------------------------------
 * Function prototypes
 * ------------------------------------------------------------------ */

/* ------------------------------------------------------------------
 * End function prototypes
 * ------------------------------------------------------------------ */


/************************ LOCAL FUNCTIONS *****************************/

void threadTmr_init(int interval_us)
{
    struct itimerspec itval;

    threadRT.us_interval = interval_us;
    /* set up non-blocking interval timer */
    threadRT.interval_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    (void)fcntl(threadRT.interval_fd, F_SETFL, O_NONBLOCK);
    itval.it_interval.tv_sec = 0;
    itval.it_interval.tv_nsec = interval_us * 1000;
    itval.it_value = itval.it_interval;
    (void)timerfd_settime(threadRT.interval_fd, 0, &itval, NULL);
}

/**********************************************************************/

int set_rt(void)
{
	int res = 0;
	int which = PRIO_PROCESS;
	id_t pid = 0;
	int ret = 0;

	pid = getpid();
	ret = setpriority(which, pid, PROCESS_PRIORITY);
	ret = getpriority(which, pid);
	if(ret != PROCESS_PRIORITY) { printf("Time handler: Process priority not set \n"); }

	pthread_t ptid = pthread_self();
    struct sched_param param;

    param.sched_priority = 99;
    if ((res = pthread_setschedparam(ptid, SCHED_FIFO, &param)) != 0) {
    	printf("Time handler: Error on pthread_setschedparam\n");
    	return (res);
    }

    return (res);
}

/************************ END LOCAL FUNCTION **************************/

/************************* GLOBAL FUNCTION ****************************/

void *timerfd_handler_library(void *arg)
{
	int epollfd = 0;
	int result = 0;
	unsigned long missed = 0;
	struct epoll_event ev, events[MAX_EVENTS];
	int us_interval = 0;

	if(*(int *) arg < 100)
	{
		printf("Time handler: Error on us_interval param, closing thread\n");
		return NULL;
	}
	else
	{
		us_interval = *(int *) arg;
	}

	result = set_rt();
	if(result)
	{
		printf("Time handler: Error on set_rt, closing thread\n");
		return NULL;
	}

    /* Init threadRT structure and file descriptors */
    threadTmr_init(us_interval);

	epollfd = epoll_create1(0);
	if (epollfd == -1) {
	   printf("Time handler: epoll_create1 error, closing thread\n");
	   return NULL;
	}

    ev.events = EPOLLIN;
    ev.data.fd = threadRT.interval_fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, threadRT.interval_fd, &ev) == -1) {
        perror("Time handler: epoll_ctl: listen_sock, closing thread\n");
        return NULL;
    }

    int nfds = 0;
    int n = 0;

    while(1)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            printf("Time handler: epoll_wait, closing thread\n");
            return NULL;
        }

        for (n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == threadRT.interval_fd)
            {
				result = read(threadRT.interval_fd, &missed, sizeof(missed));
				if (result > 0) {
					if(missed > 1)
					{
						printf("Time handler: missed %ld\n", missed);
					}
					if((*callback) != NULL)
					{
						callback();
					}
				}
				else
				{
					printf("result<0\n");
				}
            }
        }
    }
}

/**********************************************************************/

void set_callback(void (*lcallback)())
{
	callback = lcallback;
}

/************************ END GLOBAL FUNCTION **************************/
