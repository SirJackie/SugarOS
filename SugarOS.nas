; SugarOS
; TAB=4

		ORG		0x7c00          ; 指明程序的装载地址

; FAT12引导区设置

		JMP		entry           ; 跳转到标号entry
		DB		0x90            ; 固定写法
		DB		"HELLOIPL"		; 启动区名称(8字节)
		DW		512				; 扇区大小
		DB		1				; 簇大小
		DW		1				; FAT起始位置
		DB		2				; FAT个数
		DW		224				; 根目录大小
		DW		2880			; 磁盘大小
		DB		0xf0			; 磁盘种类
		DW		9				; FAT长度
		DW		18				; 每磁道扇区数
		DW		2				; 磁头数
		DD		0				; 是否使用分区
		DD		2880			; 重写一次磁盘大小
		DB		0,0,0x29        ; 固定写法
		DD		0xffffffff		; 卷标号码
		DB		"HELLO-OS   "	; 磁盘名称(11字节)
		DB		"FAT12   "		; 磁盘格式名称(8字节)
		RESB	18				; 空出18字节

; 程序主体

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX
		MOV		ES,AX

		MOV		SI,msg
putloop:
		MOV		AL,[SI]         ; 读取第SI号内存(RAM[SI])的值到AL寄存器
		ADD		SI,1			; SI寄存器加1(地址后移一位)
		CMP		AL,0            ; 比较一下AL和0两个数
		JE		fin             ; 如果AL是0(内存中的数据读完了),就跳转到fin
		MOV		AH,0x0e			; 否则就显示一个文字
		MOV		BX,15			; 指定字符颜色
		INT		0x10			; 调用显卡BIOS
		JMP		putloop         ; 进行下一个putloop操作
fin:
		HLT						; CPU等待
		JMP		fin				; 无限循环

msg:
		DB		0x0a, 0x0a		; 换行两次
		DB		"hello, world"  ; 保存字符串“hello,world”
		DB		0x0a			; 换行
		DB		0               ; 保存一个0,让putloop读到它时停止

		RESB	0x7dfe-$		; 用0占位直到0x7dfe这个地方

		DB		0x55, 0xaa      ; 告诉BIOS这个磁盘是可启动的

; 启动区外部分

		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	4600
		DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
		RESB	1469432