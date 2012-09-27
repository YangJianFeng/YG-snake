#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include "public.h"

/*键常量定义*/
#define KEY_LEFT  68
#define KEY_RIGHT 67
#define KEY_DOWN  66
#define KEY_UP    65

/*最大高度与宽度*/
#define Y_MAX 20
#define X_MAX 60

/*
 *函数声明部份
 */
void snake_init(void);
void snake_key(void);
void snake_run(void);
void snake_exit(int);
void snake_over(void);
void snake_start(void);
void snake_child(int);
void *snake_alarm(void *);
void create_food(void);
void free_queue(void);
void exit_print(char *);
char getch(void);
int  input_queue(const int *);
int  out_queue(int *);

