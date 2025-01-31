#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>
#include <string.h> /* strerror */
#include <errno.h>  /* errno */
#include <unistd.h> /* sleep */
#include <limits.h> /* PTHREAD_STACK_MIN */

double clocks_to_ms(size_t clocks);

void waste_time(size_t count);

size_t calibrate(size_t desired_ms);

void run(bool use_custom_stacksize, bool measure_stack);

