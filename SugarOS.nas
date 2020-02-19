; SugarOS Main Part
; TAB=4

    ; BOOT_INFO
    CYLS	EQU		0x0ff0            ; 定义变量CYLS(启动区)的地址
    LEDS	EQU		0x0ff1            ; 定义变量LEDS(键盘状态)的地址
    VMODE	EQU		0x0ff2            ; 定义变量VMODE(颜色位数)的地址
    SCRNX	EQU		0x0ff4            ; 定义变量SCRNX(显示器X轴分辨率)的地址
    SCRNY	EQU		0x0ff6            ; 定义变量SCRNY(显示器Y轴分辨率)的地址
    VRAM	EQU		0x0ff8            ; 定义变量VRAM(图像缓冲区开始位置)的地址

    ORG    0xc200                     ; 程序装载位置
    MOV    AL,0x13                    ; 设定视频模式(VGA显卡,320x200x8位彩色)
    MOV    AH,0x00                    ; 占位
    INT    0x10                       ; 调用BIOS

    ; 储存显示信息

    MOV    BYTE  [VMODE],8	          ; 颜色位数为8位
    MOV    WORD  [SCRNX],320          ; 屏幕宽度320
    MOV    WORD  [SCRNY],200          ; 屏幕高度200
    MOV    DWORD [VRAM],0x000a0000    ; VRAM开始地址0x000a0000

    ; 获取键盘状态
    
    MOV    AH,0x02                    ; 设定查询指令
    INT    0x16                       ; 调用Keyboard BIOS
    MOV    [LEDS],AL                  ; 把BIOS返回值存到LEDS所在内存

fin:
    HLT                               ; 休眠直到被唤醒(鼠标或键盘有动作)
    JMP    fin                        ; 如果被唤醒,继续休眠