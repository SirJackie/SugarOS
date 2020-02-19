
char* VramAddr = 0xa0000;

void io_hlt();
void write_mem8(int addr, int data);

inline void writeVram(int x, int y, int color){
	*(VramAddr+y*320+x) = color;
}

void drawRect(int x1, int y1, int x2, int y2, int color){
	int x,y;
	for(y = y1; y < y2; y++){
		for(x = x1; x < x2; x++){
			writeVram(x, y, color);
		}
	}
}

void HariMain(void)
{
	// int i;
	// for (i = 0xa0000; i <= 0xaffff; i++) {
	// 	write_mem8(i, 15);
	// }
	drawRect(0, 0, 319, 199, 15);
fin:
	io_hlt();
	goto fin;
}
