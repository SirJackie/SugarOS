
/*
** SugarOS 段号和中断记录表注册部分
*/

#include "bootpack.h"


void init_pic(void)
/* PIC初始化部分 */
{
	io_out8(PIC0_IMR,  0xff  ); //禁止所有中断
	io_out8(PIC1_IMR,  0xff  ); //禁止所有中断

	io_out8(PIC0_ICW1, 0x11  ); //边沿触发模式(Edge Trigger Mode)
	io_out8(PIC0_ICW2, 0x20  ); //IRQ0-7由INT20-27接收
	io_out8(PIC0_ICW3, 1 << 2); //PIC1由IRQ2连接
	io_out8(PIC0_ICW4, 0x01  ); //无缓冲区模式

	io_out8(PIC1_ICW1, 0x11  ); //边沿触发模式(Edge Trigger Mode)
	io_out8(PIC1_ICW2, 0x28  ); //IRQ8-15由INT28-2f接收
	io_out8(PIC1_ICW3, 2     ); //PIC1由IRQ2连接
	io_out8(PIC1_ICW4, 0x01  ); //无缓冲区模式

	io_out8(PIC0_IMR,  0xfb  ); //11111011 PIC1以外全部禁止
	io_out8(PIC1_IMR,  0xff  ); //11111111 禁止所有中断

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



/*
** 鼠标中断部分
*/

struct FIFO8 mousefifo;

void inthandler2c(int *esp)
/* 鼠标中断处理句柄 */
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);	  //通知从PIC IRQ-12已经受理完毕
	io_out8(PIC0_OCW2, 0x62);	  //通知主PIC IRQ-02已经受理完毕
	data = io_in8(PORT_KEYDAT);   //读数据
	fifo8_push(&mousefifo, data); //压入到鼠标缓冲区
	return;
}

void inthandler27(int *esp)
/* 部分机型初始化中断兼容程序 */
/* 部分机型在完成PIC初始化后,会产生IRQ-07中断,如果不予受理会导致启动失败 */
{
	io_out8(PIC0_OCW2, 0x67);     //通知从PIC IRQ-07已经受理完毕
	return;
}
