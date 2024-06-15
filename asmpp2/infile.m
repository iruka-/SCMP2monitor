/*
 ===========================================
 *	SC/MP-III Sample Program
 ===========================================
 */
#asm
   cpu sc/mp
   org 0

; byte
sp   = 0
esav = 1
asav = 2

sys_a = 3
sys_e = 4
sys_hi = 5
sys_lo = 6

; word
r1   = 0x08
r2   = 0x0a
r3   = 0x0c
r4   = 0x0e
; word
p4   = 0x10
p5   = 0x12
p6   = 0x14
p7   = 0x16
; byte
cnt1 = 0x20
cnt2 = 0x21
ea1  = 0x22
ea2  = 0x23

inbuf = 0xfe00

; 下位8bit、上位8bit を得る関数マクロ .
L   FUNCTION VAL16, (VAL16 & 0xFF)
H   FUNCTION VAL16, ((VAL16 >> 8) & 0xFF)

; Macros.
JS	MACRO P,VAL			; Jump to Subroutine
	 XPAH	P
	 LDI	H(VAL-1)
	 XPAH	P
	 XPAL	P
	 LDI	L(VAL-1)
	 XPAL	P
	 XPPC	P
	ENDM

JSR	MACRO VAL			; Jump to Subroutine
	 JS    P3,VAL
	ENDM


CALL MACRO VAL
     XPPC  P3
     DB    H(VAL-1)
     DB    L(VAL-1)
	ENDM

RET MACRO
     XPPC  P3
     DB    0
	ENDM

LEA	MACRO P,VAL			; Load Pointer
	 XPAL	P
	 LDI	L(VAL)
	 XPAL	P
	 XPAH	P
	 LDI	H(VAL)
	 XPAH	P
	ENDM

LDPTR MACRO P,VAL			; Load Pointer
	 XPAL	P
	 LD		VAL(P1)
	 XPAL	P
	 XPAH	P
	 LD		VAL+1(P1)
	 XPAH	P
	ENDM

STPTR MACRO P,VAL			; Load Pointer
	 XPAL	P
	 ST		VAL(P1)
	 XPAL	P
	 XPAH	P
	 ST		VAL+1(P1)
	 XPAH	P
	ENDM

PUSH MACRO P
	 XAE
	 DLD    SP(P1)     ; Ereg= --SP
	 XAE
	 XPAH	P
	 ST     -128(P1)
	 XPAH	P
	 XAE
	 DLD    SP(P1)     ; Ereg= --SP
	 XAE
	 XPAL	P
	 ST     -128(P1)
	 XPAL	P
	ENDM

POP MACRO P
	 XAE
	 LD     SP(P1)     ; Ereg= SP
	 XAE
	 XPAL	P
	 LD     -128(P1)
	 XPAL	P
	 XAE
	 ILD     SP(P1)     ; Ereg = ++SP
	 XAE
	 XPAH	P
	 LD     -128(P1)
	 XPAH	P
	 XAE
	 ILD     SP(P1)     ; ++SP
	 XAE
	ENDM

PUSHA MACRO
	 XAE
	 DLD    SP(P1)     ; Ereg= --SP
	 XAE
	 ST     -128(P1)
	ENDM

POPA MACRO
	 LD     SP(P1)     ; Ereg= SP
	 XAE
	 LD     -128(P1)
	 XAE
	 ILD    SP(P1)     ; ++SP
	 XAE
	ENDM

PUSH_EA MACRO
	 ST     ASAV(P1)
	 XAE
	 ST     ESAV(P1)
	 DLD    SP(P1)     ; Ereg= --SP
	 XAE
	 LD     ESAV(P1)
	 ST     -128(P1)   ; PUSH E
	 DLD    SP(P1)     ; Ereg= --SP
	 XAE
	 LD     ASAV(P1)
	 ST     -128(P1)   ; PUSH A
	 XAE
	 LD     ESAV(P1)
	 XAE
	ENDM

POP_EA MACRO
	 LD     SP(P1)     ; Ereg= SP
	 XAE
	 LD     -128(P1)
	 ST     ASAV(P1)
	 XAE
	 ILD    SP(P1)     ; Ereg= ++SP
	 XAE
	 LD     -128(P1)
	 ST     ESAV(P1)
	 ILD    SP(P1)     ; Ereg= ++SP
	 LD     ESAV(P1)
	 XAE
	 LD     ASAV(P1)
	ENDM

SUB16 MACRO WK
     SCL
     LD     WK(P1)
     CAI    16
     ST     WK(P1)
     LD     WK+1(P1)
     CAI    0
     ST     WK+1(P1)
	ENDM

ADD16 MACRO WK
     CCL
     LD     WK(P1)
     ADI    16
     ST     WK(P1)
     LD     WK+1(P1)
     ADI    0
     ST     WK+1(P1)
	ENDM

MOVEW MACRO DST,SRC
	 LD    SRC(P1)
	 ST    DST(P1)
	 LD    SRC+1(P1)
	 ST    DST+1(P1)
	ENDM



#endasm

	nop;

main()
{
	subr();
}

subr()
{
	p4=1;
	p5=p6;
}
#
