#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_PROCESSES 500
#define MAX_NAME 16

typedef struct {
    int pid;
    int arrival;
    int burst;
    int priority;

    int remaining;
    int first_run;
    int completion;
    int waiting;
    int turnaround;
    int response;

    int level;
    int quantum_used;
} PCB;

typedef struct {
    char   name[32];
    double avg_waiting;
    double avg_turnaround;
    double avg_response;
    double cpu_utilization;
    double throughput;
    int    makespan;
    int    context_switches;
    int    idle_time;
} Metrics;

typedef struct {
    int pid;
    int start;
    int end;
} GanttSlice;

#define MAX_SLICES 8000

typedef struct {
    GanttSlice slice[MAX_SLICES];
    int count;
} Gantt;

void run_fcfs    (PCB *p, int n, Metrics *m, Gantt *g);
void run_sjf     (PCB *p, int n, Metrics *m, Gantt *g);
void run_priority(PCB *p, int n, Metrics *m, Gantt *g);
void run_rr      (PCB *p, int n, Metrics *m, Gantt *g, int quantum);
void run_srtf    (PCB *p, int n, Metrics *m, Gantt *g);
void run_mlfq    (PCB *p, int n, Metrics *m, Gantt *g);

void reset_processes(PCB *p, int n);
void finalize_metrics(PCB *p, int n, Metrics *m,
                      int makespan, int idle, int switches);
void gantt_add(Gantt *g, int pid, int start, int end);
void print_gantt(const Gantt *g, int max_slices);
void print_process_table(const PCB *p, int n);
void print_metrics(const Metrics *m);

#endif
