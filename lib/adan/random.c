#include <stdlib.h>

void set_seed(int seed) {
    srand(seed);
}

int generate_random_int(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}

int generate_random_int_unrestrained() {
    return rand();
}