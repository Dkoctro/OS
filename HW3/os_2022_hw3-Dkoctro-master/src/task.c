#include "../include/task.h"
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include "../include/builtin.h"
#include "../include/command.h"
#include "../include/function.h"
#include "../include/shell.h"
#include "../include/resource.h"

void task_sleep(int ms)
{
    scheduling->t->st = WAITING;
    scheduling->t->running++;
    scheduling->wait_time += ms;
    printf("Task %s goes to sleep.\n", scheduling->t->task_name);
    swapcontext(&scheduling->t->new_task, &cpu_idle);
}

void task_exit()
{
    rr++;
    scheduling->t->st = TERMINATED;
    scheduling->t->running++;
    printf("Task %s has terminated.\n", scheduling->t->task_name);
    swapcontext(&scheduling->t->new_task, &cpu_idle);
}

void task_judge(){
    struct sche *s = head;
    while(s != NULL){
        if(s->t->st == WAITING){
            s->t->waiting_time++;
            if(s->t->waiting == 0){
                if(s->t->waiting_time == s->wait_time){
                    s->t->st = READY;
                }
            } else {
                s->wait_time++;
                int available = 0;
                for(int i = s->counting; i < s->t->resource_num; i++){
                    if(res_ava[s->t->resource[i]]){
                        available = 1;
                        break;
                    }
                }
                if(!available){
                    s->t->st = READY;
                }
            }
        }

        if(s->next == NULL){
            break;
        }
        s = s->next;
    }
}

void task_choose(){
    task_judge();
    if(algorithm_choose == 2){ // RR
        struct sche *s = scheduling;
        int ready = 0;
        if(rr != tid){
            while(s != NULL){
                if(s != tail){
                    s = s->next;
                } else {
                    s = head;
                }

                if(s->t->st == READY){
                    break;
                }

                if(s == scheduling && s->t->st != READY){
                    ready = 1;
                    break;
                } else if(s == scheduling){
                    break;
                }
            }

            if(ready == 1){
                printf("CPU idle\n");
                setcontext(&cpu_idle);
            } else {
                if(s != scheduling){
                    printf("Task %s is running.\n", s->t->task_name);
                }
                scheduling = s;
                scheduling->t->st = RUNNING;
                setcontext(&scheduling->t->new_task);
            }
        } else {
            setcontext(&init);
        }
    } else { // Not RR
        int count = 0;
        struct sche *s = head;
        while(s != NULL){
            if(s->t->st == READY){
                break;
            } else {
                if(s->t->st == WAITING){
                    count++;
                }
            }
            if(s->next == NULL){
                break;
            }
            s = s->next;
        }

        if(s == tail && s->t->st != READY && count != 0){
            printf("CPU idle\n");
            setcontext(&cpu_idle);
        } else if(s == tail && s->t->st == TERMINATED && count == 0){
            setcontext(&init);
        } else {
            if(s != scheduling){
                printf("Task %s is running.\n", s->t->task_name);
            }
            scheduling = s;
            scheduling->t->st = RUNNING;
            setcontext(&scheduling->t->new_task);
        }
    }
}

void task_idle(){
    getcontext(&cpu_idle);
    cpu_idle.uc_stack.ss_sp = malloc(65536);
    cpu_idle.uc_stack.ss_size = 65536;
    cpu_idle.uc_link = NULL;
    makecontext(&cpu_idle, idle, 0);

    getcontext(&choose);
    choose.uc_stack.ss_sp = malloc(65536);
    choose.uc_stack.ss_size = 65536;
    choose.uc_link = NULL;
    makecontext(&choose, task_choose, 0);
}