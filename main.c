#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

#define RR_QUANTUM 4

int  build_workload(PCB *p, int n, int type, unsigned long seed);
const char *workload_type_name(int type);
int  build_reference_set(PCB *p);
int  build_edge_all_zero(PCB *p);
int  build_edge_identical(PCB *p);
int  build_edge_extremes(PCB *p);
int  build_edge_starvation(PCB *p);

static PCB master[MAX_PROCESSES];
static PCB work[MAX_PROCESSES];
static Gantt gantt;

static void run_all(PCB *src, int n, Metrics *results, int show_tables) {
    void (*np[3])(PCB *, int, Metrics *, Gantt *) = {
        run_fcfs, run_sjf, run_priority
    };
    const char *labels[3] = { "FCFS", "SJF", "Priority" };

    int r = 0;
    for (int a = 0; a < 3; a++) {
        memcpy(work, src, sizeof(PCB) * n);
        gantt.count = 0;
        np[a](work, n, &results[r], &gantt);
        if (show_tables) {
            printf("\n--- %s ---\n", labels[a]);
            print_process_table(work, n);
            print_gantt(&gantt, 12);
        }
        r++;
    }

    memcpy(work, src, sizeof(PCB) * n);
    gantt.count = 0;
    run_rr(work, n, &results[r], &gantt, RR_QUANTUM);
    if (show_tables) {
        printf("\n--- Round Robin (quantum %d) ---\n", RR_QUANTUM);
        print_process_table(work, n);
        print_gantt(&gantt, 12);
    }
    r++;

    memcpy(work, src, sizeof(PCB) * n);
    gantt.count = 0;
    run_srtf(work, n, &results[r], &gantt);
    if (show_tables) {
        printf("\n--- SRTF (new) ---\n");
        print_process_table(work, n);
        print_gantt(&gantt, 12);
    }
    r++;

    memcpy(work, src, sizeof(PCB) * n);
    gantt.count = 0;
    run_mlfq(work, n, &results[r], &gantt);
    if (show_tables) {
        printf("\n--- MLFQ (new) ---\n");
        print_process_table(work, n);
        print_gantt(&gantt, 12);
    }
}

static void print_result_block(Metrics *results) {
    for (int i = 0; i < 6; i++) print_metrics(&results[i]);
}

static void mode_verify(void) {
    Metrics results[6];
    int n = build_reference_set(master);

    printf("========================================================\n");
    printf(" VERIFICATION: 4 process set from the project handout\n");
    printf(" P1(a=0,b=8,pri=3) P2(a=1,b=4,pri=1)\n");
    printf(" P3(a=2,b=9,pri=4) P4(a=3,b=5,pri=2)\n");
    printf("========================================================\n");

    run_all(master, n, results, 1);

    printf("\n--- metric summary ---\n");
    print_result_block(results);

    printf("\nHand-checked expectations:\n");
    printf("  FCFS: AWT should be 8.75, ATT 15.25\n");
    printf("  SJF : AWT should be 7.75, ATT 14.25\n");
    printf("  SRTF: AWT should be 6.50, ATT 13.00\n");
}

static void mode_demo(void) {
    Metrics results[6];
    int n = build_workload(master, 8, 2, 2026);

    printf("========================================================\n");
    printf(" DEMO: small mixed workload, 8 processes\n");
    printf("========================================================\n");
    printf("  input processes:\n");
    for (int i = 0; i < n; i++)
        printf("    P%d arrival=%d burst=%d priority=%d\n",
               master[i].pid, master[i].arrival,
               master[i].burst, master[i].priority);

    run_all(master, n, results, 1);
    printf("\n--- metric summary ---\n");
    print_result_block(results);
}

