#include <ucontext.h>

#ifndef TASK_H
#define TASK_H

typedef void (*task_function)(void);

enum state{
    READY,
    RUNNING,
    WAITING,
    TERMINATED,
};

struct task{
    ucontext_t new_task;
    char task_name[20];
    char func_name[20];
    enum state st;

    int prior;

    int waiting;
    int waiting_time;

    int tid;
    int running;

    int *resource;
    int resource_num;

    int turnaround;
    task_function function;
    
    struct task *next;
};

struct sche{
    struct task *t;
    int wait_time;
    int counting;
    struct sche *next;
};

struct task *first;
struct task *final;
struct sche *head;
struct sche *tail;
struct sche *scheduling;

ucontext_t cpu_idle;
ucontext_t init;
ucontext_t choose;

void task_sleep(int);
void task_exit();
void task_choose();
void task_judge();
void task_idle();

#endif
