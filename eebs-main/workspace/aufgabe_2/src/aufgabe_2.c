#include "aufgabe_2.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h> /* strerror */
#include <errno.h>  /* errno */

#define CALLNEW(call) \
do { int ret = call;\
if ( ret != 0 ){\
fprintf(stderr, "%s:%d %s failed: %s\n", __FILE__, __LINE__, #call, strerror(ret));\
exit(1);\
}} while(0);

#define CALLOLD(call) \
do { int ret = call;\
if ( ret < 0 ){\
fprintf(stderr, "%s:%d %s failed: %s\n", __FILE__, __LINE__, #call, strerror(errno));\
exit(1);\
}} while(0);


// Global variables and synchronization objects
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;
int people_in_room = 0; // Number of people in the room
int keep_running = 1; // Time people in the room will be enterin/exiting

// Function for the thread that simulates arrivals
void increase_stack_usage(int depth) {
    if (depth <= 0) {
        printf("recursion finished ##################\n");
        return;
    }
    increase_stack_usage(depth - 1);
}

void* arrival_thread(void* arg) {
    //increase_stack_usage(500);
    while (keep_running) {
        // Simulate the arrival of a person
        pthread_mutex_lock(&mutex);
        people_in_room++;
        printf("A person enters the room. Current occupancy: %d\n", people_in_room);
        pthread_cond_signal(&condvar); // Signal to the monitor_thread
        pthread_mutex_unlock(&mutex);

        // Wait for a random time before the next person arrives (between 1 and 3 seconds)
        sleep(rand() % 3 + 1);
    }
    printf("arrival_thread is finished.\n");
    return NULL;
}

// Function for the thread that simulates departures
void* departure_thread(void* arg) {
    //increase_stack_usage(500);
    while (keep_running) {
        pthread_mutex_lock(&mutex);
        // Simulate the leaving of a person
        if (people_in_room > 0) {
            people_in_room--;
            printf("A person leaves the room. Current occupancy: %d\n", people_in_room);
            pthread_cond_signal(&condvar); // Signal to the monitor_thread
        }
        pthread_mutex_unlock(&mutex);

        // Wait for a random time before the next person leaves the room (between 1 and 3 seconds)
        sleep(rand() % 3 + 1);
    }
    printf("departure_thread is finished.\n");
    return NULL;
}

// Function for the thread that monitors and logs the room occupancy
void* monitor_thread(void* arg) {
   while (keep_running) {
        pthread_mutex_lock(&mutex); // Lock the Mutex
        pthread_cond_wait(&condvar, &mutex); // Release the mutex and wait until the room occupancy changes
        printf("-- Monitor: Current occupancy: %d person(s) in the room.\n", people_in_room); // Re-lock the Mutex and print the output the room status
        pthread_mutex_unlock(&mutex); // Unlock the Mutex
    }
    printf("monitor_thread is finished.\n");
    return NULL;
}

void ub2() {
    srand(time(NULL));

    pthread_t arrival_tid, departure_tid, monitor_tid;

    pthread_attr_t arrival_attr = prepare_stack(ARM_32_DEFAULT_STACKSIZE, 0);
    pthread_attr_t departure_attr = prepare_stack(ARM_32_DEFAULT_STACKSIZE, 0);
    pthread_attr_t monitor_attr = prepare_stack(ARM_32_DEFAULT_STACKSIZE, 0);

    // Create threads
    CALLNEW(pthread_create(&arrival_tid, &arrival_attr, arrival_thread, NULL));
    CALLNEW(pthread_create(&departure_tid, &departure_attr, departure_thread, NULL));
    CALLNEW(pthread_create(&monitor_tid, &monitor_attr, monitor_thread, NULL));


    // Let the threads run for a specified time
    sleep(10);
    // This ensures all threads terminate cleanly, and no thread hangs indefinitely.
    // The monitor_thread waiting on pthread_cond_wait will be woken up by pthread_cond_broadcast
    // After waking up, the monitor_thread will acquire the mutex, notice that keep_running is 0, and exit the loop.
    pthread_mutex_lock(&mutex);
    keep_running = 0; // Stop the threads
    pthread_cond_broadcast(&condvar); // Only necessary when finite time is specified
    pthread_mutex_unlock(&mutex);

    // Wait for all threads to finish
    pthread_join(arrival_tid, NULL);
    pthread_join(departure_tid, NULL);
    pthread_join(monitor_tid, NULL);

    // Free resources by destroying the mutex and condition variable
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condvar);

    printf("Program terminated cleanly.\n");

    size_t arrival_stack_size = determine_used_stacksize(&arrival_attr);
    size_t departure_stack_size = determine_used_stacksize(&departure_attr);
    size_t monitor_stack_size = determine_used_stacksize(&monitor_attr);
    printf("arrival stack size: %lu\ndeparture stack size: %lu\nmonitor stack size: %lu\n", arrival_stack_size, departure_stack_size, monitor_stack_size);
}

int main(int argc, char** argv) {
    ub2();
    return EXIT_SUCCESS;
}

