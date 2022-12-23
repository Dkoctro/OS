#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <ucontext.h>
#include "../include/builtin.h"
#include "../include/command.h"
#include "../include/task.h"
#include "../include/function.h"
#include "../include/shell.h"

int help(char **args)
{
	int i;
    printf("--------------------------------------------------\n");
  	printf("My Little Shell!!\n");
	printf("The following are built in:\n");
	for (i = 0; i < num_builtins(); i++) {
    	printf("%d: %s\n", i, builtin_str[i]);
  	}
	printf("%d: replay\n", i);
    printf("--------------------------------------------------\n");
	fflush(stdout);
	return 1;
}

int cd(char **args)
{
	if (args[1] == NULL) {
    	fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  	} else {
    	if (chdir(args[1]) != 0)
      		perror("lsh");
	}
	return 1;
}

int echo(char **args)
{
	bool newline = true;
	for (int i = 1; args[i]; ++i) {
		if (i == 1 && strcmp(args[i], "-n") == 0) {
			newline = false;
			continue;
		}
		printf("%s", args[i]);
		fflush(stdout);
		if (args[i + 1])
			printf(" ");
	}
	if (newline)
		printf("\n");

	return 1;
}

int exit_shell(char **args)
{
	return 0;
}

int record(char **args)
{
	if (history_count < MAX_RECORD_NUM) {
		for (int i = 0; i < history_count; ++i)
			printf("%2d: %s\n", i + 1, history[i]);
	} else {
		for (int i = history_count % MAX_RECORD_NUM; i < history_count % MAX_RECORD_NUM + MAX_RECORD_NUM; ++i)
			printf("%2d: %s\n", i - history_count % MAX_RECORD_NUM + 1, history[i % MAX_RECORD_NUM]);
	}
	return 1;
}

bool isnum(char *str)
{
	for (int i = 0; i < strlen(str); ++i) {
    	if(str[i] >= 48 && str[i] <= 57)
			continue;
        else
		    return false;
  	}
  	return true;
}

int mypid(char **args)
{
	char fname[BUF_SIZE];
	char buffer[BUF_SIZE];
	if(strcmp(args[1], "-i") == 0) {

	    pid_t pid = getpid();
	    printf("%d\n", pid);
		fflush(stdout);
	} else if (strcmp(args[1], "-p") == 0) {
	
		if (args[2] == NULL) {
      		printf("mypid -p: too few argument\n");
      		return 1;
    	}

    	sprintf(fname, "/proc/%s/stat", args[2]);
    	int fd = open(fname, O_RDONLY);
    	if(fd == -1) {
      		printf("mypid -p: process id not exist\n");
     		return 1;
    	}

    	read(fd, buffer, BUF_SIZE);
	    strtok(buffer, " ");
    	strtok(NULL, " ");
	    strtok(NULL, " ");
    	char *s_ppid = strtok(NULL, " ");
	    int ppid = strtol(s_ppid, NULL, 10);
    	printf("%d\n", ppid);
	    fflush(stdout);
		close(fd);

  	} else if (strcmp(args[1], "-c") == 0) {

		if (args[2] == NULL) {
      		printf("mypid -c: too few argument\n");
      		return 1;
    	}

    	DIR *dirp;
    	if ((dirp = opendir("/proc/")) == NULL){
      		printf("open directory error!\n");
      		return 1;
    	}

    	struct dirent *direntp;
    	while ((direntp = readdir(dirp)) != NULL) {
      		if (!isnum(direntp->d_name)) {
        		continue;
      		} else {
        		sprintf(fname, "/proc/%s/stat", direntp->d_name);
		        int fd = open(fname, O_RDONLY);
        		if (fd == -1) {
          			printf("mypid -p: process id not exist\n");
          			return 1;
        		}

        		read(fd, buffer, BUF_SIZE);
        		strtok(buffer, " ");
        		strtok(NULL, " ");
        		strtok(NULL, " ");
		        char *s_ppid = strtok(NULL, " ");
		        if(strcmp(s_ppid, args[2]) == 0)
		            printf("%s\n", direntp->d_name);

        		close(fd);
     		}
	   	}
    	
		closedir(dirp);
	
	} else {
    	printf("wrong type! Please type again!\n");
  	}
	
	return 1;
}

