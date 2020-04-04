
/*
** SugarOS 键盘支持
*/

#include "bootpack.h"


void wait_KBC_sendready(void)
/* 等待键盘控制电路(Keyboard Controller,KBC)响应 */
{
	for (;;) {
		/* 检测从键盘设备读取的数据的倒数第二位是不是0 */
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0)
		{
			/* 如果是0,就已经收到键盘的响应信号了 */
			break; //退出循环
		}
	}
	return;
}

void init_keyboard(void)
/* 初始化键盘控制电路 */
{
	wait_KBC_sendready();                    //等待键盘响应
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE); //往键盘写模式设定指令
	wait_KBC_sendready();                    //等待键盘响应
	io_out8(PORT_KEYDAT, KBC_MODE);          //往键盘写鼠标模式指令
	return;
}


/*
** 键盘中断部分
*/

struct FIFO8 keyfifo;
#define PORT_KEYDAT		0x0060

void inthandler21(int *esp)
/* 键盘中断处理句柄 */
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	  //通知PIC IRQ-01已经受理完毕
	data = io_in8(PORT_KEYDAT);   //读数据
	fifo8_push(&keyfifo, data);   //压入到键盘缓冲区
	return;
}