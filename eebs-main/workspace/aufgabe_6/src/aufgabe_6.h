#ifndef AUFGABE_6_H_
#define AUFGABE_6_H_

#define _GNU_SOURCE     /* To get pthread_getattr_np() declaration */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

extern size_t ARM_32_DEFAULT_STACKSIZE;

pthread_attr_t prepare_stack(size_t stacksize, bool paint_stack);
size_t determine_used_stacksize(pthread_attr_t*);
size_t determine_stacksize(pthread_t* thr, void* thread_function, void* arg);

#endif /* AUFGABE_6_H_ */
