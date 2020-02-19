; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目标文件的格式
[BITS 32]						; 制作32位模式用机器语言


; 制作目标文件的信息

[FILE "naskfunc.nas"]			; 源文件名称

		GLOBAL	_io_hlt			; 程序中包含的函数名


; 实际函数

[SECTION .text]		; 目标文件前缀

_io_hlt:	; void io_hlt(void);
		HLT
		RET