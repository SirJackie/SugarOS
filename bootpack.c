
/*
** SugarOS 主程序
*/

#include "bootpack.h"

extern struct FIFO8 keyfifo, mousefifo;

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

struct MOUSE_DEC {
	unsigned char buf[3], phase;
};

void wait_KBC_sendready(void)
{
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	/* �L�[�{�[�h�R���g���[���̏����� */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(struct MOUSE_DEC *mdec)
{
	/* �}�E�X�L�� */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/* ���܂�������ACK(0xfa)�����M����Ă��� */
	mdec->phase = 0; /* �}�E�X��0xfa��҂��Ă���i�K */
	return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		/* �}�E�X��0xfa��҂��Ă���i�K */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* �}�E�X��1�o�C�g�ڂ�҂��Ă���i�K */
		mdec->buf[0] = dat;
		mdec->phase = 2;
		return 0;
	}
	if (mdec->phase == 2) {
		/* �}�E�X��2�o�C�g�ڂ�҂��Ă���i�K */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* �}�E�X��3�o�C�g�ڂ�҂��Ă���i�K */
		mdec->buf[2] = dat;
		mdec->phase = 1;
		return 1;
	}
	return -1; /* �����ɗ��邱�Ƃ͂Ȃ��͂� */
}



void HariMain(void)
{
	//初始化BootInfo
	struct BOOTINFO *binfo;
	binfo = (struct BOOTINFO *) ADR_BOOTINFO;  //从asmhead.nas里读数据

	//初始化标准输入输出
	struct ConsoleStatus cs;
	cs.callbackWhenFillRect = (void *)video_fillRect8;
	cs.callbackWhenPutChar = (void *)video_putChar8;
	cs.callbackWhenRefresh = (void *)video_refreshBackground;
	cs.console_cursorX = 8;
	cs.console_cursorY = 8;
	struct ConsoleStatus *csptr = (struct ConsoleStatus *)&cs;

	//初始化GDT和IDT
	init_gdtidt();

	//初始化PIC中断
	init_pic();
	io_sti(); /* 允许中断 */

	//初始化调色板并绘制背景
	video_init_palette();
	video_refreshBackground(binfo, (void *)video_fillRect8);

	//初始化鼠标图像,并绘制鼠标
	char mcursor[256];
	int mx = (binfo->screenWidth - 16) / 2;
	int my = (binfo->screenHeight - 28 - 16) / 2;
	video_init_mouse_cursor8(mcursor, COL8_008484);
	video_drawBitmap(binfo, mx, my, 16, 16, mcursor);

	//显示鼠标坐标
	char buffer[40];
	sprintf(buffer, "(%d, %d)", mx, my);
	// video_println(binfo, buffer, csptr);

	io_out8(PIC0_IMR, 0xf9); /* PIC1�ƃL�[�{�[�h������(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �}�E�X������(11101111) */

	//临时变量
	unsigned char i;

	//初始化键盘和鼠标FIFO缓冲区
	unsigned char keybuf[32];
	fifo8_init(&keyfifo, 32, keybuf);
	unsigned char mousebuf[32];
	fifo8_init(&mousefifo, 128, mousebuf);

	//初始化键盘和鼠标PIC
	init_keyboard();
	struct MOUSE_DEC mdec;
	enable_mouse(&mdec);

	
	for(;;){
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_pop(&keyfifo);
				io_sti();
				sprintf(buffer, "Keyboard: %02X", i);
				video_println(binfo, buffer, csptr);
			}
			if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_pop(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					/* 当鼠标的3个字节消息凑够 */
					sprintf(buffer, "Mouse: %02X %02X %02X", mdec.buf[0], mdec.buf[1], mdec.buf[2]);
					video_fillRect8(binfo, 150, 8, 300, 24, COL8_008484);
					video_putShadowString8(binfo, 150, 8, buffer);
				}
			}
		}
	}
}
