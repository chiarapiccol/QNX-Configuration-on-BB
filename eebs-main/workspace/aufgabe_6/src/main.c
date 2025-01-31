#include "aufgabe_6.h"

void run_custom_thread(size_t stacksize, size_t depth);

int main(int argc, char** argv) {
//    run_tests();

    size_t depth = 10;
    if (argc > 1) {
        depth = strtoul(argv[1], NULL, 0);
    }
    printf("depth: %ld\n", depth);

//    run_normal_thread(depth);
    //run_custom_thread(16384, depth); // fails for recursion-depths >= 700 with stacksize=16384
    // incrementing the depth by 1 increases the stack-usage by 16 Byte
}
