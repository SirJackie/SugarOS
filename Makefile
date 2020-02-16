#默认动作

default :
	make.exe img

# 描述文件的处理方法

ipl.bin : ipl.nas Makefile
	./Tools/nask.exe ipl.nas ipl.bin ipl.lst

SugarOS.img : ipl.bin Makefile
	./Tools/edimg.exe   imgin:./Tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0   imgout:SugarOS.img

# 定义指令

asm :
	make.exe -r ipl.bin

img :
	make.exe -r SugarOS.img

run :
	make.exe img
	copy SugarOS.img .\Tools\qemu\fdimage0.bin
	make.exe -C ./Tools/qemu

install :
	make.exe img
	./Tools/Win32DiskImager/Win32DiskImager.exe SugarOS.img

clear :
	-del ipl.bin
	-del ipl.lst

pure :
	make.exe clear
	-del SugarOS.img