int add(char **args)
{
	struct task *the_task = (struct task*)calloc(1, sizeof(struct task));
	strcpy(the_task->task_name, args[1]);
	strcpy(the_task->func_name, args[2]);
	the_task->st = READY;
	the_task->prior = atoi(args[3]);
	the_task->waiting_time = -1;
	the_task->waiting = 0;
	the_task->resource  = (int*)malloc(sizeof(int) * 5);
	the_task->resource_num = 0;
	the_task->running = 0;
	the_task->next = NULL;
	the_task->tid = ++tid;
	for(int i = 0; i < 14; i++){
		if(strcmp(the_task->func_name, task_fun_str[i]) == 0){
			the_task->function = task_func[i];
		}
	}

	getcontext(&the_task->new_task);
	the_task->new_task.uc_stack.ss_sp = malloc(65536);
	the_task->new_task.uc_stack.ss_size = 65536;
	the_task->new_task.uc_link = NULL;
	makecontext(&the_task->new_task, the_task->function, 0);
	the_task->next = NULL;
	
	if(first == NULL){
		first = the_task;
		final = the_task;
	} else {
		final->next = the_task;
		final = final->next;
	}

	struct sche *schedule = (struct sche*)calloc(1, sizeof(struct sche));
	schedule->t = the_task;
	schedule->wait_time = 0;
	schedule->counting = 0;
	schedule->next = NULL;
	if(head == NULL){
		head = schedule;
		tail = schedule;
	} else {
		if(algorithm_choose == 3){ // PP
			struct sche *temp = head;
			if(tail->t->prior < schedule->t->prior){
				tail->next = schedule;
				tail = tail->next;
			} else if(head->t->prior > schedule->t->prior){
				schedule->next = head;
				head = schedule;
			} else {
				while(temp->next != NULL){
					if(temp->next->t->prior > schedule->t->prior){
						schedule->next = temp->next;
						temp->next = schedule;
						break;
					}
					temp = temp->next;
				}
			}
		} else {
			tail->next = schedule;
			tail = tail->next;
		}
	}
	printf("Task %s is ready.\n", args[1]);
	fflush(stdout);
	return 1;
}

int del(char **args)
{
	struct task *temp = first;
	while(temp != NULL){
		if(strcmp(temp->task_name, args[1]) == 0){
			temp->st = TERMINATED;
			break;
		}
		temp = temp->next;
	}
	rr++;
	struct sche *tmp = head;
	if(strcmp(tmp->t->task_name, args[1]) == 0){
		tmp->t->st = TERMINATED;
		head = head->next;
		free(tmp);
	} else {
		while(tmp->next != NULL){
			if(strcmp(tmp->next->t->task_name, args[1]) == 0){
				struct sche *s = tmp->next;
				tmp->next = s->next;
				if(s == tail){
					tail = tmp;
				}
				free(s);
				break;
			}
			tmp = tmp->next;
		}
	}
	printf("Task %s is killed.\n", args[1]);
	fflush(stdout);
	return 1;
}

