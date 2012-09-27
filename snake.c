#include "snake.h"

struct control ske;                /*控制数据*/
struct termios save_tty;           /*终端*/
unsigned int   snake_speed;        /*蛇的速度*/
char           smap[Y_MAX][X_MAX]; /*图形映射*/
pthread_t      pth_id;             /*线程ID*/
sigset_t       newmask;            /*屏蔽的信号*/
sigset_t       oldmask;            /*保存的信号*/
pid_t          pid_id;             /*进程ID*/
int            fd_pipe[2];         /*传送坐标的管道*/
int            skey;               /*上下左右按键值*/
int            if_over;            /*判断游戏是否结果*/


pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

int
main(void)
{
	snake_init();

	if ((pid_id = fork()) == 0) {
		close(fd_pipe[1]);
		if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
			exit_print("sigprocmask error\n");
		if (dup2(fd_pipe[0], STDIN_FILENO) < 0)
			exit_print("dup error\n");
		if (execl("./snake-gui", "snake-gui", (char *) 0) < 0)
			exit_print("exec snake-gui error\n");
	}
	close(fd_pipe[0]);

	snake_start();

	return 0;
}


/*
 * 程序初始化
 * 初始化蛇头坐标、蛇尾坐标和食物坐标
 * 初始化蛇行动方向
 * 初始化图形映射
 * 初始化随机种子
 * 初始化管道
 * 初始化信号集与信号处理
 */
void
snake_init(void)
{
	struct sigaction sig_dispose;

	ske.con_shead[0] = 1;
	ske.con_shead[1] = 2;
	ske.con_slast[0] = 1;
	ske.con_slast[1] = 1;
	smap[1][1] = 1;
	smap[1][2] = 1;
	skey = KEY_RIGHT;
	ske.con_command = CMD_START;
	snake_speed     = 100000;
	
	input_queue(ske.con_slast);   /*把蛇尾坐标送入列队*/
	input_queue(ske.con_shead);   /*把蛇头坐标送入列队*/
	srand(time(NULL));
	create_food();
	
	if (pipe(fd_pipe) < 0)
		exit_print("pipe error\n");

	if (sigemptyset(&newmask) < 0)
		exit_print("sigemptyset error\n");
	if (sigaddset(&newmask, SIGINT) < 0)
		exit_print("sigaddset error\n");
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		exit_print("sigprocmask error\n");

	if (sigemptyset(&sig_dispose.sa_mask) < 0)
		exit_print("sig_dispose for isgemptyset error\n");
	if (sigaddset(&sig_dispose.sa_mask, SIGCHLD) < 0)
		exit_print("sig_dispose add SIGCHLD error\n");
	sig_dispose.sa_handler = SIG_IGN;
	sig_dispose.sa_flags   = 0;
	if (sigaction(SIGINT, &sig_dispose, NULL) < 0)
		exit_print("sigaction error\n");
	if (sigaction(SIGPIPE, &sig_dispose, NULL) < 0)
		exit_print("sigaction error\n");

	sig_dispose.sa_handler = snake_exit;
	if (sigemptyset(&sig_dispose.sa_mask) < 0)
		exit_print("sig_dispose for isgemptyset error\n");
	if (sigaddset(&sig_dispose.sa_mask, SIGINT) < 0)
		exit_print("sig_dispose add SIGINT error\n");
	sig_dispose.sa_flags  |= SA_NOCLDWAIT | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sig_dispose, NULL) < 0)
		exit_print("sigaction error\n");
	
	if (tcgetattr(STDIN_FILENO, &save_tty) < 0)
		exit_print("tcgetattr error\n");
}

/*
 * 游戏开始函数
 * 主要调用两个函数
 * snake_key()负责接收玩家操作
 * snake_alarm()用于倒计时（控制蛇的速度）
 */
void
snake_start(void)
{
	if (sigprocmask(SIG_UNBLOCK, &newmask, NULL) < 0)
		snake_exit(1);
	while (write(fd_pipe[1], &ske, sizeof(ske)) != sizeof(ske))
		;
	if (pthread_create(&pth_id, NULL, snake_alarm, NULL) != 0)
		exit_print("pthread_create error\n");

	snake_key();

	snake_exit(1);
}

