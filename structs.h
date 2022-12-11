#ifndef STRUCTS_H_
#define STRUCTS_H_

#include "so_scheduler.h"

enum Status
{
    NEW = 0,
    READY,
    WAITING,
    RUNNING,
    TERMINATED
};

typedef struct Thread
{
    enum Status status;
    int priority;
    so_handler *handler;
    int io;
    int time;
    tid_t tid;
    sem_t sem;
} Thread;

typedef struct queue
{
    Thread **container;
    int size;
    int capacity;
} queue;

typedef struct Scheduler
{
    int time;
    int io;
    queue *pq;
    queue *threads;
    Thread *running;
} Scheduler;

#endif /* STRUCTS_H_ */
