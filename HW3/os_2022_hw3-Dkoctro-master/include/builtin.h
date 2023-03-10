#ifndef BUILTIN_H
#define BUILTIN_H

int help(char **args);
int cd(char **args);
int echo(char **args);
int exit_shell(char **args);
int record(char **args);
int mypid(char **args);

int add(char **args);
int del(char **args);
int ps(char **args);
int start(char **args);

extern const char *builtin_str[];

extern const int (*builtin_func[]) (char **);

extern const char *task_fun_str[];

extern const void (*task_func[])(void);

extern int num_builtins();

void signal_handle();
void pause_the_simulation();
void timer();

int tid;
int rr;
int rr_timer;

#endif
