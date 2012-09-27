#include <stdlib.h>
#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include <locale.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "public.h"

/*
 * 窗口的高宽
 */
#define WIN_Y 20
#define WIN_X 60

#define WIN_COLOR(win, x) wbkgd(win, COLOR_PAIR(x))

void wins_init(void);
void wins_exit(int);
void wins_loadwin(int);
void wins_run(void);
void wins_print(WINDOW *, const void *, const char);
void exit_print(char *);
