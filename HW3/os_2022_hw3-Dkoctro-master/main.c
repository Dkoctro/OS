#include <stdlib.h>
#include "include/shell.h"
#include "include/command.h"
#include <string.h>
#include "include/builtin.h"
#include <stdio.h>

int algorithm_choose = 0;

int main(int argc, char *argv[])
{
	history_count = 0;
	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	history[i] = (char *)malloc(BUF_SIZE * sizeof(char));

	if(strstr(argv[1], "FCFS") != NULL){
		algorithm_choose = 1; // For FCFS
	} else if(strstr(argv[1], "RR") != NULL){
		algorithm_choose = 2; // For RR
	} else if(strstr(argv[1], "PP") != NULL){
		algorithm_choose = 3; // For PP
	}

	tid = 0;
	rr = 0;
	rr_timer = 0;

	shell();

	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	free(history[i]);

	return 0;
}
