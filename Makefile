# CS 3502 Project 2 - CPU Scheduling Simulator
CC      = gcc
CFLAGS  = -Wall -Wextra -O2
SRC     = src/main.c src/algorithms.c src/metrics.c src/workload.c
TARGET  = scheduler

all: $(TARGET)

$(TARGET): $(SRC) src/scheduler.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) all

clean:
	rm -f $(TARGET) results.csv

.PHONY: all run clean
