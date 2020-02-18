; SugarOS Main Part
; TAB=4

        ORG 0xc200
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
        DB       "Successfully loaded SugarOS!"
        DB       0x0a              ; 换行
        DB       0                 ; 保存一个0,让putloop读到它时停止

fin:
        HLT
        JMP fin