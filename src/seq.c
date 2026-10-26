#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"

static double now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(void) {
    long n = get_n();
    int *a = malloc(n * sizeof(int));
    int *sorted = malloc(n * sizeof(int));
    if (!a || !sorted) { printf("malloc error\n"); return 1; }

    for (long i = 0; i < n; i++) a[i] = gen(i);

    long hist[K] = {0};

    double t0 = now();

    // гистограмма
    for (long i = 0; i < n; i++)
        hist[a[i]]++;

    // сортировка подсчётом: каждое значение v пишем hist[v] раз
    long pos = 0;
    for (int v = 0; v < K; v++)
        for (long j = 0; j < hist[v]; j++)
            sorted[pos++] = v;

    double t = now() - t0;

    printf("seq;1;%.6f;%s\n", t, check(checksum(sorted, n), n, 1));

    free(a);
    free(sorted);
    return 0;
}
