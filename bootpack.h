
/*
** SugarOS 全局变量和声明
*/

#include <stdio.h>


/*
** asmhead.nas
*/

struct BOOTINFO{
	char cyls, leds, vmode, reserve;
	short screenWidth, screenHeight;
	char *VideoRamAddress;
};

#define ADR_BOOTINFO	0x00000ff0


/*
** naskfunc.nas
*/

void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
unsigned int memtest_sub(unsigned int start, unsigned int end);


/*
** dsctbl.c
*/

#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e

struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);


/*
** init.c
*/

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

void init_pic(void);
void inthandler27(int *esp);


/*
** keyboard.c
*/

#define PORT_KEYSTA				0x0064 //键盘设备地址
#define PORT_KEYCMD				0x0064 //键盘写指令设备地址
#define PORT_KEYDAT				0x0060 //键盘写数据设备地址
#define KEYSTA_SEND_NOTREADY	0x02   //一个倒数第二位是1的二进制,用于检测KBC的响应
#define KEYCMD_WRITE_MODE		0x60   //键盘控制电路的模式设定指令
#define KBC_MODE				0x47   //使用鼠标模式指令

void wait_KBC_sendready(void);
void init_keyboard(void);
void inthandler21(int *esp);


/*
** mouse.c
*/

#define KEYCMD_SENDTO_MOUSE		0xd4 //告诉KBC下一个指令发给鼠标的指令
#define MOUSECMD_ENABLE			0xf4 //激活鼠标指令

struct MOUSE_DEC
/* 鼠标解码结构体 */
{
	unsigned char buf[3], phase;
	int x, y, btn;
};

void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
void inthandler2c(int *esp);

/*
** graphic.c
*/

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

struct ConsoleStatus {
	void (*callbackWhenFillRect)(struct BOOTINFO *binfo, int x0, int y0, int x1, int y1, unsigned char color);
    void (*callbackWhenPutChar)(struct BOOTINFO *binfo, int x, int y, char color, char *fontLibrary);
	int x0;
	int y0;
	int x1;
	int y1;
    int console_cursorX;
    int console_cursorY;
	int backgroundColor;
	char* fontLibrary;
};

//调色板部分
void video_set_palette(int start, int end, unsigned char *rgb);
void video_init_palette(void);

//绘图部分
void video_fillRect8(struct BOOTINFO *binfo, int x0, int y0, int x1, int y1, unsigned char color);
void video_drawBitmap(struct BOOTINFO *binfo, int bitmapX, int bitmapY, int bitmapWidth, int bitmapHeight, char *bitmap);
void video_refreshBackground(struct BOOTINFO *binfo, void (*callbackWhenFillRect)());
void video_refreshMenuBar(struct BOOTINFO *binfo, void (*callbackWhenFillRect)());

//文字部分
void video_putChar8(struct BOOTINFO *binfo, int x, int y, char color, char *fontLibrary);
void video_putString8(struct BOOTINFO *binfo, int x, int y, char color, unsigned char *stringPointer);
void video_putShadowString8(struct BOOTINFO *binfo, int x, int y, unsigned char *stringPointer);

//标准输入输出
struct ConsoleStatus* console_init(struct BOOTINFO *binfo, struct ConsoleStatus *cs);
void console_print(struct BOOTINFO *binfo, unsigned char *string, struct ConsoleStatus *cs);
void console_println(struct BOOTINFO *binfo, unsigned char *string, struct ConsoleStatus *cs);

//鼠标部分
void video_init_mouse_cursor8(char *mouseBitmap, char backgroundColor);


/*
** fifo.c
*/

struct FIFO8 {
	unsigned char *buf;
	int next_r, next_w, size, free, isOverflow;
};
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_push(struct FIFO8 *fifo, unsigned char data);
int fifo8_pop(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

/*
** memory.c
*/
#define MEMMAN_FREES		4090	    //设定内存管理器大小为32KB
#define MEMMAN_ADDR			0x003c0000  //设定内存管理器地址
struct FREEINFO
//内存块结构体
{
	unsigned int addr, size;
};
struct MEMMAN
//内存管理器结构体
{
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);
