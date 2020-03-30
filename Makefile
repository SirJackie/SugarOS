TOOLS = ./Tools/
INCPATH  = ./Tools/SugarOS/

MAKE     = ./Tools/make.exe -r
NASK     = ./Tools/nask.exe
CC1      = ./Tools/cc1.exe -I $(INCPATH) -Os -Wall -quiet
GAS2NASK = ./Tools/gas2nask.exe -a
OBJ2BIM  = ./Tools/obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = ./Tools/bim2hrb.exe
RULEFILE = ./Tools/SugarOS/SugarOS.rul
EDIMG    = ./Tools/edimg.exe
IMGTOL   = ./Tools/Win32DiskImager/Win32DiskImager.exe
COPY     = copy
DEL      = del

# Ĭ�϶���

default :
	$(MAKE) img

# �ļ�����

ipl.bin : ipl.nas Makefile
	./Tools/nask.exe ipl.nas ipl.bin ipl.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

bootpack.gas : bootpack.c Makefile
	$(CC1) -o bootpack.gas bootpack.c

bootpack.nas : bootpack.gas Makefile
	$(GAS2NASK) bootpack.gas bootpack.nas

bootpack.obj : bootpack.nas Makefile
	$(NASK) bootpack.nas bootpack.obj bootpack.lst

naskfunc.obj : naskfunc.nas Makefile
	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : bootpack.obj naskfunc.obj hankaku.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj naskfunc.obj hankaku.obj
# 3MB+64KB=3136KB

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

SugarOS.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb SugarOS.sys

SugarOS.img : ipl.bin SugarOS.sys Makefile
	$(EDIMG)   imgin:./Tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 \
		copy from:SugarOS.sys to:@: \
		imgout:SugarOS.img

# �����

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
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) SugarOS.sys
	-$(DEL) .\Tools\qemu\fdimage0.bin
	
pure :
	$(MAKE) clear
	-$(DEL) SugarOS.img