/*
 * 接收与过滤玩家的按键、处理暂停
 */
void
snake_key(void)
{
	int t_key;
	int cmm;
	
	if (pthread_sigmask(SIG_BLOCK, &newmask, NULL) != 0)
		snake_exit(1);
	while ((t_key = getch()) != 0) {
		if (if_over)
			snake_exit(0);
		if (t_key == ' ' || t_key == 'p') {
			pthread_mutex_lock(&mut);
			cmm = ske.con_command;
			ske.con_command = CMD_STOP;
			while (write(fd_pipe[1], &ske, sizeof(ske)) != sizeof(ske))
				;
			ske.con_command = cmm;
			while ((t_key = getch()) != ' ' && t_key != 'p')
				;
			pthread_mutex_unlock(&mut);
			continue;
		}
		if (t_key == KEY_LEFT || t_key == KEY_RIGHT ||
		    t_key == KEY_DOWN || t_key == KEY_UP) {
			pthread_mutex_lock(&mut);
			skey = t_key;
			pthread_mutex_unlock(&mut);
		}
	}

	snake_exit(1);
}

/*
 * 食物生成
 */
void
create_food(void)
{
	int x, y;
	do {
		x = rand() % X_MAX;
		y = rand() % Y_MAX;
	} while (smap[y][x] == 1);
	ske.con_sfood[0] = y;
	ske.con_sfood[1] = x;
	smap[y][x] = 2;
}

/*
 * 蛇行动的处理
 * 蛇头坐标向前移动一位
 * 并判断是否有出界或碰上自己身体
 * 判断是否有吃到食物
 */
void
snake_run(void)
{
	int x, y;
	pthread_mutex_lock(&mut);
	switch (skey) {
		case KEY_LEFT :
			--ske.con_shead[1];
			break;
		case KEY_RIGHT :
			++ske.con_shead[1];
			break;
		case KEY_UP :
			--ske.con_shead[0];
			break;
		case KEY_DOWN :
			++ske.con_shead[0];
			break;
	}
	pthread_mutex_unlock(&mut);

	x = ske.con_shead[1];
	y = ske.con_shead[0];

	if ((y < 0 || y >= Y_MAX) || (x < 0 || x >= X_MAX)) {
		ske.con_command = CMD_OVER;
		goto Snake_write;
	}

	switch (smap[y][x]) {
		case 0 :
			if (out_queue(ske.con_slast) < 0)
				snake_exit(1);
			smap[ske.con_slast[0]][ske.con_slast[1]] = 0;
			ske.con_command = CMD_NORMAL;
			break;
		case 1 :
			ske.con_command = CMD_OVER;
			break;
		case 2 :
			ske.con_command = CMD_FOOD;
			create_food();
			break;
	}
	smap[y][x] = 1;

Snake_write:
	pthread_mutex_lock(&mut);
	while (write(fd_pipe[1], &ske, sizeof(ske)) != sizeof(ske))
		;
	pthread_mutex_unlock(&mut);
	if (ske.con_command == CMD_OVER) {
		snake_over();
	}

	if (input_queue(ske.con_shead) < 0)
		snake_exit(1);
}

/*
 * 游戏结束
 * 等待结束信号 
 */
void
snake_over(void)
{
	if_over = 1;
	for (;;);
}

/*
 * 退出序程的一些处理
 */
void
snake_exit(int value)
{
	kill(pid_id, SIGINT);
	free_queue();
	tcsetattr(STDIN_FILENO, TCSANOW, &save_tty);
	exit(value);
}

/*
 * 计时，蛇的速度
 */
void *
snake_alarm(void *value)
{
	struct timeval time_value;
	
	for (;;) {
		time_value.tv_sec  = 0;
		time_value.tv_usec = snake_speed;
		if (select(0, NULL, NULL, NULL, &time_value) < 0)
			continue;
		snake_run();
	}

	return NULL;
}
