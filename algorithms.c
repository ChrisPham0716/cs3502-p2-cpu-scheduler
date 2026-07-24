#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "scheduler.h"

static int next_arrival(PCB *p, int n, int time) {
    int best = INT_MAX;
    for (int i = 0; i < n; i++)
        if (p[i].remaining > 0 && p[i].arrival > time && p[i].arrival < best)
            best = p[i].arrival;
    return best == INT_MAX ? -1 : best;
}

void run_fcfs(PCB *p, int n, Metrics *m, Gantt *g) {
    reset_processes(p, n);
    strcpy(m->name, "FCFS");

    int time = 0, completed = 0, idle = 0, switches = 0, last = -1;

    while (completed < n) {
        int best = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].remaining > 0 && p[i].arrival <= time) {
                if (best == -1 || p[i].arrival < p[best].arrival ||
                   (p[i].arrival == p[best].arrival && p[i].pid < p[best].pid))
                    best = i;
            }
        }
        if (best == -1) {
            int na = next_arrival(p, n, time);
            if (na < 0) break;
            gantt_add(g, -1, time, na);
            idle += na - time;
            time = na;
            continue;
        }
        if (p[best].first_run < 0) p[best].first_run = time;
        if (last != best) { switches++; last = best; }

        gantt_add(g, p[best].pid, time, time + p[best].burst);
        time += p[best].burst;
        p[best].remaining  = 0;
        p[best].completion = time;
        completed++;
    }
    finalize_metrics(p, n, m, time, idle, switches);
}

void run_sjf(PCB *p, int n, Metrics *m, Gantt *g) {
    reset_processes(p, n);
    strcpy(m->name, "SJF");

    int time = 0, completed = 0, idle = 0, switches = 0, last = -1;

    while (completed < n) {
        int best = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].remaining > 0 && p[i].arrival <= time) {
                if (best == -1 || p[i].burst < p[best].burst ||
                   (p[i].burst == p[best].burst && p[i].arrival < p[best].arrival))
                    best = i;
            }
        }
        if (best == -1) {
            int na = next_arrival(p, n, time);
            if (na < 0) break;
            gantt_add(g, -1, time, na);
            idle += na - time;
            time = na;
            continue;
        }
        if (p[best].first_run < 0) p[best].first_run = time;
        if (last != best) { switches++; last = best; }

        gantt_add(g, p[best].pid, time, time + p[best].burst);
        time += p[best].burst;
        p[best].remaining  = 0;
        p[best].completion = time;
        completed++;
    }
    finalize_metrics(p, n, m, time, idle, switches);
}

void run_priority(PCB *p, int n, Metrics *m, Gantt *g) {
    reset_processes(p, n);
    strcpy(m->name, "Priority");

    int time = 0, completed = 0, idle = 0, switches = 0, last = -1;

    while (completed < n) {
        int best = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].remaining > 0 && p[i].arrival <= time) {
                if (best == -1 || p[i].priority < p[best].priority ||
                   (p[i].priority == p[best].priority &&
                    p[i].arrival  < p[best].arrival))
                    best = i;
            }
        }
        if (best == -1) {
            int na = next_arrival(p, n, time);
            if (na < 0) break;
            gantt_add(g, -1, time, na);
            idle += na - time;
            time = na;
            continue;
        }
        if (p[best].first_run < 0) p[best].first_run = time;
        if (last != best) { switches++; last = best; }

        gantt_add(g, p[best].pid, time, time + p[best].burst);
        time += p[best].burst;
        p[best].remaining  = 0;
        p[best].completion = time;
        completed++;
    }
    finalize_metrics(p, n, m, time, idle, switches);
}

void run_rr(PCB *p, int n, Metrics *m, Gantt *g, int quantum) {
    reset_processes(p, n);
    sprintf(m->name, "RR (q=%d)", quantum);

    int queue[MAX_PROCESSES * 4];
    int head = 0, tail = 0;
    int admitted[MAX_PROCESSES] = {0};

    int time = 0, completed = 0, idle = 0, switches = 0, last = -1;
    int cur = -1, used = 0;

    while (completed < n) {

        for (int i = 0; i < n; i++)
            if (!admitted[i] && p[i].arrival <= time) {
                queue[tail++] = i; admitted[i] = 1;
            }

        if (cur == -1) {
            if (head < tail) {
                cur  = queue[head++];
                used = 0;
                if (p[cur].first_run < 0) p[cur].first_run = time;
                if (last != cur) { switches++; last = cur; }
            } else {
                int na = next_arrival(p, n, time);
                if (na < 0) break;
                gantt_add(g, -1, time, na);
                idle += na - time;
                time = na;
                continue;
            }
        }

        gantt_add(g, p[cur].pid, time, time + 1);
        p[cur].remaining--;
        used++;
        time++;

        for (int i = 0; i < n; i++)
            if (!admitted[i] && p[i].arrival <= time) {
                queue[tail++] = i; admitted[i] = 1;
            }

        if (p[cur].remaining == 0) {
            p[cur].completion = time;
            completed++;
            cur = -1;
        } else if (used >= quantum) {
            queue[tail++] = cur;
            cur = -1;
        }
    }
    finalize_metrics(p, n, m, time, idle, switches);
}

