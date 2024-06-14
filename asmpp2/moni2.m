/*
 ===========================================
 *	SC/MP-III Sample Program
 ===========================================
 */

#asm
; プリアンブル
; ------------
	  cpu sc/mp
	  org 0

sp   = 0
cnt1 = 1
cnt2 = 2
r4   = 3
p4   = 4
p5   = 6

L FUNCTION VAL16, (VAL16 & 0xFF)
H FUNCTION VAL16, ((VAL16 >> 8) & 0xFF)

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

RET MACRO
     XPPC  P3
	ENDM

LEA	MACRO P,VAL			; Load Pointer
	 XPAL	P
	 LDI	L(VAL)
	 XPAL	P
	 XPAH	P
	 LDI	H(VAL)
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

#endasm
// スタート
	 nop;
	 jmp(main);

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

main()
{
	push(p2);
	LDI('A');putc();
	a='A';putc();
	pop(p2);
	exit();
}

	pusha;
	popa;

#if 0
// メイン
main()
{
	p2=#msg1;puts();	

//	p2=0;mdump();
//	exit();

	while(1) {	
		a='>';putc();
		p2=#inbuf;gets();

		// ECHO BACK.
		p2=#inbuf;puts();

		//
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
	if(e=='d') {
		sp_skip();
		readhex();
		if(a!=0) {
			ea=p4;
			p5=ea;
		}
		ea=p5;p2=ea;mdump(); //メモリーダンプの実行.

		ea=p5;ea+=0x100;p5=ea;
	}	
	if(e=='q') {
		exit();
	}	
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

p4mul16()
{
	push(ea);
	ea=p4;
	sl(ea);
	sl(ea);
	sl(ea);
	sl(ea);
	p4=ea;
	pop(ea);
}

// P2 ポインタから16進HEX読み. ==> p4に結果. 入力された桁数=Areg
readhex()
{
	ea=0;
	p4=ea;
	r4=a;
	while(1) {
		a=*p2++;e=a;
		readhex1();e=a;
		if(e!=0xff) {
			p4mul16();
			a=e;
			e<>a; a=0; e<>a; //e=0;
			ea+=p4;
			p4=ea;
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
ascdump_16()
{
	push(p2);
	p2=ea;
	ascdump_8();
	pr_spc();
	ascdump_8();
	pop(p2);
}

//  アスキーダンプ8byte
ascdump_8()
{
	a=8;cnt2=a;
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


//  メモリーダンプ
mdump()
{
	a=16;cnt1=a;
	do {
		mdump_16();
	} while(--cnt1);
}

//  メモリーダンプ16byte
mdump_16()
{
	ea=p2;
	prhex4();
	pr_spc();

	mdump_8();
	pr_spc();
	mdump_8();

// ASCII DUMP
	ea=p2;ea-=16;
	ascdump_16();

	put_crlf();
}

//  メモリーダンプ8byte
mdump_8()
{
	a=8;cnt2=a;
	do {
		a=*p2++;
		prhex2();
		pr_spc();
	} while(--cnt2);
}

//  EAレジスタを16進4桁表示
prhex4()
{
	a<>e;
	prhex2();
	a<>e;
	prhex2();
}

//  Aレジスタを16進2桁表示
prhex2()
{
	push(ea);
	e=a;
	a>>=1;
	a>>=1;
	a>>=1;
	a>>=1;
	prhex1();

	a=e;
	prhex1();
	pop(ea);
}

//  Aレジスタ下位4bitのみを16進1桁表示
prhex1()
{
	push(ea);
	a&=0x0f;
	e=a;
	if( a >= 10) {
		a=e;a+=7;
	}else{
		a=e;
	}
	a += 0x30;
	putc();
	pop(ea);
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

//  文字列出力( P2 )ヌル終端.
puts()
{
	do {
		a=*p2++;
		if(a==0) break;
		putc();
	}while(1);	
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

//  文字列サンプル
msg1:
	db(" * SC/MP-III Monitor *");
	db(0x0d);
	db(0x0a);
	db(0);

inbuf:
	ds(128);

bufend:
	db(0);


//
#endif
