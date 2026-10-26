#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    long n = get_n();

    // сколько элементов у каждого процесса, остаток раздаём первым
    int *counts = malloc(p * sizeof(int));
    int *displs = malloc(p * sizeof(int));
    int base = n / p, rem = n % p;
    for (int i = 0; i < p; i++) {
        counts[i] = base + (i < rem ? 1 : 0);
        displs[i] = (i == 0) ? 0 : displs[i - 1] + counts[i - 1];
    }
    int my_n = counts[rank];
    long lo = displs[rank], hi = lo + my_n;   // мой кусок в отсортированном массиве

    int *a = NULL;
    if (rank == 0) {
        a = malloc(n * sizeof(int));
        if (!a) { printf("malloc error\n"); MPI_Abort(MPI_COMM_WORLD, 1); }
        for (long i = 0; i < n; i++) a[i] = gen(i);
    }
    int *part = malloc(my_n * sizeof(int));

    long loc[K] = {0}, hist[K] = {0};

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    // 1. раздаём массив
    MPI_Scatterv(a, counts, displs, MPI_INT,
                 part, my_n, MPI_INT, 0, MPI_COMM_WORLD);
    double t1 = MPI_Wtime();

    // 2. своя гистограмма
    for (int i = 0; i < my_n; i++)
        loc[part[i]]++;
    double t2 = MPI_Wtime();

    // 3. общая гистограмма: складываем на rank 0 и рассылаем всем
    MPI_Reduce(loc, hist, K, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Bcast(hist, K, MPI_LONG, 0, MPI_COMM_WORLD);
    double t3 = MPI_Wtime();

    // 4. сортировка подсчётом: каждый заполняет свой кусок [lo, hi)
    //    значение v в отсортированном массиве стоит на позициях [start, start + hist[v])
    long start = 0;
    for (int v = 0; v < K && start < hi; v++) {
        long end = start + hist[v];
        long from = start > lo ? start : lo;
        long to = end < hi ? end : hi;
        for (long j = from; j < to; j++)
            part[j - lo] = v;
        start = end;
    }
    double t4 = MPI_Wtime();

    // проверка: считаем контрольную сумму по кускам и складываем
    uint64_t s = 0, sum = 0;
    for (int j = 0; j < my_n; j++)
        s += (uint64_t)part[j] * (uint64_t)(lo + j + 1);
    MPI_Reduce(&s, &sum, 1, MPI_UINT64_T, MPI_SUM, 0, MPI_COMM_WORLD);

    double my[5] = {t1 - t0, t2 - t1, t3 - t2, t4 - t3, t4 - t0};
    double mx[5], mn_hist;
    MPI_Reduce(my, mx, 5, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&my[1], &mn_hist, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("mpi;%d;%.6f;%s\n", p, mx[4], check(sum, n, 0));
        // для анализа: сколько ушло на каждый этап
        printf("#detail;mpi;%d;scatter=%.6f;hist_min=%.6f;hist_max=%.6f;reduce_bcast=%.6f;sort=%.6f\n",
               p, mx[0], mn_hist, mx[1], mx[2], mx[3]);
        free(a);
    }
    free(part);
    free(counts);
    free(displs);
    MPI_Finalize();
    return 0;
}
