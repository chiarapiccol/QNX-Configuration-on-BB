#include "aufgabe_3_4.h"
#include "aufgabe_6.h"

#define CALLNEW(call) \
do {\
    int ret = call;\
    if ( ret != 0 ){\
        fprintf(stderr, "%s:%d %s failed: %s\n", __FILE__, __LINE__, #call, strerror(ret));\
        exit(1);\
    }\
} while(0);

#define CALLOLD(call) \
do {\
    int ret = call;\
    if ( ret < 0 ){\
        fprintf(stderr, "%s:%d %s failed: %s\n", __FILE__, __LINE__, #call, strerror(errno));\
        exit(1);\
    }\
} while(0);

// Calibration values
#define MASTER_PERIOD_MS 2   // Master thread period in milliseconds
#define N 8                 // Number of cycles before triggering following tasks
#define SLAVE_TIME_MS 1      // CPU time for slave thread in milliseconds
#define FOLLOWING_TIME_MS 3  // CPU time for following threads in milliseconds

// Declare Semaphores and
sem_t master_slave_sem;
sem_t slave_following_sem1;
sem_t slave_following_sem2;

// previously calibrated parameter for waste_time
static size_t iterations_1000_ms = 141278291;

// Shared Variables
volatile int n_counter = 0;

// Experimentally determined stacksizes
size_t stacksize_following = 544;
size_t stacksize_slave = 544;
size_t stacksize_master = 560;

// Master Thread Task
void* masterT_task(void* arg) {
    printf("MasterT: masterT_task started\n");
    long tact_count_masterT = MASTER_PERIOD_MS;
    struct timespec next_time;
    clock_gettime(CLOCK_MONOTONIC, &next_time);
    while (1) {
        next_time.tv_nsec += tact_count_masterT * 1000000;
        if (next_time.tv_nsec >= 1000000000) {
            next_time.tv_sec += next_time.tv_nsec / 1000000000;
            next_time.tv_nsec %= 1000000000;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_time, NULL);
        sem_post(&master_slave_sem);
    }

    return NULL;
}

// Slave Thread Task
void* slaveT_task(void* arg) {
    printf("SlaveT: slaveT_task started\n");
    while (1) {
        sem_wait(&master_slave_sem);
        waste_time(iterations_1000_ms / 1000 * SLAVE_TIME_MS);
        if (n_counter % N == 0) {
            sem_post(&slave_following_sem1);
            sem_post(&slave_following_sem2);
        }
        n_counter++;
    }

    return NULL;
}

// Following Thread 1 Task
void* followingT1_task(void* arg) {
    printf("followingT1_task started\n");
    while (1) {
        sem_wait(&slave_following_sem1);
        waste_time(iterations_1000_ms / 1000 * FOLLOWING_TIME_MS);
    }

    return NULL;
}

// Following Thread 2 Task
void* followingT2_task(void* arg) {
    printf("followingT2_task started\n");
    while (1) {
    	sem_wait(&slave_following_sem2);
        waste_time(iterations_1000_ms / 1000 * FOLLOWING_TIME_MS);
    }

    return NULL;
}

void run(bool use_custom_stacksize, bool measure_stack) {
    // Initialize Semaphores
    printf("*Initialize the Semaphores\n");
    sem_init(&master_slave_sem, 0, 0);
    sem_init(&slave_following_sem1, 0, 0);
    sem_init(&slave_following_sem2, 0, 0);

    // Initialize the Threads
    printf("*Initialize the Threads\n");
    pthread_t masterT, slaveT, followingT1, followingT2;

    // Set the Threads' attributes and parameters
    int s;
    pthread_attr_t attr_follow_1, attr_follow_2, attr_slave, attr_master;

    if (use_custom_stacksize || measure_stack) {
        size_t s_follow, s_slave, s_master;
        if (use_custom_stacksize) {
            s_follow = PTHREAD_STACK_MIN + stacksize_following;
            s_slave = PTHREAD_STACK_MIN + stacksize_slave;
            s_master = PTHREAD_STACK_MIN + stacksize_master;
        } else {
            s_follow = ARM_32_DEFAULT_STACKSIZE;
            s_slave = ARM_32_DEFAULT_STACKSIZE;
            s_master = ARM_32_DEFAULT_STACKSIZE;
        }

        // create pthread_attr_t with custom stack for each thread
        attr_follow_1 = prepare_stack(s_follow, measure_stack);
        attr_follow_2 = prepare_stack(s_follow, measure_stack);
        attr_slave = prepare_stack(s_slave, measure_stack);
        attr_master = prepare_stack(s_master, measure_stack);
    } else {
        // initialize regular pthread_attr_t for each thread
        s = pthread_attr_init(&attr_follow_1);
        if (s != 0) {
            printf("pthread_attr_init failed\n");
            exit(1);
        }
        s = pthread_attr_init(&attr_follow_2);
        if (s != 0) {
            printf("pthread_attr_init failed\n");
            exit(1);
        }
        s = pthread_attr_init(&attr_slave);
        if (s != 0) {
            printf("pthread_attr_init failed\n");
            exit(1);
        }
        s = pthread_attr_init(&attr_master);
        if (s != 0) {
            printf("pthread_attr_init failed\n");
            exit(1);
        }
    }

    // set scheduling params
    // following
    s = pthread_attr_setinheritsched(&attr_follow_1, PTHREAD_EXPLICIT_SCHED);
    if (s != 0) {
        printf("pthread_attr_setinheritsched failed\n");
        exit(1);
    }
    s = pthread_attr_setschedpolicy(&attr_follow_1, SCHED_FIFO);
    if (s != 0) {
        printf("pthread_attr_setschedpolicy failed\n");
        exit(1);
    }
    s = pthread_attr_setinheritsched(&attr_follow_2, PTHREAD_EXPLICIT_SCHED);
    if (s != 0) {
        printf("pthread_attr_setinheritsched failed\n");
        exit(1);
    }
    s = pthread_attr_setschedpolicy(&attr_follow_2, SCHED_FIFO);
    if (s != 0) {
        printf("pthread_attr_setschedpolicy failed\n");
        exit(1);
    }
    // slave
    s = pthread_attr_setinheritsched(&attr_slave, PTHREAD_EXPLICIT_SCHED);
    if (s != 0) {
        printf("pthread_attr_setinheritsched failed\n");
        exit(1);
    }
    s = pthread_attr_setschedpolicy(&attr_slave, SCHED_FIFO);
    if (s != 0) {
        printf("pthread_attr_setschedpolicy failed\n");
        exit(1);
    }
    // master
    s = pthread_attr_setinheritsched(&attr_master, PTHREAD_EXPLICIT_SCHED);
    if (s != 0) {
        printf("pthread_attr_setinheritsched failed\n");
        exit(1);
    }
    s = pthread_attr_setschedpolicy(&attr_master, SCHED_FIFO);
    if (s != 0) {
        printf("pthread_attr_setschedpolicy failed\n");
        exit(1);
    }
    // set priorities
    // prepare
    int max_priority = sched_get_priority_max(SCHED_FIFO);
    int mid_priority = max_priority - 1;
    int min_priority = max_priority - 2;
    struct sched_param param_follow;
    struct sched_param param_slave;
    struct sched_param param_master;
    param_follow.sched_priority = min_priority;
    param_slave.sched_priority = mid_priority;
    param_master.sched_priority = max_priority;
    // set
    // min
    s = pthread_attr_setschedparam(&attr_follow_1, &param_follow);
    if (s != 0) {
        printf("pthread_attr_setschedparam failed\n");
        exit(1);
    }
    s = pthread_attr_setschedparam(&attr_follow_2, &param_follow);
    if (s != 0) {
        printf("pthread_attr_setschedparam failed\n");
        exit(1);
    }
    // mid
    s = pthread_attr_setschedparam(&attr_slave, &param_slave);
    if (s != 0) {
        printf("pthread_attr_setschedparam failed\n");
        exit(1);
    }
    // max
    s = pthread_attr_setschedparam(&attr_master, &param_master);
    if (s != 0) {
        printf("pthread_attr_setschedparam failed\n");
        exit(1);
    }

    // run threads
    // Create Master Thread
    CALLNEW(pthread_create(&masterT, &attr_master, masterT_task, NULL));
    // Create Slave Thread
    CALLNEW(pthread_create(&slaveT, &attr_slave, slaveT_task, NULL));
    // Create Following Threads
    CALLNEW(pthread_create(&followingT1, &attr_follow_1, followingT1_task, NULL));
    CALLNEW(pthread_create(&followingT2, &attr_follow_2, followingT2_task, NULL));

    sleep(3);

    if (measure_stack) {
        size_t used_stack_follow_1 = determine_used_stacksize(&attr_follow_1);
        size_t used_stack_follow_2 = determine_used_stacksize(&attr_follow_2);
        size_t used_stack_slave = determine_used_stacksize(&attr_slave);
        size_t used_stack_master = determine_used_stacksize(&attr_master);
        printf("used stack by all deployed threads:\n\t- follow 1:\t%lu\n\t- follow 2:\t%lu\n\t- slave:\t%lu\n\t- master:\t%lu\n",
                used_stack_follow_1, used_stack_follow_2, used_stack_slave, used_stack_master);
        //printf("pthread_stack_min: \n");
    }

    // Wait for Threads to Finish
    CALLNEW(pthread_join(masterT, NULL));
    CALLNEW(pthread_join(slaveT, NULL);)
    CALLNEW(pthread_join(followingT1, NULL));
    CALLNEW(pthread_join(followingT2, NULL));

    // Cleanup
    sem_destroy(&master_slave_sem);
    sem_destroy(&slave_following_sem1);
    sem_destroy(&slave_following_sem2);
    pthread_attr_destroy(&attr_follow_1);
    pthread_attr_destroy(&attr_follow_2);
    pthread_attr_destroy(&attr_slave);
    pthread_attr_destroy(&attr_master);
}