int ps(char **args)
{
	char print[10];
	printf(" TID|       name|      state| running| waiting| turnaround| resources| priority\n");
	printf("-------------------------------------------------------------------------------\n");
	fflush(stdout);
	struct task *tmp = first;
	while(tmp != NULL){
		memset(print, '\0', 10);
		if(tmp->resource_num  == 0){
			strcat(print, "none");
		} else {
			for(int i = 0; i < tmp->resource_num; i++){
				char temp[3];
				sprintf(temp, " %d", tmp->resource[i]);
				strcat(print, temp);
			}
		}
		int wait_time = 0;
		if(tmp->waiting_time >= 0){
			wait_time = tmp->waiting_time;
		}

		char *turn = "none";
		if(tmp->st == TERMINATED){
			int total = tmp->waiting_time + tmp->running;
			if(tmp->waiting_time < 0){
				total++;
			}
			char time[10];
			memset(time, '\0', 10);
			sprintf(time, "%d", total);
			turn = time;
		}

		char *state_c;
		if(tmp->st == WAITING){
			state_c = "WAITING";
		} else if(tmp->st == READY){
			state_c = "READY";
		} else if(tmp->st == TERMINATED){
			state_c = "TERMINATED";
		} else if(tmp->st == RUNNING){
			state_c = "RUNNING";
		}

		printf("%4d|%11s|%11s|%8d|%8d|%11s|%10s|%9d\n", tmp->tid, tmp->task_name, state_c, tmp->running, wait_time, turn, print, tmp->prior);
		fflush(stdout);
		if(tmp->next == NULL){
			break;
		}
		tmp = tmp->next;
	}
	return 1;
}

int start(char **args)
{
	struct sche *s = head;
	while(s != NULL){
		if(s->t->st == READY){
			break;
		} else {
			if(s->next != NULL){
				s = s->next;
			} else {
				break;
			}
		}
	}
	if(s != tail || s->t->st != TERMINATED){
		printf("Task %s is running.\n", s->t->task_name);
		fflush(stdout);
		scheduling = s;
		scheduling->t->st = RUNNING;
		task_idle();
		timer();
		swapcontext(&init, &scheduling->t->new_task);
	}
	printf("Simulation over\n");
	fflush(stdout);
	return 1;
}

void signal_handle(){
	if(algorithm_choose == 2 && scheduling->t->st == RUNNING){ // RR
		rr_timer++;
		if(rr_timer == 3){
			rr_timer = 0;
		} else {
			scheduling->t->running++;
			task_judge();
			return;
		}
	}

	if(scheduling->t->st == RUNNING){
		scheduling->t->running++;
		scheduling->t->st = READY;
	}

	if(scheduling->t->st != READY){
		rr_timer = 0;
		setcontext(&choose);
	} else {
		swapcontext(&scheduling->t->new_task, &choose);
	}
}

void pause_the_simulation(){
	setcontext(&init);
}

void timer(){
	signal(SIGVTALRM, signal_handle);
	signal(SIGTSTP, pause_the_simulation);
	signal(SIGCONT, pause_the_simulation);
	struct itimerval it1, it2;
	it1.it_interval.tv_sec = 0;
	it1.it_interval.tv_usec = 10000; // 10ms
	it1.it_value.tv_sec =0 ;
	it1.it_value.tv_usec =100000;    // 10ms
	if(setitimer(ITIMER_VIRTUAL, &it1, &it2) != 0){
		perror("Timer is error");
	}

}

const char *builtin_str[] = {
 	"help",
 	"cd",
	"echo",
 	"exit",
 	"record",
	"mypid",
	"add",
	"del",
	"ps",
	"start"
};

const int (*builtin_func[]) (char **) = {
	&help,
	&cd,
	&echo,
	&exit_shell,
  	&record,
	&mypid,
	&add,
	&del,
	&ps,
	&start
};

int num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

const char *task_fun_str[] = {
	"test_exit",
	"test_sleep",
	"test_resource1",
	"test_resource2",
	"idle",
	"task1",
	"task2",
	"task3",
	"task4",
	"task5",
	"task6",
	"task7",
	"task8",
	"task9"
};

const void (*task_func[])(void) = {
	&test_exit,
	&test_sleep,
	&test_resource1,
	&test_resource2,
	&idle,
	&task1,
	&task2,
	&task3,
	&task4,
	&task5,
	&task6,
	&task7,
	&task8,
	&task9
};

