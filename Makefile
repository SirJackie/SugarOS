TOOLPATH = ./Tools/
MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)Win32DiskImager/Win32DiskImager.exe
COPY     = copy
DEL      = del

# 默认动作

default :
	$(MAKE) img

# 文件动作

ipl.bin : ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.lst

SugarOS.sys : SugarOS.nas Makefile
	$(NASK) SugarOS.nas SugarOS.sys SugarOS.lst

SugarOS.img : ipl.bin SugarOS.sys Makefile
	$(EDIMG)   imgin:./Tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 \
		copy from:SugarOS.sys to:@: \
		imgout:SugarOS.img

# 命令动作

img :
	$(MAKE) SugarOS.img

run :
	$(MAKE) img
	$(COPY) SugarOS.img .\Tools\qemu\fdimage0.bin
	$(MAKE) -C ./Tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) SugarOS.img

clear :
	-$(DEL) ipl.bin
	-$(DEL) ipl.lst
	-$(DEL) SugarOS.sys
	-$(DEL) SugarOS.lst

pure :
	$(MAKE) clear
	-$(DEL) SugarOS.img
