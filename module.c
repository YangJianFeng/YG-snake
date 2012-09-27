#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>

struct queue {
	int qe_data[2];
	struct queue *qe_next;
};

static struct queue *p_head;
static struct queue *p_last;

/*
 * input queue
 */
int
input_queue(const int *value)
{
	struct queue *p_tmp;

	if ((p_tmp = malloc(sizeof(struct queue))) == NULL)
		return -1;

	memcpy(p_tmp->qe_data, value, 8);
	p_tmp->qe_next = NULL;
	if (p_head != NULL) {
		p_head->qe_next = p_tmp;
	}
	if (p_last == NULL) {
		p_last = p_tmp;
	}
	p_head = p_tmp;
	return 0;
}

/*
 * 检测是否为空列队
 * 为空返回真
 * 不为空返回假
 */
static int
if_empty(void)
{
	return (p_last == NULL ? 1 : 0);
}

/*
 * out queue
 */
int
out_queue(int *value)
{
	struct queue *p;
	if (if_empty())
		return -1;
	memcpy(value, p_last->qe_data, 8);
	p = p_last;
	p_last = p_last->qe_next;
	free(p);
	return 0;
}

/*
 * free queue
 */
void
free_queue(void)
{
	struct queue *p;
	p = p_last;
	while (p_last != NULL) {
		p_last = p_last->qe_next;
		free(p);
		p = p_last;
	}
}

/*
 * 从终端得到一个字符
 */
char
getch(void)
{
	char           c;


	read(STDIN_FILENO, &c, 1);
	if (c == 27) {
		read(STDIN_FILENO, &c, 1);
		read(STDIN_FILENO, &c, 1);
	}
	tcflush(STDIN_FILENO, TCIFLUSH);

	return c;
}

/*
 * 打印出错并退出
 */
void
exit_print(char *value)
{
	printf("%s", value);
	exit(1);
}
