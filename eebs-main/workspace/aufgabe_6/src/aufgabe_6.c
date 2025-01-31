#include "aufgabe_6.h"

size_t ARM_32_DEFAULT_STACKSIZE = 16384;
uint64_t PATTERN = 0xAAAAAAAAAAAAAAAA;

void paint_stack(void* stackptr, size_t stacksize) {
    if (stacksize / 4 * 4 != stacksize) {
        printf("stack alignment problem\n");
        exit(1);
    }

    uint64_t* sp = (uint64_t*) stackptr;
    for (size_t i = 0; i < stacksize / 8; ++i) {
        *(sp + i) = PATTERN;
    }
}

void* find_watermark_bottom_up(void* stackptr, size_t stacksize) {
    bool print_debug_info = false;

    uint64_t* sp = (uint64_t*) stackptr;
    size_t stacksize_64 = stacksize / 8;

    for (size_t i = 0; i < stacksize_64; ++i) {
        // as long as the current 8-Byte-Block is equal to PATTERN, it was not used
        if (sp[i] == PATTERN) {
            continue;
        }

        // first used 8-Byte-Block found, test individual Bytes
        uint64_t mask = 0x00FFFFFFFFFFFFFF; // test lowest 7 Bytes
        if (print_debug_info) {
            printf("sp + i: %p\n", sp + i);
            printf("value: %lx\n", sp[i]);
        }
        for (size_t j = 0; j < 7; ++j) {
            if (print_debug_info) {
                printf("mask: %lx\n", mask);
            }
            if ((sp[i] & mask) == (PATTERN & mask)) {
                return ((char*)(sp + i)) + (8 - j - 1);
//                return index * 8 + j + 1; // upper 7 Bytes of last 8-Byte-Block unused --> i-1 8-Byte-Blocks plus j Byte(s) were used
            }
            mask >>= 8;
        }
        return sp + i;
    }
    // PATTERN was not found in the stack, therefore it is not
    return stackptr;
}

pthread_attr_t prepare_stack(size_t stacksize, bool measure_stack) {
    // define variables
    pthread_attr_t attr;
    int s;

    // Try it with different stacksize
    // allocate custom stack
    void* stackptr = malloc(stacksize);
    if (stackptr == NULL) {
        printf("malloc failed\n");
        exit(1);
    }

    if (measure_stack) {
        paint_stack(stackptr, stacksize);
    }

    // initialize attributes with default values
    s = pthread_attr_init(&attr);
    if (s != 0) {
        printf("pthread_attr_init failed\n");
        exit(1);
    }

    // set custom stack
    s = pthread_attr_setstack(&attr, stackptr, stacksize);
    if (s != 0) {
        printf("pthread_attr_setstack failed\n");
        exit(1);
    }

    return attr;
}

size_t determine_used_stacksize(pthread_attr_t* attr) {
    void* ptr;
    size_t size;
    int s;

    s = pthread_attr_getstack(attr, &ptr, &size);
    if (s != 0) {
        printf("pthread_attr_getstack failed\n");
        exit(1);
    }

    void* watermark = find_watermark_bottom_up(ptr, size);

    return size - (watermark - ptr);
}

void recurse(size_t depth) {
    if (depth % 10 == 0)
        printf("thread %lu with remaining recursions: %ld\n", pthread_self(), depth);

    if (!depth) {
        return;
    }

    recurse(depth - 1);
}

void print_attributes(pthread_attr_t* attr) {
    printf("Thread attributes:\n");
    void* stackptr;
    size_t stacksize;
    int s = pthread_attr_getstack(attr, &stackptr, &stacksize);
    if (s != 0) {
        printf("pthread_attr_getstack failed\n");
        exit(1);
    }
    printf("\tstack size:\t%ld\n", stacksize);
    printf("\tstack adress:\t%p\n", stackptr);
}

