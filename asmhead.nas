; SugarOS Main Part
; TAB=4

    BOTPAK     EQU     0x00280000        ; bootpack�̃��[�h��
    DSKCAC     EQU     0x00100000        ; �f�B�X�N�L���b�V���̏ꏊ
    DSKCAC0     EQU     0x00008000        ; �f�B�X�N�L���b�V���̏ꏊ�i���A�����[�h�j


    ; BOOT_INFO
    CYLS     EQU    0x0ff0            ; 定义变量CYLS(启动区)的地址
    LEDS     EQU    0x0ff1            ; 定义变量LEDS(键盘状态)的地址
    VMODE     EQU    0x0ff2            ; 定义变量VMODE(颜色位数)的地址
    SCRNX     EQU    0x0ff4            ; 定义变量SCRNX(显示器X轴分辨率)的地址
    SCRNY     EQU    0x0ff6            ; 定义变量SCRNY(显示器Y轴分辨率)的地址
    VRAM     EQU    0x0ff8            ; 定义变量VRAM(图像缓冲区开始位置)的地址

    ORG    0xc200                     ; 程序装载位置

    ; 调用显卡

    MOV    AL,0x13                    ; 设定视频模式(VGA显卡,320x200x8位彩色)
    MOV    AH,0x00                    ; 占位
    INT    0x10                       ; 调用BIOS

    ; 储存显示信息

    MOV    BYTE  [VMODE],8               ; 颜色位数为8位
    MOV    WORD  [SCRNX],320          ; 屏幕宽度320
    MOV    WORD  [SCRNY],200          ; 屏幕高度200
    MOV    DWORD [VRAM],0x000a0000    ; VRAM开始地址0x000a0000

    ; 获取键盘状态
    
    MOV    AH,0x02                    ; 设定查询指令
    INT    0x16                       ; 调用Keyboard BIOS
    MOV    [LEDS],AL                  ; 把BIOS返回值存到LEDS所在内存

;   PIC����؂̊��荞�݂��󂯕t���Ȃ��悤�ɂ���
;     AT�݊��@�̎d�l�ł́APIC�̏�����������Ȃ�A
;     ������CLI�O�ɂ���Ă����Ȃ��ƁA���܂Ƀn���O�A�b�v����
;     PIC�̏������͂��Ƃł��

    MOV    AL,0xff
    OUT    0x21,AL
    NOP            ; OUT���߂�A��������Ƃ��܂������Ȃ��@�킪����炵���̂�
    OUT    0xa1,AL

    CLI            ; �����CPU���x���ł����荞�݋֎~

; CPU����1MB�ȏ�̃������ɃA�N�Z�X�ł���悤�ɁAA20GATE��ݒ�

    CALL   waitkbdout
    MOV    AL,0xd1
    OUT    0x64,AL
    CALL   waitkbdout
    MOV    AL,0xdf         ; enable A20
    OUT    0x60,AL
    CALL   waitkbdout

; �v���e�N�g���[�h�ڍs

[INSTRSET "i486p"]        ; 486�̖��߂܂Ŏg�������Ƃ����L�q

    LGDT   [GDTR0]         ; �b��GDT��ݒ�
    MOV    EAX,CR0
    AND    EAX,0x7fffffff     ; bit31��0�ɂ���i�y�[�W���O�֎~�̂��߁j
    OR     EAX,0x00000001     ; bit0��1�ɂ���i�v���e�N�g���[�h�ڍs�̂��߁j
    MOV    CR0,EAX
    JMP    pipelineflush
pipelineflush:
    MOV    AX,1*8         ;  �ǂݏ����\�Z�O�����g32bit
    MOV    DS,AX
    MOV    ES,AX
    MOV    FS,AX
    MOV    GS,AX
    MOV    SS,AX

; bootpack�̓]��

    MOV    ESI,bootpack     ; �]����
    MOV    EDI,BOTPAK    ; �]����
    MOV    ECX,512*1024/4
    CALL   memcpy

; ���łɃf�B�X�N�f�[�^���{���̈ʒu�֓]��

; �܂��̓u�[�g�Z�N�^����

    MOV    ESI,0x7c00    ; �]����
    MOV    EDI,DSKCAC    ; �]����
    MOV    ECX,512/4
    CALL   memcpy

; �c��S��

    MOV    ESI,DSKCAC0+512     ; �]����
    MOV    EDI,DSKCAC+512     ; �]����
    MOV    ECX,0
    MOV    CL,BYTE [CYLS]
    IMUL   ECX,512*18*2/4     ; �V�����_������o�C�g��/4�ɕϊ�
    SUB    ECX,512/4    ; IPL�̕�������������
    CALL   memcpy

; asmhead�ł��Ȃ���΂����Ȃ����Ƃ͑S�����I������̂ŁA
;     ���Ƃ�bootpack�ɔC����

; bootpack�̋N��

    MOV    EBX,BOTPAK
    MOV    ECX,[EBX+16]
    ADD    ECX,3         ; ECX += 3;
    SHR    ECX,2         ; ECX /= 4;
    JZ     skip         ; �]������ׂ����̂��Ȃ�
    MOV    ESI,[EBX+20]     ; �]����
    ADD    ESI,EBX
    MOV    EDI,[EBX+12]     ; �]����
    CALL   memcpy
skip:
    MOV    ESP,[EBX+12]     ; �X�^�b�N�����l
    JMP    DWORD 2*8:0x0000001b

waitkbdout:
    IN     AL,0x64
    AND    AL,0x02
    JNZ    waitkbdout    ; AND�̌��ʂ�0�łȂ����waitkbdout��
    RET

memcpy:
    MOV    EAX,[ESI]
    ADD    ESI,4
    MOV    [EDI],EAX
    ADD    EDI,4
    SUB    ECX,1
    JNZ    memcpy         ; �����Z�������ʂ�0�łȂ����memcpy��
    RET

; memcpy�̓A�h���X�T�C�Y�v���t�B�N�X�����Y��Ȃ���΁A�X�g�����O���߂ł�������

    ALIGNB 16

GDT0:
    RESB   8        ; �k���Z���N�^
    DW     0xffff,0x0000,0x9200,0x00cf     ; �ǂݏ����\�Z�O�����g32bit
    DW     0xffff,0x0000,0x9a28,0x0047     ; ���s�\�Z�O�����g32bit�ibootpack�p�j

    DW     0
    
GDTR0:
    DW     8*3-1
    DD     GDT0

    ALIGNB 16

bootpack:
