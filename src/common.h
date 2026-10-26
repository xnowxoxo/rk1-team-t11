#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define K 1024   // значения от 0 до K-1

// размер массива, по умолчанию 5*10^8
// для теста можно поменять: HIST_N=1000000 ./bin/seq
static inline long get_n(void) {
    char *s = getenv("HIST_N");
    return s ? atol(s) : 500000000L;
}

// i-й элемент массива, во всех версиях одинаковый
static inline int gen(long i) {
    uint32_t x = (uint32_t)i * 2654435761u;
    x ^= x >> 15;
    x *= 2246822519u;
    x ^= x >> 13;
    return x % K;
}

// контрольная сумма отсортированного массива
static inline uint64_t checksum(const int *s, long n) {
    uint64_t sum = 0;
    for (long i = 0; i < n; i++)
        sum += (uint64_t)s[i] * (uint64_t)(i + 1);
    return sum;
}

// seq записывает эталон в ref_<n>.txt, omp и mpi с ним сравнивают
static inline const char *check(uint64_t sum, long n, int is_seq) {
    char path[64];
    snprintf(path, sizeof path, "ref_%ld.txt", n);
    FILE *f = fopen(path, "r");
    if (!f) {
        if (!is_seq) return "NOREF";
        f = fopen(path, "w");
        if (!f) return "NOREF";
        fprintf(f, "%llu\n", (unsigned long long)sum);
        fclose(f);
        return "OK";
    }
    unsigned long long ref = 0;
    int ok = fscanf(f, "%llu", &ref) == 1;
    fclose(f);
    return (ok && ref == sum) ? "OK" : "FAIL";
}

#endif
