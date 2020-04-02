
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
	int x, y, btn;
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
		/* 接收初始化数据 */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* 校验鼠标是否接触良好 */
		if ((dat & 0xc8) == 0x08) {
			/* 接收第1个字节 */
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

		//将第1个字节高5位于XY轴移动情况合并
		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}

		mdec->y = - mdec->y; /* �}�E�X�ł�y�����̕�������ʂƔ��� */
		return 1;
	}
	return -1; /* �����ɗ��邱�Ƃ͂Ȃ��͂� */
}



void HariMain(void)
{
	//初始化BootInfo
	struct BOOTINFO *binfo;
	binfo = (struct BOOTINFO *) ADR_BOOTINFO;  //从asmhead.nas里读数据

	//初始化GDT和IDT
	init_gdtidt();

	//初始化PIC中断
	init_pic();
	io_sti(); /* 允许中断 */

	//初始化调色板并绘制背景
	video_init_palette();
	video_refreshBackground(binfo, (void *)video_fillRect8);

	//初始化Console并绘制控制台背景
	struct ConsoleStatus cs;
	cs.callbackWhenFillRect = (void *)video_fillRect8;
	cs.callbackWhenPutChar = (void *)video_putChar8;
	cs.x0 = 8;
	cs.y0 = 24;
	cs.x1 = binfo->screenWidth  - 8;
	cs.y1 = binfo->screenHeight - 32;
	cs.backgroundColor = COL8_008484;
	extern char hankaku[4096];
	cs.fontLibrary = hankaku;
	struct ConsoleStatus *csptr = console_init(binfo, (struct ConsoleStatus *)&cs);

	//初始化鼠标图像,并绘制鼠标
	char mcursor[256];
	int mx = (binfo->screenWidth - 16) / 2;
	int my = (binfo->screenHeight - 28 - 16) / 2;
	video_init_mouse_cursor8(mcursor, COL8_008484);
	video_drawBitmap(binfo, mx, my, 16, 16, mcursor);

	//PIC设定
	io_out8(PIC0_IMR, 0xf9); /* PIC1�ƃL�[�{�[�h������(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �}�E�X������(11101111) */

	//临时变量
	char buffer[40];
	unsigned char i;

	//初始化键盘和鼠标FIFO缓冲区
	unsigned char keybuf[32];
	fifo8_init(&keyfifo, 32, keybuf);
	unsigned char mousebuf[128];
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
				console_println(binfo, buffer, csptr);
			}
			if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_pop(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					/* 当鼠标的3个字节消息凑够 */
					sprintf(buffer, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						buffer[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						buffer[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						buffer[2] = 'C';
					}
					video_fillRect8(binfo, 150, 8, 300, 23, COL8_008484);
					video_putShadowString8(binfo, 150, 8, buffer);

					//擦除鼠标
					video_fillRect8(binfo, mx, my, mx+15, my+15, COL8_008484);
					//加入移动量
					mx += mdec.x;
					my += mdec.y;
					//边界处理
					if(mx <= 0){
						mx = 0;
					}
					else if(mx >= binfo->screenWidth - 16){
						mx = binfo->screenWidth - 16;
					}
					if(my <= 0){
						my = 0;
					}
					else if(my >= binfo->screenHeight - 16){
						my = binfo->screenHeight - 16;
					}
					//显示鼠标坐标
					sprintf(buffer, "(%3d, %3d)", mx, my);
					video_fillRect8(binfo, 8, 8, 88, 24, COL8_008484);
					video_putShadowString8(binfo, 8, 8, buffer);
					//绘制鼠标
					video_drawBitmap(binfo, mx, my, 16, 16, mcursor);
				}
			}
		}
	}
}
