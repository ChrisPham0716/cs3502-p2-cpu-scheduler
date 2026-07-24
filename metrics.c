#include <stdio.h>
#include <string.h>
#include "scheduler.h"

void reset_processes(PCB *p, int n) {
    for (int i = 0; i < n; i++) {
        p[i].remaining    = p[i].burst;
        p[i].first_run    = -1;
        p[i].completion   = 0;
        p[i].waiting      = 0;
        p[i].turnaround   = 0;
        p[i].response     = 0;
        p[i].level        = 0;
        p[i].quantum_used = 0;
    }
}

void finalize_metrics(PCB *p, int n, Metrics *m,
                      int makespan, int idle, int switches) {
    long total_wait = 0, total_tat = 0, total_resp = 0;

    for (int i = 0; i < n; i++) {
        p[i].turnaround = p[i].completion - p[i].arrival;
        p[i].waiting    = p[i].turnaround - p[i].burst;
        p[i].response   = p[i].first_run  - p[i].arrival;

        if (p[i].waiting  < 0) p[i].waiting  = 0;
        if (p[i].response < 0) p[i].response = 0;

        total_wait += p[i].waiting;
        total_tat  += p[i].turnaround;
        total_resp += p[i].response;
    }

    m->avg_waiting    = (double)total_wait / n;
    m->avg_turnaround = (double)total_tat  / n;
    m->avg_response   = (double)total_resp / n;
    m->makespan       = makespan;
    m->idle_time      = idle;
    m->context_switches = switches;

    m->cpu_utilization = makespan > 0
        ? 100.0 * (double)(makespan - idle) / (double)makespan
        : 0.0;

    m->throughput = makespan > 0 ? (double)n / (double)makespan : 0.0;
}

void gantt_add(Gantt *g, int pid, int start, int end) {
    if (!g || end <= start) return;

    if (g->count > 0 && g->slice[g->count - 1].pid == pid
                     && g->slice[g->count - 1].end == start) {
        g->slice[g->count - 1].end = end;
        return;
    }
    if (g->count < MAX_SLICES) {
        g->slice[g->count].pid   = pid;
        g->slice[g->count].start = start;
        g->slice[g->count].end   = end;
        g->count++;
    }
}

void print_gantt(const Gantt *g, int max_slices) {
    if (!g) return;
    int limit = g->count < max_slices ? g->count : max_slices;

    printf("  timeline: ");
    for (int i = 0; i < limit; i++) {
        if (g->slice[i].pid < 0)
            printf("[idle %d-%d]", g->slice[i].start, g->slice[i].end);
        else
            printf("[P%d %d-%d]", g->slice[i].pid,
                   g->slice[i].start, g->slice[i].end);
    }
    if (g->count > limit) printf(" ... (%d more)", g->count - limit);
    printf("\n");
}

void print_process_table(const PCB *p, int n) {
    printf("  %-5s %-8s %-6s %-5s %-6s %-8s %-6s %-5s\n",
           "PID", "Arrival", "Burst", "Pri", "Start", "Complete",
           "TAT", "Wait");
    for (int i = 0; i < n; i++) {
        printf("  P%-4d %-8d %-6d %-5d %-6d %-8d %-6d %-5d\n",
               p[i].pid, p[i].arrival, p[i].burst, p[i].priority,
               p[i].first_run, p[i].completion,
               p[i].turnaround, p[i].waiting);
    }
}

void print_metrics(const Metrics *m) {
    printf("  %-22s AWT=%7.2f  ATT=%7.2f  RESP=%7.2f  CPU=%6.2f%%  "
           "THRU=%6.4f  CS=%d\n",
           m->name, m->avg_waiting, m->avg_turnaround, m->avg_response,
           m->cpu_utilization, m->throughput, m->context_switches);
}