static void mode_edge(void) {
    Metrics results[6];
    struct { const char *name; int (*build)(PCB *); } cases[] = {
        { "all processes arrive at time 0", build_edge_all_zero  },
        { "identical burst times",          build_edge_identical },
        { "extreme burst mix (1 vs 120)",   build_edge_extremes  },
        { "priority starvation scenario",   build_edge_starvation}
    };

    for (int c = 0; c < 4; c++) {
        int n = cases[c].build(master);
        printf("\n========================================================\n");
        printf(" EDGE CASE: %s\n", cases[c].name);
        printf("========================================================\n");
        for (int i = 0; i < n; i++)
            printf("  P%d arrival=%d burst=%d priority=%d\n",
                   master[i].pid, master[i].arrival,
                   master[i].burst, master[i].priority);
        printf("\n");
        run_all(master, n, results, 0);
        print_result_block(results);

        if (c == 3) {

            Metrics tmp;
            int done_pri, done_mlfq, first_pri, first_mlfq;

            memcpy(work, master, sizeof(PCB) * n);
            gantt.count = 0;
            run_priority(work, n, &tmp, &gantt);
            done_pri = work[1].completion; first_pri = work[1].first_run;

            memcpy(work, master, sizeof(PCB) * n);
            gantt.count = 0;
            run_mlfq(work, n, &tmp, &gantt);
            done_mlfq = work[1].completion; first_mlfq = work[1].first_run;

            printf("\n  Victim process P2 (arrives t=1, worst priority,\n");
            printf("  longest burst). CPU is saturated, so total work is\n");
            printf("  fixed and the victim finishes last either way:\n");
            printf("    under Priority: first CPU at t=%d, finishes at %d\n",
                   first_pri, done_pri);
            printf("    under MLFQ:     first CPU at t=%d, finishes at %d\n",
                   first_mlfq, done_mlfq);
            printf("  The difference is responsiveness, not completion.\n");
            printf("  Priority makes it wait %d units before it runs at all,\n",
                   first_pri - 1);
            printf("  while MLFQ gives it a slice after %d units because every\n",
                   first_mlfq - 1);
            printf("  new process starts at the top queue regardless of its\n");
            printf("  priority field. A process that is ignored for %d units\n",
                   first_pri - 1);
            printf("  is starving even if the final average looks acceptable.\n");
        }
    }
}

static void mode_all(void) {
    const int sizes[3] = { 8, 30, 120 };
    const char *size_names[3] = { "small", "medium", "large" };
    Metrics results[6];

    FILE *csv = fopen("results.csv", "w");
    if (!csv) { perror("results.csv"); return; }
    fprintf(csv, "workload_size,n,workload_type,algorithm,avg_waiting,"
                 "avg_turnaround,avg_response,cpu_utilization,throughput,"
                 "context_switches,makespan\n");

    for (int s = 0; s < 3; s++) {
        for (int type = 0; type < 3; type++) {
            int n = build_workload(master, sizes[s], type, 2026 + s * 10 + type);

            printf("\n========================================================\n");
            printf(" %s workload, %d processes, %s\n",
                   size_names[s], n, workload_type_name(type));
            printf("========================================================\n");

            run_all(master, n, results, 0);
            print_result_block(results);

            for (int i = 0; i < 6; i++) {
                fprintf(csv, "%s,%d,%s,%s,%.4f,%.4f,%.4f,%.4f,%.6f,%d,%d\n",
                        size_names[s], n, workload_type_name(type),
                        results[i].name, results[i].avg_waiting,
                        results[i].avg_turnaround, results[i].avg_response,
                        results[i].cpu_utilization, results[i].throughput,
                        results[i].context_switches, results[i].makespan);
            }
        }
    }
    fclose(csv);
    printf("\nresults.csv written (%d rows).\n", 3 * 3 * 6);
}

int main(int argc, char **argv) {
    const char *mode = argc > 1 ? argv[1] : "all";

    if      (!strcmp(mode, "verify")) mode_verify();
    else if (!strcmp(mode, "demo"))   mode_demo();
    else if (!strcmp(mode, "edge"))   mode_edge();
    else if (!strcmp(mode, "all"))    mode_all();
    else {
        printf("usage: %s [verify|demo|edge|all]\n", argv[0]);
        return 1;
    }
    return 0;
}
