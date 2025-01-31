#include "aufgabe_3_4.h"

#include <math.h>

// Calibration function: Determines how many iterations are needed to consume a desired amount of time
size_t calibrate(size_t desired_ms) {
    // Initial estimate of the iteration count for the desired time in milliseconds
    size_t count = desired_ms * 1700000; // Assumption: 1 ms corresponds to about 1,700,000 iterations
    clock_t start;
    clock_t end;
    // Repeats until the desired time is matched
    while(true) {
        start = clock();   // Capture the start time
        waste_time(count); // Consume CPU time
        end = clock();     // Capture the end time
        // Calculate the elapsed time in milliseconds
        double elapsed_ms = clocks_to_ms(end - start);
        // Calculate the factor to adjust the number of iterations by
        double factor = (double)desired_ms / (double)elapsed_ms;
        // If the deviation is small enough, terminate
        if (fabs(factor - 1.0) < 0.0001) { // 0.0001 ~ 0.1ms tolerance
            break;
        }
        // Adjust the iteration count based on the correction factor
        count *= factor;
    }
    printf("Most accurate time: %lfms, achieved with %ld iterations.\n", clocks_to_ms(end - start), count);
    return count;
}

