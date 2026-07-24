# CS 3502 Project 2 - CPU Scheduling Simulator

Chris Pham, Kennesaw State University

A CPU scheduling simulator written from scratch in C. It implements six
scheduling algorithms and reports the required performance metrics for
each one across nine different workloads.

## Algorithms

Baseline (equivalent to the provided starter code):

- **FCFS** - First Come First Served
- **SJF** - Shortest Job First, non-preemptive
- **Priority** - non-preemptive, lower number means higher priority
- **Round Robin** - quantum of 4

New algorithms added for this project:

- **SRTF** - Shortest Remaining Time First, the preemptive version of
  SJF. Re-evaluates every tick and preempts the running process when a
  shorter job arrives.
- **MLFQ** - Multi-Level Feedback Queue with three levels (RR quantum 4,
  RR quantum 8, then FCFS). Processes are demoted when they use a full
  quantum, and every 40 ticks all processes are boosted back to the top
  queue to prevent starvation.

## Metrics

Calculated identically for every algorithm:

- Average Waiting Time (turnaround - burst)
- Average Turnaround Time (completion - arrival)
- CPU Utilization ((makespan - idle) / makespan * 100)
- Throughput (processes / makespan)
- Average Response Time (first CPU - arrival)
- Context switch count

## Build

Requires gcc and make. No external libraries.

```
make
```

This produces the `scheduler` binary. To clean up:

```
make clean
```

## Run

```
./scheduler verify   # 4 process set that can be checked by hand
./scheduler demo     # small mixed workload with timelines
./scheduler edge     # edge cases: all arrive at 0, identical bursts,
                     # extreme burst mix, starvation scenario
./scheduler all      # all 6 algorithms on all 9 workloads (default),
                     # writes results.csv
```

`make run` is a shortcut for `./scheduler all`.

## Charts

The report figures are generated from `results.csv`:

```
python3 make_charts.py
```

Requires matplotlib. This step is only needed to rebuild the report
figures and is not required to run the simulator.

## Workloads

Three sizes (8, 30, and 120 processes) crossed with three types:

- **CPU-bound**: bursts of 20 to 60, slow arrivals
- **I/O-bound**: bursts of 1 to 5 with wider arrival spacing, since a
  process waiting on I/O is not runnable and appears to the CPU as idle
  time
- **Mixed**: about 30% long jobs among short ones

Workload generation uses a fixed seed, so every run produces identical
input and the numbers in the report are reproducible.

## Verification

`./scheduler verify` runs the 4 process example from the project
description and prints the hand calculated values next to the measured
ones. FCFS (AWT 8.75, ATT 15.25), SJF (7.75, 14.25), and SRTF (6.50,
13.00) all match exactly.

## Files

```
src/scheduler.h    PCB and Metrics structures, algorithm declarations
src/algorithms.c   all six scheduling algorithms
src/metrics.c      metric calculation and printing
src/workload.c     workload generation and edge case sets
src/main.c         driver and run modes
Makefile           build
make_charts.py     generates report figures from results.csv
```
