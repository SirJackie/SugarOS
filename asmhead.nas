
;;
;; SugarOS 设备信息获取和32位引导程序
;;


    BOTPAK     EQU    0x00280000      ; bootpack�̃��[�h��
    DSKCAC     EQU    0x00100000      ; �f�B�X�N�L���b�V���̏ꏊ
    DSKCAC0    EQU    0x00008000      ; �f�B�X�N�L���b�V���̏ꏊ�i���A�����[�h�j


    ; BOOT_INFO
    CYLS     EQU    0x0ff0            ; 定义变量CYLS(启动区)的地址
    LEDS     EQU    0x0ff1            ; 定义变量LEDS(键盘状态)的地址
    VMODE    EQU    0x0ff2            ; 定义变量VMODE(颜色位数)的地址
    SCRNX    EQU    0x0ff4            ; 定义变量SCRNX(显示器X轴分辨率)的地址
    SCRNY    EQU    0x0ff6            ; 定义变量SCRNY(显示器Y轴分辨率)的地址
    VRAM     EQU    0x0ff8            ; 定义变量VRAM(图像缓冲区开始位置)的地址

    ORG    0xc200                     ; 程序装载位置

    ; 调用显卡

    MOV    AL,0x13                    ; 设定视频模式(VGA显卡,320x200x8位彩色)
    MOV    AH,0x00                    ; 占位
    INT    0x10                       ; 调用BIOS

    ; 储存显示信息

    MOV    BYTE  [VMODE],8            ; 颜色位数为8位
    MOV    WORD  [SCRNX],320          ; 屏幕宽度320
    MOV    WORD  [SCRNY],200          ; 屏幕高度200
    MOV    DWORD [VRAM],0x000a0000    ; VRAM开始地址0x000a0000

    ; 获取键盘状态
    
    MOV    AH,0x02                    ; 设定查询指令
    INT    0x16                       ; 调用Keyboard BIOS
    MOV    [LEDS],AL                  ; 把BIOS返回值存到LEDS所在内存

; PIC关闭一切中断
; -根据AT兼容机的规格,如果要初始化PIC,
; -必须在CLI之前进行,否则又是会挂起,
; -随后进行PIC的初始化。

    MOV    AL,0xff                    ; 数据值位0xff
    OUT    0x21,AL                    ; 向0x21设备发送数据0xff (禁止主PIC的全部中断)
    NOP                               ; 连续执行OUT指令,有些机种会无法正常运行
    OUT    0xa1,AL                    ; 向0xa1设备发送数据0xff (禁止从PIC的全部中断)

    CLI                               ; 禁止CPU级别的中断,为之后模式设置做准备

; 为了让CPU能够访问1MB以上的内存空间,设定A20GATE

    CALL   waitkbdout                 ; 等待KBC电路响应,相当于wait_KBC_sendready()
    MOV    AL,0xd1                    ; 数据设为0xd1
    OUT    0x64,AL                    ; 将KBC电路设为OUTPORT模式
    CALL   waitkbdout                 ; 等待KBC电路响应,相当于wait_KBC_sendready()
    MOV    AL,0xdf                    ; 数据值为0xdf
    OUT    0x60,AL                    ; 启用A20GATE天线,激活大容量内存
    CALL   waitkbdout                 ; 等待KBC电路响应,相当于wait_KBC_sendready()

; 切换到保护模式

[INSTRSET "i486p"]                    ; 使用486指令

    LGDT   [GDTR0]                    ; 设定临时GDT
    MOV    EAX,CR0                    ; 读取CR0(Control Register 0)
    AND    EAX,0x7fffffff             ; 修改第31位为0
    OR     EAX,0x00000001             ; 修改第1位为1
    MOV    CR0,EAX                    ; 写回CR0(Control Register 0),完成保护模式的转换
    JMP    pipelineflush              ; 进入保护模式后,立刻进行JUMP来清除CPU的pipeline缓存,防止CPU缓存了这行下面的指令,从而执行错误

; 刷新段寄存器

pipelineflush:
    MOV    AX,1*8                     ; 可读写的段 32bit (0x0008相当于gdt+1段)
    MOV    DS,AX
    MOV    ES,AX
    MOV    FS,AX
    MOV    GS,AX
    MOV    SS,AX

; 从bootpack的地址开始的512KB内容复制到0x00280000号地址去

    MOV    ESI,bootpack               ; 传送源
    MOV    EDI,BOTPAK                 ; 传送目的地
    MOV    ECX,512*1024/4
    CALL   memcpy

; 硬盘数据传送到内存中,并保证复制后程序与它在磁盘上本来的位置相吻合

; 将启动扇区复制到1M以后的内存去

    MOV    ESI,0x7c00                 ; 传送源
    MOV    EDI,DSKCAC                 ; 传送目的地
    MOV    ECX,512/4
    CALL   memcpy

; 将0x00008200之后的磁盘内容复制到内存地址0x00100000后面去

    MOV    ESI,DSKCAC0+512            ; 传送源
    MOV    EDI,DSKCAC+512             ; 传送目的地
    MOV    ECX,0
    MOV    CL,BYTE [CYLS]
    IMUL   ECX,512*18*2/4             ; 从柱面数变换为字节数要除以4
    SUB    ECX,512/4                  ; 减去IPL启动区部分
    CALL   memcpy

; asmhead的工作完成,接下来交给bootpack处理

; bootpack的启动部分(将bootpack.hrb的第0x10c8字节开始的0x11a8个字节复制到0x310000号地址去)

    MOV    EBX,BOTPAK
    MOV    ECX,[EBX+16]
    ADD    ECX,3                      ; ECX += 3;
    SHR    ECX,2                      ; ECX /= 4;
    JZ     skip                       ; 当没有要传送的东西时,跳转到skip标签去
    MOV    ESI,[EBX+20]               ; 传送源
    ADD    ESI,EBX
    MOV    EDI,[EBX+12]               ; 传送目的地
    CALL   memcpy

skip:
    MOV    ESP,[EBX+12]               ; 栈初始值
    JMP    DWORD 2*8:0x0000001b       ; 将2*8带入到CS中,同时移动到0x1b号地址,开始执行bootpack

waitkbdout:
    IN     AL,0x64
    AND    AL,0x02
    IN     Al,0x60                    ; 空读，清空键盘缓冲区垃圾数据
    JNZ    waitkbdout                 ; AND的结果如果不是0,继续跳转到waitkbdout
    RET

memcpy:
    MOV    EAX,[ESI]
    ADD    ESI,4
    MOV    [EDI],EAX
    ADD    EDI,4
    SUB    ECX,1
    JNZ    memcpy                     ; 减法运算的结果如果不是0,继续memcpy
    RET

; 一直添加DBO,直到时机合适 (ALIGNB 16的地址能被16整除的情况下),这样能够使汇编指令对齐在16字节上,加快MOV的速度

    ALIGNB 16

GDT0:
    RESB   8                          ; 不在GDT0定义段(空出8字节)
    DW     0xffff,0x0000,0x9200,0x00cf     ; 定义GDT1
    DW     0xffff,0x0000,0x9a28,0x0047     ; 定义GDT2

    DW     0
    
GDTR0:
    DW     8*3-1                      ; 写入16位段上限
    DD     GDT0                       ; 写入32位段上限

    ALIGNB 16

bootpack:
