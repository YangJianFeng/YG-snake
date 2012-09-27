/*
 * 最大X、Y座标定义
 */
#define X_MAX 60
#define Y_MAX 20

/*
 * 指令常量
 */
#define CMD_START  0x00   /*游戏开始*/
#define CMD_NORMAL 0x01   /*正常游戏状态*/
#define CMD_FOOD   0x02   /*吃到食物*/
#define CMD_STOP   0x03   /*暂停*/
#define CMD_OVER   0x04   /*游戏结束*/

/*
 * 控制桔构
 */
struct control {
	int con_command;    /*指令*/
	int con_shead[2];   /*蛇头坐标,[0]为Y纵坐标,[1]为X横坐标*/
	int con_slast[2];   /*蛇尾坐标,[0]为Y纵坐标,[1]为X横坐标*/
	int con_sfood[2];   /*物食坐标,[0]为Y纵坐标,[1]为X横坐标*/
};
