
;;
;; SugarOS 汇编函数部分
;;


[FORMAT "WCOFF"]				; 制作目标文件的格式	
[INSTRSET "i486p"]				; 使用到486为止的指令
[BITS 32]						; 制作32位模式机器语言
[FILE "naskfunc.nas"]			; 源文件名信息

		; 需要链接的函数
		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
		EXTERN	_inthandler21, _inthandler27, _inthandler2c

[SECTION .text]                 ; 目标文件中写了这些之后再写程序

_io_hlt:
; 休眠
; void io_hlt(void);
		HLT
		RET

_io_cli:
; 禁止中断
; void io_cli(void);
		CLI
		RET

_io_sti:
; 允许中断
; void io_sti(void);
		STI
		RET

_io_stihlt:
; 允许中断并休眠
; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:
; 输入到设备(8位)
; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

_io_in16:
; 输入到设备(16位)
; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:
; 输入到设备(32位)
; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:
; 输出到设备(8位)
; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:
; 输出到设备(16位)
; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:
; 输出到设备(32位)
; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:
; 获取EFLAGS寄存器的值
; int io_load_eflags(void);
		PUSHFD		; 相当于PUSH EFLAGS
		POP		EAX
		RET

_io_store_eflags:
; 写入值到EFLAGS寄存器
; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; 相当于POP EFLAGS
		RET

_load_gdtr:
; 载入GDTR寄存器
; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; 读入limit
		MOV		[ESP+6],AX      ; 右移limit两位
		LGDT	[ESP+6]         ; 载入GDTR寄存器(48位)
		RET

_load_idtr:
; 载入IDTR寄存器
; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; 读入limit
		MOV		[ESP+6],AX      ; 右移limit两位
		LIDT	[ESP+6]         ; 载入IDTR寄存器(48位)
		RET

_asm_inthandler21:
; 键盘中断回调程序
; 调用void inthandler21(int *esp);
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
; 鼠标中断回调程序
; 调用void inthandler27(int *esp);
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
; 开机中断兼容回调程序
; 调用void inthandler2c(int *esp);
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
