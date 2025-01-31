#include "aufgabe_3_4.h"

double clocks_to_ms(size_t clocks) {
    return (double)clocks / (double)CLOCKS_PER_SEC * 1000;
}

// Function that runs an empty loop to consume CPU time
void waste_time(size_t count) {
    // Executes 'count' iterations without performing any operations
    volatile size_t i = 0;
    while (i < count) {
        ++i;
    }
//    for(size_t i = 0; i < count; ++i) {}
}

// Test function: Runs waste_time with a large iteration count and measures the runtime
void test() {
    clock_t start = clock();
    waste_time(1700000000);
    clock_t end = clock();
    printf("elapsed %ld, cps %ld\n", (end - start), CLOCKS_PER_SEC);
    printf("time: %lf\n", (double)(end - start) / (double)CLOCKS_PER_SEC * (double)1000.0);
}

