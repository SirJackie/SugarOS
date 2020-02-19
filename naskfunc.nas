; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目标文件的格式
[INSTRSET "i486p"]				; 生成486使用的汇编(32位)
[BITS 32]						; 制作32位模式用机器语言
[FILE "naskfunc.nas"]			; 源文件名

		GLOBAL	_io_hlt			; 声明函数io_hlt()
		GLOBAL  _write_mem8		; 声明函数write_mem8()

[SECTION .text]

_io_hlt:	    ; void io_hlt(void);
		HLT
		RET

_write_mem8:	; void write_mem8(int addr, int data);
		MOV		ECX,[ESP+4]		; 把[ESP+4](addr)存到ECX
		MOV		AL,[ESP+8]		; 把[ESP+8](data)存到AL
		MOV		[ECX],AL        ; 将第ECX号内存的值设为AL(addr=data)
		RET