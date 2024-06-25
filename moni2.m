/*
 ===========================================
 *	SC/MP-II Sample Program
 ===========================================

 CALL / RET の実装

 P3=SYSCALL

   XPPC P3
   DB HIGH,LOW


SYSCALL-1:
   XPPC P3
SYSCALL:
   P3をPUSH
   HIGH,LOWをP3に取得
   JMP SYSCALL-1 ==> XPPC P3により HIGH,LOWに分岐

   HIGH=0のときは
　 P3をPOP
   JMP SYSCALL-1 ==> XPPC P3により 呼び出し元に戻る

   AとEが壊れるので、保存、復帰する.

 ===========================================
 */

#asm
; プリアンブル
; ------------
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
cnt3 = 0x22
ea1  = 0x23
ea2  = 0x24

; オペコード.
op1  = 0x25
op2  = 0x26
op3  = 0x27
op1mask = 0x28
; オペサイズ.
opsize = 0x29
pcl    = 0x2a
pch    = 0x2b

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
// スタート
	 nop;
	 p1=#0xff80;
	 sp=0xfe;
	jsr(main);

//  文字列サンプル
help_msg:
	db(" * COMMAND *");
	db(0x0d);
	db(0x0a);
	db(">D ADRS ... DUMP");
	db(0x0d);
	db(0x0a);
	db(">L ADRS ... LIST");
	db(0x0d);
	db(0x0a);
	db(">Q      ... QUIT");
	db(0x0d);
	db(0x0a);
	
msg1:
	db(" * SC/MP-II Monitor *");
	db(0x0d);
	db(0x0a);
	db(0);

syscall_ret0:
	a=sys_e;a<>e;
	a=sys_a;
syscall_ret1:
	xppc(p3);


/* SYSCALL:
   HIGH,LOWをP3に取得
   P3をPUSH
   JMP SYSCALL-1 ==> XPPC P3により HIGH,LOWに分岐

   HIGH=0のときは
　 P3をPOP
   JMP SYSCALL-1 ==> XPPC P3により 呼び出し元に戻る

   AとEが壊れるので、保存、復帰する.
*/
syscall:
	sys_a=a;a<>e;
	sys_e=a;
	//HIGH,LOWを取得
	a=*p3++;
	a=*p3++;sys_hi=a;
	if(a==0) goto sys_ret;
	a=*p3  ;sys_lo=a;

    //P3をPUSH
#asm
	 DLD    SP(P1)     ; Ereg= --SP
	 XAE
	 XPAH	P3
	 ST     -128(P1)

	 DLD    SP(P1)     ; Ereg= --SP
	 XAE
	 XPAL	P3
	 ST     -128(P1)
#endasm
	a=sys_hi;xpah(p3);
	a=sys_lo;xpal(p3);
	goto syscall_ret0;

sys_ret:
	//P3をPOP
#asm
	 LD     SP(P1)     ; Ereg= SP
	 XAE
	 LD     -128(P1)
	 XPAL	P3
	 ILD     SP(P1)     ; Ereg = ++SP
	 XAE
	 LD     -128(P1)
	 XPAH	P3
	 ILD     SP(P1)     ; ++SP
#endasm
	goto syscall_ret0;
	


#include "mnem2.m"

//  Fence:::
//  ================================================
//  ここのアドレスが 0x100 以降に配置される必要がある
//  CALL と RET を上位アドレスが 00 かどうかで判別するため.
//

// メイン
main()
{
	p3=#syscall_ret1;
	p2=#msg1;puts();	

//	benchmark();

	while(1) {	
		a='>';putc();
		p2=#inbuf;gets();

		// ECHO BACK.
		p2=#inbuf;puts();

		// CMD EXEC.
		p2=#inbuf;cmd();
	}
	exit();
}

// P2 ポインタの１行バッファをcmd解釈.
// ワーク：
//    p4 = readhex()の戻り値.
//    p5 = 注目メモリーアドレスを覚えておく.
cmd()
{
	a=*p2++;lc();e=a;
	if(e=='q') {
		exit();
	}	
	if(e=='l') {
		disasm();
	}else
	if(e=='d') {
		sp_skip();
		readhex();
		if(a!=0) {
			movew(p5,p4);
		}
		ldptr(p2,p5);
		mdump(); //メモリーダンプの実行.
	}else if(e==0x0d) {
		
	}else{
		p2=#help_msg;puts();
	}

}

getc()
{
     db(0x21);
}
putc()
{
     db(0x20);
}
exit()
{
     db(0);
}

benchmark()
{
	cnt1=0;
	do {
		cnt2=0;
		do {
			bench_subr();
		}while(--cnt2);
	}while(--cnt1);
	p2=#msg1;puts();	
	exit();
}

bench_subr()
{
	p4=0;
	do {
		bench_subr2();
	}while(--p4);
}
bench_subr2()
{
}
// ==========================================
// 入力関数

