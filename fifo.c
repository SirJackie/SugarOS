/* FIFO���C�u���� */

#include "bootpack.h"

#define Overflowed		0x0001

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
/* FIFO�o�b�t�@�̏����� */
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /* �� */
	fifo->isOverflow = 0;
	fifo->next_w = 0; /* �������݈ʒu */
	fifo->next_r = 0; /* �ǂݍ��݈ʒu */
	return;
}

int fifo8_push(struct FIFO8 *fifo, unsigned char data)
/* FIFO�փf�[�^�𑗂荞��Œ~���� */
{
	if (fifo->free == 0) {
		/* �󂫂��Ȃ��Ă��ӂꂽ */
		fifo->isOverflow |= Overflowed;
		return -1;
	}
	fifo->buf[fifo->next_w] = data;
	fifo->next_w++;
	if (fifo->next_w == fifo->size) {
		fifo->next_w = 0;
	}
	fifo->free--;
	return 0;
}

int fifo8_pop(struct FIFO8 *fifo)
/* FIFO����f�[�^����Ƃ��Ă��� */
{
	int data;
	if (fifo->free == fifo->size) {
		/* �o�b�t�@������ۂ̂Ƃ��́A�Ƃ肠����-1���Ԃ���� */
		return -1;
	}
	data = fifo->buf[fifo->next_r];
	fifo->next_r++;
	if (fifo->next_r == fifo->size) {
		fifo->next_r = 0;
	}
	fifo->free++;
	return data;
}

int fifo8_status(struct FIFO8 *fifo)
/* �ǂ̂��炢�f�[�^�����܂��Ă��邩��񍐂��� */
{
	return fifo->size - fifo->free;
}
