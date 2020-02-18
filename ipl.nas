; SugarOS
; TAB=4

        CYLS      EQU      10      ; 磁盘总煮柱面数位10
        ORG       0x7c00           ; 指明程序的装载地址

; FAT12引导区设置

        JMP       entry            ; 跳转到标号entry
        DB        0x90             ; 固定写法
        DB        "SUGAROS "       ; 启动区名称(8字节)
        DW        512              ; 扇区大小
        DB        1                ; 簇大小
        DW        1                ; FAT起始位置
        DB        2                ; FAT个数
        DW        224              ; 根目录大小
        DW        2880             ; 磁盘大小
        DB        0xf0             ; 磁盘种类
        DW        9                ; FAT长度
        DW        18               ; 每磁道扇区数
        DW        2                ; 磁头数
        DD        0                ; 是否使用分区
        DD        2880             ; 重写一次磁盘大小
        DB        0,0,0x29         ; 固定写法
        DD        0xffffffff       ; 卷标号码
        DB        "SUGAROS    "    ; 磁盘名称(11字节)
        DB        "FAT12   "       ; 磁盘格式名称(8字节)
        RESB      18               ; 空出18字节

; 程序主体

entry:
        MOV       AX,0             ; 初始化寄存器
        MOV       SS,AX            ; 初始化寄存器
        MOV       DS,AX            ; 初始化寄存器
        MOV       SP,0x7c00        ; 将栈顶指针(Stack Pointer,SP寄存器)移到ORG

        ; 读盘

        MOV       AX,0x0820        ; 临时变量
        MOV       ES,AX            ; 读取数据将要缓冲到0x8200(0x0820<<1)-0x83ff
        MOV       CH,0             ; 柱面0
        MOV       DH,0             ; 磁头0
        MOV       CL,2             ; 扇区2

readloop:
        MOV       SI,0             ; 错误次数:0次

retry:
        MOV       AH,0x02          ; 读盘(AH=0x02)
        MOV       AL,1             ; 读1个扇区
        MOV       BX,0             ; 空闲
        MOV       DL,0x00          ; 驱动器A

        INT       0x13             ; 调用磁盘BIOS
        JNC       next             ; 如果没出错就跳转到fin
        ADD       SI,1             ; 否则SI加1
        CMP       SI,5             ; 比较SI和5
        JAE       error            ; 如果SI >= 5,跳转到error
        
        MOV       AH,0x00          ; 重置驱动器(AH=0x00),为下一次read做准备
        MOV       DL,0x00          ; 驱动器A
        INT       0x13             ; 调用磁盘BIOS
        JMP       retry            ; 继续重读

next:
        MOV       AX,ES            ; 将ES移到到AX
        ADD       AX,0x0020        ; 将内存后移0x200(AX+=0x20)
        MOV       ES,AX            ; 将AX移回ES
        ADD       CL,1             ; 扇区 += 1
        CMP       CL,18            ; 比较扇区和18
        JBE       readloop         ; 当扇区 <= 18时，继续读取下一个扇区

        ; 否则说明柱面0磁头0已经读完

        MOV       CL,1             ; 扇区位置归1
        ADD       DH,1             ; 磁头 += 1
        CMP       DH,2             ; 比较磁头和2
        JB        readloop         ; 当磁头 < 2时,继续读下一个磁头

        ; 否则说明柱面0已经读完

        MOV       DH,0             ; 磁头归0
        ADD       CH,1             ; 柱面 += 1
        CMP       CH,CYLS          ; 比较柱面和CYLS总柱面数
        JB        readloop         ; 当柱面小于CYLS总柱面数时,继续读下一个柱面

fin:
        HLT                        ; CPU等待
        JMP       fin              ; 无限循环

error:
        MOV       SI,msg           ; 指定错误信息字符串地址

putloop:
        MOV       AL,[SI]          ; 读取第SI号内存(RAM[SI])的值到AL寄存器
        ADD       SI,1             ; SI寄存器加1(地址后移一位)
        CMP       AL,0             ; 比较一下AL和0两个数
        JE        fin              ; 如果AL是0(内存中的数据读完了),就跳转到fin
        MOV       AH,0x0e          ; 否则就显示一个文字
        MOV       BX,15            ; 指定字符颜色
        INT       0x10             ; 调用显卡BIOS
        JMP       putloop          ; 进行下一个putloop操作

msg:
        DB       0x0a              ; 换行
        DB       0x0a              ; 换行
        DB       "Error: Failed to load disk."
        DB       0x0a              ; 换行
        DB       0                 ; 保存一个0,让putloop读到它时停止

        RESB     0x7dfe-$          ; 用0占位直到0x7dfe这个地方

        DB       0x55, 0xaa        ; 告诉BIOS这个磁盘是可启动的