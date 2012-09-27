#include "winsnake.h"

WINDOW   *win_frame, *win_main, *win_stop;
char *error_str;

int bug;
int
main(void)
{
	wins_init();
	wins_loadwin(0);
	wins_run();

	return 0;
}

/*
 * 初始化数据
 */
void
wins_init(void)
{
	struct sigaction sig_win;
	sigset_t sig_mas;

	setlocale(LC_ALL, "");
	if (sigemptyset(&sig_mas) < 0)
		exit_print("sig_mas sigemptyset error\n");
	if (sigaddset(&sig_mas, SIGINT) < 0)
		exit_print("sig_mas sigaddset error\n");
	if (sigaddset(&sig_mas, SIGCONT) < 0)
		exit_print("sig_mas sigaddset error\n");
	if (sigaddset(&sig_mas, SIGWINCH) < 0)
		exit_print("sig_mas sigaddset error\n");
	if (sigprocmask(SIG_BLOCK, &sig_mas, NULL) < 0)
		exit_print("sig_mas sigprocmask error\n");

	if (sigemptyset(&sig_win.sa_mask) < 0)
		exit_print("sig_win sigemptyset error\n");
	sig_win.sa_handler = wins_loadwin;
	sig_win.sa_flags   = 0;
	if (sigaction(SIGWINCH, &sig_win, NULL) < 0)
		exit_print("SIGWINCH of sig_win sigaction error\n");
	if (sigaction(SIGCONT, &sig_win, NULL) < 0)
		exit_print("SIGCONT of sigaction sigaction error\n");
	
	sig_win.sa_handler = wins_exit;
	if (sigaction(SIGINT, &sig_win, NULL) < 0)
		exit_print("SIGINT of sigaction sigaction error\n");


	/*curses init*/
	if (initscr() == NULL)
		exit_print("initscr of curses error\n");
	curs_set(FALSE);
	start_color();
	cbreak();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_YELLOW);
	init_pair(3, COLOR_RED, COLOR_GREEN);
	init_pair(4, COLOR_YELLOW, COLOR_BLUE);
	init_pair(5, COLOR_YELLOW, COLOR_RED);

	win_frame = newwin(WIN_Y + 2, WIN_X + 2, 0, 0);
	win_main  = newwin(WIN_Y, WIN_X, 0, 0);
	win_stop  = newwin(1, 30, 0, 0);
	if (win_frame == NULL && win_main == NULL && win_stop == NULL) {
		error_str = "newwin or derwin error\n";
		wins_exit(1);
	}

	WIN_COLOR(stdscr, 1);
	WIN_COLOR(win_frame, 2);
	WIN_COLOR(win_main, 3);
	WIN_COLOR(win_stop, 1);

	if (sigprocmask(SIG_UNBLOCK, &sig_mas, NULL) < 0) {
		error_str = "sig_mas sigprocmask error\n";
		wins_exit(1);
	}
}

/*
 * 载入窗体
 */
void
wins_loadwin(int value)
{
	struct winsize wsize;
	unsigned short y, x;

	getmaxyx(stdscr, y, x);
	ioctl(STDOUT_FILENO, TIOCGWINSZ, (char *) &wsize);
	y = wsize.ws_row;
	x = wsize.ws_col;
	if (y < WIN_Y + 4 || x < WIN_X + 2) {
		error_str = "出错！因为屏幕太小。\n";
		wins_exit(1);
	}

	clear();
	mvaddstr(0, (x - 13) / 2, "YG-贪吃蛇 ver1.2");
	refresh();
	mvwin(win_frame, (y - WIN_Y - 2) / 2, (x - WIN_X - 2) / 2);
	mvwin(win_main, (y - WIN_Y) / 2, (x - WIN_X) / 2);
	mvwin(win_stop, (y - 1) / 2, (x - 30) / 2);
	wattron(win_frame, A_BOLD);
	box(win_frame, '|', '-');
	touchwin(win_frame);
	wrefresh(win_frame);
	touchwin(win_main);
	wrefresh(win_main);
}

/*
 * 运行函数
 * 用一个无限循环读取控制进程发来的数据
 * 然后根据指令，绘出蛇的图形
 */
void
wins_run(void)
{
	struct control snake_data;
	int oldxy[2];
	int i;

	while ((i = read(STDIN_FILENO, &snake_data, sizeof(snake_data)))) {
		if (i != sizeof(snake_data)) {
			if (i < 0)
				continue;
			else
				wins_exit(1);
		}
		switch (snake_data.con_command) {
			case CMD_START :
				wins_print(win_main, snake_data.con_shead, 'O'); 
				wins_print(win_main, snake_data.con_slast, '*');
				wins_print(win_main, snake_data.con_sfood, '$');
				oldxy[0] = snake_data.con_shead[0];
				oldxy[1] = snake_data.con_shead[1];
				continue;
			case CMD_NORMAL :
				wins_print(win_main, snake_data.con_shead, 'O'); 
				wins_print(win_main, snake_data.con_slast, ' ');
				break;
			case CMD_FOOD :
				wins_print(win_main, snake_data.con_shead, 'O'); 
				wins_print(win_main, snake_data.con_sfood, '$');
				break;
			case CMD_STOP :
				wins_print(win_stop, "游戏已暂停， 按“空格”键继续游戏", 0);
				continue;
			case CMD_OVER :
				wins_print(win_stop, "\t游戏结束", 0);
				continue;
		}
		wins_print(win_main, oldxy, '*');
		oldxy[0] = snake_data.con_shead[0];
		oldxy[1] = snake_data.con_shead[1];
	}

	wins_exit(1);
}

/*
 * 打印图形
 */
void
wins_print(WINDOW *win, const void *data, const char ch)
{
	if (win == win_main) {
		int *snake_xy;
		snake_xy = (int *) data;
		mvwaddch(win, snake_xy[0], snake_xy[1], ch);
	} else if (win == win_stop) {
		char *str;
		str = (char *) data;
		wclear(win);
		waddstr(win, str);
	}
	touchwin(win);
	wrefresh(win);
}

/*
 * 退出
 */
void
wins_exit(int value)
{

	delwin(win_main);
	delwin(win_frame);
	delwin(win_stop);
	endwin();
	if (error_str != NULL)
		fputs(error_str, stdout);
	exit(value);
}
