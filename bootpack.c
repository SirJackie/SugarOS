
/*
** SugarOS 主程序
*/

#include "bootpack.h"


/*
** 外部变量引用
*/

extern char hankaku[4096]; //引用字库
extern struct FIFO8 keyfifo, mousefifo; //从int.c中引用键盘和鼠标缓冲区


/*
** 键盘和鼠标控制电路设定
*/

#define PORT_KEYSTA				0x0064 //键盘设备地址
#define PORT_KEYCMD				0x0064 //键盘写指令设备地址
#define PORT_KEYDAT				0x0060 //键盘写数据设备地址
#define KEYSTA_SEND_NOTREADY	0x02   //一个倒数第二位是1的二进制,用于检测KBC的响应
#define KEYCMD_WRITE_MODE		0x60   //键盘控制电路的模式设定指令
#define KBC_MODE				0x47   //使用鼠标模式指令

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

struct MOUSE_DEC
/* 鼠标解码结构体 */
{
	unsigned char buf[3], phase;
	int x, y, btn;
};

#define KEYCMD_SENDTO_MOUSE		0xd4 //告诉KBC下一个指令发给鼠标的指令
#define MOUSECMD_ENABLE			0xf4 //激活鼠标指令

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
	cs.console_cursorX = 8;
	cs.console_cursorY = 24;
	cs.backgroundColor = COL8_008484;
	cs.fontLibrary = hankaku;
	// struct ConsoleStatus *csptr = console_init(binfo, (struct ConsoleStatus *)&cs);
	struct ConsoleStatus *csptr = (struct ConsoleStatus *)&cs;

	//初始化鼠标图像,并绘制鼠标
	char mcursor[256];
	int mx = (binfo->screenWidth - 16) / 2;
	int my = (binfo->screenHeight - 28 - 16) / 2;
	video_init_mouse_cursor8(mcursor, COL8_008484);
	video_drawBitmap(binfo, mx, my, 16, 16, mcursor);

	//PIC设定
	io_out8(PIC0_IMR, 0xf9); /* PIC1�ƃL�[�{�[�h������(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �}�E�X������(11101111) */

	//创建字符缓冲区
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

	/* 开始响应 */
	for(;;){
		io_cli(); //禁止中断
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0)
		{
			/* 如果鼠标和键盘没有动作 */
			io_stihlt(); //恢复中断并紧接着休眠(STI和HLT的汇编必须编译在一起!)
		} else {
			/* 如果有动作 */
			if (fifo8_status(&keyfifo) != 0) {
				/* 如果是键盘的动作 */
				i = fifo8_pop(&keyfifo);    //取出动作
				io_sti();                   //恢复中断
				/* 打印键盘动作 */
				sprintf(buffer, "Keyboard: %02X", i); 
				console_println(binfo, buffer, csptr);
			}
			if (fifo8_status(&mousefifo) != 0) {
				/* 如果是鼠标的动作 */
				i = fifo8_pop(&mousefifo);  //取出动作
				io_sti();                   //恢复中断
				if (mouse_decode(&mdec, i) != 0) {
					/* 当鼠标的3个字节消息凑够 */
					
					/* 打印键盘动作 */
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

					//擦除鼠标并重绘菜单栏
					video_fillRect8(binfo, mx, my, mx+15, my+15, COL8_008484);
					if(my >= binfo->screenHeight - (28+16)){
						/* 上次鼠标在菜单栏运动 */
						video_refreshMenuBar(binfo, video_fillRect8); //还要重绘菜单栏
					}

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
