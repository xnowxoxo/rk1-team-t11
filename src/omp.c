#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "common.h"

// два варианта, выбираются через OMP_MODE:
//   atomic - общая гистограмма, hist[a[i]]++ под #pragma omp atomic
//   local  - у каждого потока своя гистограмма, потом складываем
int main(void) {
    long n = get_n();
    int *a = malloc(n * sizeof(int));
    int *sorted = malloc(n * sizeof(int));
    if (!a || !sorted) { printf("malloc error\n"); return 1; }

    for (long i = 0; i < n; i++) a[i] = gen(i);

    char *mode = getenv("OMP_MODE");
    if (!mode) mode = "local";

    long hist[K] = {0};

    double t0 = omp_get_wtime();

    if (strcmp(mode, "atomic") == 0) {
        // TODO
        for (long i = 0; i < n; i++)
            hist[a[i]]++;
    } else {
        // TODO
        for (long i = 0; i < n; i++)
            hist[a[i]]++;
    }

    // TODO: сортировку тоже можно распараллелить
    long pos = 0;
    for (int v = 0; v < K; v++)
        for (long j = 0; j < hist[v]; j++)
            sorted[pos++] = v;

    double t = omp_get_wtime() - t0;

    printf("omp_%s;%d;%.6f;%s\n", mode, omp_get_max_threads(), t,
           check(checksum(sorted, n), n, 0));

    free(a);
    free(sorted);
    return 0;
}