// P2 ポインタの空白文字飛ばし.
sp_skip()
{
	a=*p2;
	while(a==' ') {
		a=*p2++;  // p2++ だけしたい.
		a=*p2;
	}
}

//  p4 <<=4;
p4mul16()
{
	cnt1=4;
	do {
		ccl; a=p4;add p4(p1);p4=a; ld p4+1(p1);add p4+1(p1); st p4+1(p1)
	} while(--cnt1);
}

// P2 ポインタから16進HEX読み. ==> p4に結果. 入力された桁数=Areg
readhex()
{
	// p4=0
	a=0;
	p4=a;
	st p4+1(p1);
	// r4=0
	r4=a;
	while(1) {
		a=*p2++;e=a;
		readhex1();e=a;
		if(e!=0xff) {
			p4mul16();
			a=e;
			a+=p4;p4=a;
			a=r4;a+=1;r4=a;
		}else{
			a=r4;
			return;
		}
	}
}

readhex1()
{
	lc();e=a;
	if(e>='0') {
		if(e<0x3a) { // <='9'
			a=e;
			a-=0x30;
			return;
		}
	}
	if(e>='a') {
		if(e<'g') {
			a=e;
			a-=0x57;  // 0x61 - 10
			return;
		}
	}
	a=0xff;
}


// ==========================================
// 出力関数

//  アスキーダンプ１行
//  アドレスは ea1
ascdump_16()
{
	push(p2);
	ldptr(p2,ea1);
	ascdump_8();
	pr_spc();
	ascdump_8();
	pop(p2);
}

//  アスキーダンプ8byte
ascdump_8()
{
	cnt2=8;
	do {
		a=*p2++;
		ascdump1();
	} while(--cnt2);
}

//  アスキーダンプ1byte
ascdump1()
{
	e=a;
	if(e<0x20) {
		a=' ';e=a;
	}
	if(e>=0x7f) {
		a=' ';e=a;
	}
	a=e;putc();
}

//  大文字にする.
uc()
{
	e=a;
	if(e>='a') {
		if(e<0x7b) {  // <='z'
			a=e;
			a-=0x20;
			return;
		}
	}
	a=e;
}

//  小文字にする.
lc()
{
	e=a;
	if(e>='A') {
		if(e<0x5b) {  // <='Z'
			a=e;
			a+=0x20;
			return;
		}
	}
	a=e;
}

inc_p2()
{
	xpal(p2);
	ccl ; adi(1);
	xpal(p2);

	xpah(p2);
	adi(0);
	xpah(p2);
}

//  メモリーダンプ
//  アドレスは p2
mdump()
{
	a=16;cnt1=a;
	do {
		mdump_16();
	} while(--cnt1);

	stptr(p2,p5);
}

//  メモリーダンプ16byte
//  アドレスは p2
mdump_16()
{
	stptr(p2,ea1)

	a=ea2;a<>e;
	a=ea1;
	prhex4();
	pr_spc();

	mdump_8();
	pr_spc();
	mdump_8();

	// ASCII DUMP
	stptr(p2,ea1);
	sub16(ea1);
	ascdump_16();

	put_crlf();
}

//  メモリーダンプ8byte
mdump_8()
{
	cnt2=8;
	do {
		a=*p2;
		prhex2();
		pr_spc();
		inc_p2();
	} while(--cnt2);
}

//  EAレジスタを16進4桁表示
prhex4()
{
	ea2=a;
	a<>e;
	ea1=a;

prhex4ea1:
	a=ea1;
	prhex2();
	a=ea2;
	prhex2();
}

//  Aレジスタを16進2桁表示
prhex2()
{
	push_ea;
	e=a;
	a>>=1;
	a>>=1;
	a>>=1;
	a>>=1;
	prhex1();

	a=e;
	prhex1();
	pop_ea;
}

//  Aレジスタ下位4bitのみを16進1桁表示
prhex1()
{
	push_ea;
	a&=0x0f;
	e=a;
	if( a >= 10) {
		a=e;a+=7;
	}else{
		a=e;
	}
	a += 0x30;
	putc();
	pop_ea;
}
//  空白文字を1つ出力
pr_spc()
{
	a=' ';putc();
}

//  改行コード出力
put_crlf()
{
	a=0x0d;putc();
	a=0x0a;putc();
}

//  文字列入力( P2 ) 0x0a + ヌル終端.
gets()
{
	do {
		getc();
		*p2++=a;
		e=a;
		if(e==0x0a) break;
		if(e==0x0d) break;
	}while(1);	

	a=0; *p2++=a;
}


//  文字列出力( P2 )ヌル終端.
puts()
{
	do {
		a=*p2++;
		if(a==0) break;
		putc();
	}while(1);	
}

#include "dis2.m"

//
