
/*
** SugarOS 鼠标支持
*/

#include "bootpack.h"


void enable_mouse(struct MOUSE_DEC *mdec)
/* 激活鼠标 */
{
	wait_KBC_sendready();                      //等待键盘响应
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE); //告诉KBC下一个指令发给鼠标
	wait_KBC_sendready();                      //等待键盘响应
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);     //激活鼠标

	/* 初始化鼠标结构体 */
	mdec->phase = 0; //鼠标解码结构体进入第0阶段（等待初始化）
	return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
/* 鼠标解码部分 */
{
	if (mdec->phase == 0) {
		/* 接收初始化数据 */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* 校验鼠标是否接触良好 */
		if ((dat & 0xc8) == 0x08) {
			/* 如果良好,接收第1个字节 */
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		/* 接收第2个字节 */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* 接收第3个字节 */
		mdec->buf[2] = dat;
		mdec->phase = 1;

		//鼠标点击情况取第1个字节的低3位
		mdec->btn = mdec->buf[0] & 0x07;
		//X轴移动情况取第2个字节
		mdec->x = mdec->buf[1];
		//Y轴移动情况取第3个字节
		mdec->y = mdec->buf[2];

		//将第1个字节高5位与XY轴移动情况合并
		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}

		mdec->y = - mdec->y; //反转Y轴
		return 1;
	}
	return -1; //如果数据不能处理,就返回错误
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