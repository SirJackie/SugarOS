
/*
** SugarOS FIFO队列支持
*/

#include "bootpack.h"


#define Overflowed 0x0001

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
/* 初始化FIFO队列 */
{
	fifo->size = size;    //长度
	fifo->buf  = buf;     //缓冲区指针
	fifo->free = size;    //剩余空间
	fifo->isOverflow = 0; //是否溢出
	fifo->next_w = 0;     //下一个写入位置
	fifo->next_r = 0;     //下一个读取位置
	return;
}

int fifo8_push(struct FIFO8 *fifo, unsigned char data)
/* 将元素压入FIFO队列 */
{
	if (fifo->free == 0) {
		/* 当没有剩余空间时 */
		fifo->isOverflow |= Overflowed; //标志为溢出
		return -1; //返回错误信号
	}
	/* 如果有剩余空间,就压入队列 */
	fifo->buf[fifo->next_w] = data; //将数据写入到下一个写入位置
	fifo->next_w++; //右移写入位置
	if (fifo->next_w == fifo->size) {
		/* 当写到缓冲区末尾时 */
		fifo->next_w = 0; //从头写起
	}
	fifo->free--; //剩余空间减少
	return 0; //返回正常信号
}

int fifo8_pop(struct FIFO8 *fifo)
/* 将元素弹出FIFO队列 */
{
	int data;
	if (fifo->free == fifo->size) {
		/* 当队伍中没有东西时 */
		return -1; //返回一个无效值
	}
	/* 如果还有剩余的东西,就弹出一个 */
	data = fifo->buf[fifo->next_r]; //获取下一个要读取的值
	fifo->next_r++; //读取指针右移
	if (fifo->next_r == fifo->size) {
		/* 当读到缓冲区末尾时 */
		fifo->next_r = 0; //从头读起
	}
	fifo->free++; //剩余空间增加
	return data; //返回数据
}

int fifo8_status(struct FIFO8 *fifo)
/* 获取FIFO队列长度 */
{
	return fifo->size - fifo->free; //返回总空间 - 剩余空间
}
