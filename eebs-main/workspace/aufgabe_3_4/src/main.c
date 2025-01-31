#include "aufgabe_3_4.h"

size_t calibrated_iterations;

void* run_calibration(void* ms) {
    calibrated_iterations = calibrate(*((size_t*)ms));
    pthread_exit(NULL);
}

void calibrate_max_priority(size_t ms) {
    printf("running calibration for %lu ms\n", ms);
    // initualize pthread_attr_t
    pthread_attr_t attr;
    int s = pthread_attr_init(&attr);
    if (s != 0) {
        printf("pthread_attr_init failed\n");
        exit(1);
    }
    s = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (s != 0) {
        printf("pthread_attr_setinheritsched failed\n");
        exit(1);
    }
    s = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if (s != 0) {
        printf("pthread_attr_setschedpolicy failed\n");
        exit(1);
    }

    // set scheduling priority
    int max_priority = sched_get_priority_max(SCHED_FIFO);
    struct sched_param param;
    param.sched_priority = max_priority;
    s = pthread_attr_setschedparam(&attr, &param);
    if (s != 0) {
        printf("pthread_attr_setschedparam failed\n");
        exit(1);
    }

    // run thread
    pthread_t thr;
    s = pthread_create(&thr, &attr, run_calibration, &ms);
    if (s != 0) {
        switch (s) {
            case EAGAIN:
                printf("pthread_create failed with EAGAIN\n");
                exit(1);
            case EINVAL :
                printf("pthread_create failed with EINVAL \n");
                exit(1);
            case EPERM:
                printf("pthread_create failed with EPERM\n");
                exit(1);
            default:
                printf("pthread_create failed with unknown error: %d\n", s);
                exit(1);
        }
    }

    pthread_join(thr, NULL);
    printf("iterations calibrated for %lu ms: %lu\n", ms, calibrated_iterations);
}

int main(int argc, char** argv) {
    // run the scheduling simulation
    run(1, 0);
	return EXIT_SUCCESS;
}