void run_srtf(PCB *p, int n, Metrics *m, Gantt *g) {
    reset_processes(p, n);
    strcpy(m->name, "SRTF");

    int time = 0, completed = 0, idle = 0, switches = 0, last = -1;

    while (completed < n) {

        int best = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].remaining > 0 && p[i].arrival <= time) {
                if (best == -1 ||
                    p[i].remaining < p[best].remaining ||
                   (p[i].remaining == p[best].remaining &&
                    p[i].arrival   < p[best].arrival)  ||
                   (p[i].remaining == p[best].remaining &&
                    p[i].arrival  == p[best].arrival &&
                    p[i].pid       < p[best].pid))
                    best = i;
            }
        }

        if (best == -1) {
            int na = next_arrival(p, n, time);
            if (na < 0) break;
            gantt_add(g, -1, time, na);
            idle += na - time;
            time = na;
            continue;
        }

        if (p[best].first_run < 0) p[best].first_run = time;
        if (last != best) { switches++; last = best; }

        gantt_add(g, p[best].pid, time, time + 1);
        p[best].remaining--;
        time++;

        if (p[best].remaining == 0) {
            p[best].completion = time;
            completed++;
        }
    }
    finalize_metrics(p, n, m, time, idle, switches);
}

#define MLFQ_LEVELS 3
#define BOOST_INTERVAL 40

static const int mlfq_quantum[MLFQ_LEVELS] = { 4, 8, 999999 };

void run_mlfq(PCB *p, int n, Metrics *m, Gantt *g) {
    reset_processes(p, n);
    strcpy(m->name, "MLFQ");

    int q[MLFQ_LEVELS][MAX_PROCESSES * 4];
    int head[MLFQ_LEVELS] = {0}, tail[MLFQ_LEVELS] = {0};
    int admitted[MAX_PROCESSES] = {0};

    int time = 0, completed = 0, idle = 0, switches = 0, last = -1;
    int cur = -1;

    while (completed < n) {

        for (int i = 0; i < n; i++)
            if (!admitted[i] && p[i].arrival <= time) {
                p[i].level = 0;
                q[0][tail[0]++] = i;
                admitted[i] = 1;
            }

        if (time > 0 && time % BOOST_INTERVAL == 0) {
            int rescued[MAX_PROCESSES], count = 0;
            for (int lv = 1; lv < MLFQ_LEVELS; lv++) {
                while (head[lv] < tail[lv]) {
                    int idx = q[lv][head[lv]++];
                    if (p[idx].remaining > 0) rescued[count++] = idx;
                }
                head[lv] = tail[lv] = 0;
            }
            for (int i = 0; i < count; i++) {
                p[rescued[i]].level = 0;
                p[rescued[i]].quantum_used = 0;
                q[0][tail[0]++] = rescued[i];
            }
            if (cur != -1) { p[cur].level = 0; p[cur].quantum_used = 0; }
        }

        if (cur != -1) {
            for (int lv = 0; lv < p[cur].level; lv++) {
                if (head[lv] < tail[lv]) {
                    q[p[cur].level][tail[p[cur].level]++] = cur;
                    p[cur].quantum_used = 0;
                    cur = -1;
                    break;
                }
            }
        }

        if (cur == -1) {
            for (int lv = 0; lv < MLFQ_LEVELS && cur == -1; lv++)
                if (head[lv] < tail[lv]) cur = q[lv][head[lv]++];

            if (cur == -1) {
                int na = next_arrival(p, n, time);
                if (na < 0) break;
                gantt_add(g, -1, time, na);
                idle += na - time;
                time = na;
                continue;
            }
            p[cur].quantum_used = 0;
            if (p[cur].first_run < 0) p[cur].first_run = time;
            if (last != cur) { switches++; last = cur; }
        }

        gantt_add(g, p[cur].pid, time, time + 1);
        p[cur].remaining--;
        p[cur].quantum_used++;
        time++;

        for (int i = 0; i < n; i++)
            if (!admitted[i] && p[i].arrival <= time) {
                p[i].level = 0;
                q[0][tail[0]++] = i;
                admitted[i] = 1;
            }

        if (p[cur].remaining == 0) {
            p[cur].completion = time;
            completed++;
            cur = -1;
        } else if (p[cur].quantum_used >= mlfq_quantum[p[cur].level]) {

            if (p[cur].level < MLFQ_LEVELS - 1) p[cur].level++;
            p[cur].quantum_used = 0;
            q[p[cur].level][tail[p[cur].level]++] = cur;
            cur = -1;
        }
    }
    finalize_metrics(p, n, m, time, idle, switches);
}
