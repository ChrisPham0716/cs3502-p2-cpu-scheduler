#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

static unsigned long rng_state = 12345;

static void rng_seed(unsigned long s) { rng_state = s; }

static int rng_range(int lo, int hi) {
    rng_state = rng_state * 6364136223846793005UL + 1442695040888963407UL;
    unsigned long v = (rng_state >> 33);
    return lo + (int)(v % (unsigned long)(hi - lo + 1));
}

int build_workload(PCB *p, int n, int type, unsigned long seed) {
    rng_seed(seed);
    int t = 0;

    for (int i = 0; i < n; i++) {
        p[i].pid      = i + 1;
        p[i].arrival  = t;
        p[i].priority = rng_range(1, 5);

        switch (type) {
        case 0:
            p[i].burst = rng_range(20, 60);
            t += rng_range(0, 12);
            break;
        case 1:

            p[i].burst = rng_range(1, 5);
            t += rng_range(0, 8);
            break;
        default:
            if (rng_range(1, 10) <= 3)
                p[i].burst = rng_range(25, 55);
            else
                p[i].burst = rng_range(1, 8);
            t += rng_range(0, 6);
            break;
        }
    }
    return n;
}

const char *workload_type_name(int type) {
    switch (type) {
    case 0:  return "CPU-bound";
    case 1:  return "IO-bound";
    default: return "Mixed";
    }
}

int build_reference_set(PCB *p) {
    int arrival[4]  = {0, 1, 2, 3};
    int burst[4]    = {8, 4, 9, 5};
    int priority[4] = {3, 1, 4, 2};

    for (int i = 0; i < 4; i++) {
        p[i].pid      = i + 1;
        p[i].arrival  = arrival[i];
        p[i].burst    = burst[i];
        p[i].priority = priority[i];
    }
    return 4;
}

int build_edge_all_zero(PCB *p) {
    int burst[6] = {7, 3, 12, 1, 9, 4};
    for (int i = 0; i < 6; i++) {
        p[i].pid = i + 1; p[i].arrival = 0;
        p[i].burst = burst[i]; p[i].priority = (i % 3) + 1;
    }
    return 6;
}

int build_edge_identical(PCB *p) {
    for (int i = 0; i < 6; i++) {
        p[i].pid = i + 1; p[i].arrival = i;
        p[i].burst = 5; p[i].priority = 2;
    }
    return 6;
}

int build_edge_extremes(PCB *p) {
    int burst[6]   = {100, 1, 2, 1, 120, 2};
    int arrival[6] = {0,   1, 2, 3, 4,   5};
    for (int i = 0; i < 6; i++) {
        p[i].pid = i + 1; p[i].arrival = arrival[i];
        p[i].burst = burst[i]; p[i].priority = (i % 4) + 1;
    }
    return 6;
}

int build_edge_starvation(PCB *p) {
    int n = 18;

    p[0].pid = 1; p[0].arrival = 0; p[0].burst = 5; p[0].priority = 1;

    p[1].pid = 2; p[1].arrival = 1; p[1].burst = 12; p[1].priority = 9;

    for (int i = 2; i < n; i++) {
        p[i].pid      = i + 1;
        p[i].arrival  = 2 + (i - 2) * 3;
        p[i].burst    = 4;
        p[i].priority = 1;
    }
    return n;
}
