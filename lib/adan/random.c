#include <stdlib.h>
#include <time.h>

void set_seed(int seed) {
    srand(seed);
}

int randint(int min, int max) {
    if (min > max) {
        int tmp = min;
        min = max;
        max = tmp;
    }

    return (rand() % (max - min + 1)) + min;
}