/*
 *  Created on: May 16, 2022
 *      Author: Luca Castelli
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License (not later!)
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not,  see <http://www.gnu.org/licenses>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * test.c
 *
 * This program is used to test the timerfd. The software creates and timer
 * file descriptor, it assigns a timeout and a callback to verify
 * the timeout frequency. It has been tested on a ARM cortex A53
 *
 */
#include <stdio.h>
#include <time.h>
#include <timerfd_handler.h>



/* ------------------------------------------------------------------
 * Local define
 * ------------------------------------------------------------------ */
#define US_INTERVAL	1E3
/* -----------------------------------------------------------------
 * End local define
 * ------------------------------------------------------------------ */

/* ------------------------------------------------------------------
 * Local type
 * ------------------------------------------------------------------ */

/* ------------------------------------------------------------------
 * End local type
 * ------------------------------------------------------------------ */

/* ------------------------------------------------------------------
 * Local variable
 * ------------------------------------------------------------------ */

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

static int timeThreshold_usec(long usec_threshold, struct timespec *ltime_reference)
{
	int res = 0;
	struct timespec time; /* @suppress("Variable without initialization") */
	long long time_usec = 0;
	long long time_reference_usec = (ltime_reference->tv_sec * 1E6) + (ltime_reference->tv_nsec / 1E3);

	clock_gettime(CLOCK_MONOTONIC, &time);
	time_usec = (time.tv_sec * 1E6) + (time.tv_nsec / 1E3);

	if(time_usec > (time_reference_usec + usec_threshold)) { res = 1; }

	return (res);
}

/**********************************************************************/

static void routine_rate(void)
{
	static struct timespec time_reference = { 0,0 };
	const int one_second_usec = 1E6;
	static unsigned int counter = 0;
	int res = 0;

	if(time_reference.tv_sec == 0) { clock_gettime(CLOCK_MONOTONIC, &time_reference); }

	++counter;

	res = timeThreshold_usec(one_second_usec, &time_reference);

	if(res == 1)
	{
		time_reference.tv_sec = 0;
		printf("Timerfd rate %d\n", counter);
		counter = 0;
	}
}

/************************ END LOCAL FUNCTION **************************/

/************************* GLOBAL FUNCTION ****************************/

void lcallback(void)
{
	routine_rate();
}

/**********************************************************************/

int main(void)
{
	int time_interval_us = US_INTERVAL;
	printf("Test for timerfd handler\n");
	set_callback(&lcallback);
	timerfd_handler_library(&time_interval_us);
}

/************************ END GLOBAL FUNCTION **************************/
