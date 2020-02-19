
char* VramAddr = 0xa0000;

void io_hlt();
void write_mem8(int addr, int data);

inline void writeVram(int x, int y, int color){
	*(VramAddr+y*320+x) = color;
}

void drawRect(int x1, int y1, int width, int height, int color){
	int x,y;
	for(y = y1; y < y1+width; y++){
		for(x = x1; x < x1+width; x++){
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
	int i, k;
	for(i = 0; i < 32; i++){
		drawRect(i*6 , i*6, 50, 50, i+32);
	}
	
fin:
	io_hlt();
	goto fin;
}
