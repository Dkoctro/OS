#include "../include/resource.h"
#include <stdio.h>
#include <stdlib.h>
#include "../include/task.h"
#include <ucontext.h>

bool res_ava[8] = {false, false, false, false, false, false, false, false};

void get_resources(int count, int *resources)
{
    bool get = true;
    scheduling->t->resource_num = scheduling->counting;
    scheduling->t->waiting = 0;

    for(int i = scheduling->counting; i < (scheduling->counting + count); i++){
        if(res_ava[resources[i]]){
            get = false;
        }
        scheduling->t->resource[i] = resources[i - (scheduling->counting)];
    }

    if(get){
        printf("Task %s gets resources", scheduling->t->task_name);
        fflush(stdout);
        for(int i = 0; i < count; i++){
            printf(" %d", resources[i]);
            fflush(stdout);
            res_ava[resources[i]] = true;
        }
        printf("\n");
        fflush(stdout);
        scheduling->t->resource_num += count;
        scheduling->counting = scheduling->t->resource_num;
    } else {
        scheduling->t->waiting = 1;
        scheduling->t->running++;
        scheduling->t->resource_num += count;
        scheduling->t->st = WAITING;
        printf("Task %s is waiting resource.\n", scheduling->t->task_name);
        fflush(stdout);
        setcontext(&cpu_idle);
    }
}

void release_resources(int count, int *resources)
{
    for(int i = 0; i < count; i++){
        res_ava[resources[i]] = false;
        scheduling->t->resource[i] = resources[i];
    }
    printf("Task %s releases resource", scheduling->t->task_name);
    fflush(stdout);
    for(int i = 0; i < count; i++){
        printf(" %d", resources[i]);
        fflush(stdout);
    }
    printf("\n");
    fflush(stdout);
}
