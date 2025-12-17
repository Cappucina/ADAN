#include <stdlib.h>
#include <time.h>

void set_seed(int seed) {
    srand(seed);
}

void set_seed_as_current_time() {
    srand(time(NULL));
}

int generate_random_int(int min, int max) {
    max++;

    if (min > max) {
        int tmp = min;
        min = max;
        max = tmp;
    }

    return (rand() % (max - min + 1)) + min;
}

int generate_random_int_unrestrained() {
    // return rand();

    // I got segmentation faults when doing this needs to be limmited

    return generate_random_int(0, 1024);
}