//static void* run(void *arg) {
//    // inspired by pthread_attr_init(3)'s manpage
//    pthread_attr_t gattr;
//
//    /* pthread_getattr_np() is a non-standard GNU extension that
//       retrieves the attributes of the thread specified in its
//       first argument. */
//    int s = pthread_getattr_np(pthread_self(), &gattr);
//    if (s != 0) {
//        printf("pthread_getattr_np failed\n");
//        exit(1);
//    }
//    print_attributes(&gattr);
//
//    size_t depth = *(size_t*)arg;
//    recurse(depth);
//
//    return NULL;
//}

//void run_custom_thread(size_t stacksize, size_t depth) {
//    if (stacksize < 16384) {
//        printf("WARNING: pthread_attr_setstack will fail for stacksizes < 16384\n");
//    }
//
//    // define variables
//    pthread_t thr;
//    pthread_attr_t attr;
//    int s;
//
//    // Try it with different stacksize
//    // allocate custom stack
//    void* stackptr = malloc(stacksize);
//    if (stackptr == NULL) {
//        printf("malloc failed\n");
//        exit(1);
//    }
//    paint_stack(stackptr, stacksize);
//
//    // initialize attributes with default values
//    s = pthread_attr_init(&attr);
//    if (s != 0) {
//        printf("pthread_attr_init failed\n");
//        exit(1);
//    }
//
//    // set custom stack
//    s = pthread_attr_setstack(&attr, stackptr, stacksize);
//    if (s != 0) {
//        printf("pthread_attr_setstack failed\n");
//        exit(1);
//    }
//
//    // run thread
//    s = pthread_create(&thr, &attr, run, &depth);
//    if (s != 0) {
//        printf("thread creation failed");
//        exit(1);
//    }
//
//    // destroy attr
//    s = pthread_attr_destroy(&attr);
//    if (s != 0) {
//        printf("pthread_attr_destroy failed");
//    }
//    pthread_join(thr, NULL);
//
//    // print the used stacksize
//    void* watermark_ptr = find_watermark_bottom_up(stackptr, stacksize);
//    printf("The stack used %ld Bytes\n", stacksize - (watermark_ptr - stackptr));
//}


// ##############################
// TESTS
// ##############################

void test_paint_stack() {
    size_t size = 16384;
    void* ptr = malloc(16384);

    if (ptr == NULL) {
        printf("malloc failed\n");
        exit(1);
    }

    paint_stack(ptr, size);

    char* byte_ptr = (char*)ptr;
    for (size_t i = 0; i < size; ++i) {
        if (byte_ptr[i] != (char)(PATTERN & 0xFF)) {
            printf("paint_stack incorrect at Byte: %ld\n", i);
            return;
        }
    }
    printf("paint_stack successful\n");
}

void test_find_watermark(size_t iterations) {
    size_t size = 16384;
    void* ptr = malloc(16384);

    if (ptr == NULL) {
        printf("malloc failed\n");
        exit(1);
    }

    paint_stack(ptr, size);

    char* byte_ptr = (char*)ptr;

    int correct = 0;
    int incorrect = 0;
    // use time as seed for random number generation
    srandom(time(NULL));
    for (size_t i = 0; i < iterations; ++i) {
        // use random index
        size_t use_index = random() % size;
        byte_ptr[use_index] = (PATTERN & 0xFF) + 1;

        void* result = find_watermark_bottom_up(ptr, size);
        if (result != (char*)ptr + use_index) {
            if (iterations < 20) {
                printf("find_watermark returned incorrect value,\n\tdetected value: %p\n\tcorrect value: %p\n", result, (char*)ptr + use_index);
            }
            ++incorrect;
        } else {
//            printf("find_watermark successful\n");
            ++correct;
        }

        // restore watermark
        byte_ptr[use_index] = PATTERN & 0xFF;
    }
    if (!incorrect) {
        printf("find_watermark was always correct\n");
    } else {
        printf("find_watermark was correct %f percent\n", (float)correct / (float)(incorrect + correct));
    }
}

void run_tests() {
    test_paint_stack();
    test_find_watermark(1000);

    exit(0);
}

