;/*
; ===========================================
; *	SC/MP-II Sample Program
; ===========================================
;
; CALL / RET の実装
;
; P3=SYSCALL
;
;   XPPC P3
;   DB HIGH,LOW
;
;
;SYSCALL-1:
;   XPPC P3
;SYSCALL:
;   P3をPUSH
;   HIGH,LOWをP3に取得
;   JMP SYSCALL-1 ==> XPPC P3により HIGH,LOWに分岐
;
;   HIGH=0のときは
;　 P3をPOP
;   JMP SYSCALL-1 ==> XPPC P3により 呼び出し元に戻る
;
;   AとEが壊れるので、保存、復帰する.
;
; ===========================================
; */
;
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


;// スタート
;	 nop;
	nop
;	 p1=#0xff80;
	lea	p1,0xff80
;	 a=0xfe;
	ldi	0xfe
;	 sp=a;
	st	sp(P1)
;	jsr(main);
	jsr	main
;
;//  文字列サンプル
;help_msg:
help_msg:
;	db(" * COMMAND *");
	db	" * COMMAND *"
;	db(0x0d);
	db	0x0d
;	db(0x0a);
	db	0x0a
;	db(">D ADRS ... DUMP");
	db	">D ADRS ... DUMP"
;	db(0x0d);
	db	0x0d
;	db(0x0a);
	db	0x0a
;	db(">L ADRS ... LIST");
	db	">L ADRS ... LIST"
;	db(0x0d);
	db	0x0d
;	db(0x0a);
	db	0x0a
;	db(">Q      ... QUIT");
	db	">Q      ... QUIT"
;	db(0x0d);
	db	0x0d
;	db(0x0a);
	db	0x0a
;	
;msg1:
msg1:
;	db(" * SC/MP-III Monitor *");
	db	" * SC/MP-III Monitor *"
;	db(0x0d);
	db	0x0d
;	db(0x0a);
	db	0x0a
;	db(0);
	db	0
;
;
;syscall_ret0:
syscall_ret0:
;	a=sys_e;a<>e;
	ld	sys_e(P1)
	xae
;	a=sys_a;
	ld	sys_a(P1)
;syscall_ret1:
syscall_ret1:
;	xppc(p3);
	xppc	p3
;
;
;/* SYSCALL:
;   HIGH,LOWをP3に取得
;   P3をPUSH
;   JMP SYSCALL-1 ==> XPPC P3により HIGH,LOWに分岐
;
;   HIGH=0のときは
;　 P3をPOP
;   JMP SYSCALL-1 ==> XPPC P3により 呼び出し元に戻る
;
;   AとEが壊れるので、保存、復帰する.
;*/
;syscall:
syscall:
;	sys_a=a;a<>e;
	st	sys_a(P1)
	xae
;	sys_e=a;
	st	sys_e(P1)
;	//HIGH,LOWを取得
;	a=*p3++;
	ld	@1(p3)
;	a=*p3++;sys_hi=a;
	ld	@1(p3)
	st	sys_hi(P1)
;	a=*p3  ;sys_lo=a;
	ld	0(p3)
	st	sys_lo(P1)
;
;	a=sys_hi;
	ld	sys_hi(P1)
;	if(a==0) goto sys_ret;
	jz	sys_ret
;
;    //P3をPUSH
	 DLD    SP(P1)     ; Ereg= --SP
	 XAE
	 XPAH	P3
	 ST     -128(P1)

	 DLD    SP(P1)     ; Ereg= --SP
	 XAE
	 XPAL	P3
	 ST     -128(P1)
;	a=sys_hi;xpah(p3);
	ld	sys_hi(P1)
	xpah	p3
;	a=sys_lo;xpal(p3);
	ld	sys_lo(P1)
	xpal	p3
;	goto syscall_ret0;
	jmp	syscall_ret0
;
;sys_ret:
sys_ret:
;	//P3をPOP
	 LD     SP(P1)     ; Ereg= SP
	 XAE
	 LD     -128(P1)
	 XPAL	P3
	 ILD     SP(P1)     ; Ereg = ++SP
	 XAE
	 LD     -128(P1)
	 XPAH	P3
	 ILD     SP(P1)     ; ++SP
;	goto syscall_ret0;
	jmp	syscall_ret0
;	
;
;// メイン
;main()
;{
main:
;	p3=#syscall_ret1;
	lea	p3,syscall_ret1
;	p2=#msg1;puts();	
	lea	p2,msg1
	call	puts
;	while(1) {	
__wh000:
;		a='>';putc();
	ldi	'>'
	call	putc
;		p2=#inbuf;gets();
	lea	p2,inbuf
	call	gets
;
;		// ECHO BACK.
;		p2=#inbuf;puts();
	lea	p2,inbuf
	call	puts
;
;		//		
;		p2=#inbuf; //cmd();
	lea	p2,inbuf
;// P2 ポインタの１行バッファをcmd解釈.
;// ワーク：
;//    p4 = readhex()の戻り値.
;//    p5 = 注目メモリーアドレスを覚えておく.
;//cmd()
;//{
;	a=*p2++;lc();e=a;
	ld	@1(p2)
	call	lc
	xae
	lde
;	if(e=='q') {
	lde
	scl
	cai	'q'
	jnz	__el001
;		exit();
	call	exit
;	}	
;	if(e=='l') {
__el001:
	lde
	scl
	cai	'l'
	jnz	__el002
;		disasm();
	call	disasm
;	}else
	jmp	__fi002
__el002:
;	if(e=='d') {
	lde
	scl
	cai	'd'
	jnz	__el003
;		sp_skip();
	call	sp_skip
;		readhex();
	call	readhex
;		if(a!=0) {
	jz	__el004
;			movew(p5,p4);
	movew	p5,p4
;		}
;		ldptr(p2,p5);
__el004:
	ldptr	p2,p5
;		mdump(); //メモリーダンプの実行.
	call	mdump
;	}else if(e==0x0d) {
	jmp	__fi003
__el003:
	lde
	scl
	cai	0x0d
	jnz	__el005
;		
;	}else{
	jmp	__fi005
__el005:
;		p2=#help_msg;puts();
	lea	p2,help_msg
	call	puts
;	}
__fi005:
;
;//}
;
;	}
__fi003:
__fi002:
	jmp	__wh000
__ew000:
;	exit();
	call	exit
;}
	ret
;
;//  Fence:::
;//  ================================================
;//  ここのアドレスが 0x100 以降に配置される必要がある
;//  CALL と RET を上位アドレスが 00 かどうかで判別するため.
;//
;getc()
;{
getc:
;     db(0x21);
	db	0x21
;}
	ret
;putc()
;{
putc:
;     db(0x20);
	db	0x20
;}
	ret
;exit()
;{
exit:
;     db(0);
	db	0
;}
	ret
;
;
;// ==========================================
;// 入力関数
;
;// P2 ポインタの空白文字飛ばし.
;sp_skip()
;{
sp_skip:
;	a=*p2;
	ld	0(p2)
;	while(a==' ') {
__wh006:
	scl
	cai	' '
	jnz	__ew006
;		a=*p2++;  // p2++ だけしたい.
	ld	@1(p2)
;		a=*p2;
	ld	0(p2)
;	}
	jmp	__wh006
__ew006:
;}
	ret
;
;//  p4 <<=4;
;p4mul16()
;{
p4mul16:
;	cnt1=4;
	ldi	4
	st	cnt1(P1)
;	do {
__do007:
;		ccl; a=p4;add p4(p1);p4=a; ld p4+1(p1);add p4+1(p1); st p4+1(p1)
	ccl
	ld	p4(P1)
	add	p4(p1)
	st	p4(P1)
	ld	p4+1(p1)
	add	p4+1(p1)
	st	p4+1(p1)
;	} while(--cnt1);
	dld	cnt1(P1)
	jnz	__do007
__od007:
;}
	ret
;
;// P2 ポインタから16進HEX読み. ==> p4に結果. 入力された桁数=Areg
;readhex()
;{
readhex:
;	// p4=0
;	a=0;
	ldi	0
;	p4=a;
	st	p4(P1)
;	st p4+1(p1);
	st	p4+1(p1)
;	// r4=0
;	r4=a;
	st	r4(P1)
;	while(1) {
__wh008:
;		a=*p2++;e=a;
	ld	@1(p2)
	xae
	lde
;		readhex1();e=a;
	call	readhex1
	xae
	lde
;		if(e!=0xff) {
	lde
	scl
	cai	0xff
	jz	__el009
;			p4mul16();
	call	p4mul16
;			a=e;
	lde
;			a+=p4;p4=a;
	ccl
	add	p4(P1)
	st	p4(P1)
;			a=r4;a+=1;r4=a;
	ld	r4(P1)
	ccl
	adi	1
	st	r4(P1)
;		}else{
	jmp	__fi009
__el009:
;			a=r4;
	ld	r4(P1)
;			return;
	ret
;		}
__fi009:
;	}
	jmp	__wh008
__ew008:
;}
	ret
;
;readhex1()
;{
readhex1:
;	lc();e=a;
	call	lc
	xae
	lde
;	if(e>='0') {
	lde
	scl
	cai	'0'
	jp	$+4
	jmp	__el010
;		if(e<0x3a) { // <='9'
	lde
	scl
	cai	0x3a
	jp	__el011
;			a=e;
	lde
;			a-=0x30;
	scl
	cai	0x30
;			return;
	ret
;		}
;	}
__el011:
;	if(e>='a') {
__el010:
	lde
	scl
	cai	'a'
	jp	$+4
	jmp	__el012
;		if(e<'g') {
	lde
	scl
	cai	'g'
	jp	__el013
;			a=e;
	lde
;			a-=0x57;  // 0x61 - 10
	scl
	cai	0x57
;			return;
	ret
;		}
;	}
__el013:
;	a=0xff;
__el012:
	ldi	0xff
;}
	ret
;
;
;// ==========================================
;// 出力関数
;
;//  アスキーダンプ１行
;//  アドレスは ea1
;ascdump_16()
;{
ascdump_16:
;	push(p2);
	push	p2
;	ldptr(p2,ea1);
	ldptr	p2,ea1
;	ascdump_8();
	call	ascdump_8
;	pr_spc();
	call	pr_spc
;	ascdump_8();
	call	ascdump_8
;	pop(p2);
	pop	p2
;}
	ret
;
;//  アスキーダンプ8byte
;ascdump_8()
;{
ascdump_8:
;	cnt2=8;
	ldi	8
	st	cnt2(P1)
;	do {
__do014:
;		a=*p2++;
	ld	@1(p2)
;		ascdump1();
	call	ascdump1
;	} while(--cnt2);
	dld	cnt2(P1)
	jnz	__do014
__od014:
;}
	ret
;
;//  アスキーダンプ1byte
;ascdump1()
;{
ascdump1:
;	e=a;
	xae
	lde
;	if(e<0x20) {
	lde
	scl
	cai	0x20
	jp	__el015
;		a=' ';e=a;
	ldi	' '
	xae
	lde
;	}
;	if(e>=0x7f) {
__el015:
	lde
	scl
	cai	0x7f
	jp	$+4
	jmp	__el016
;		a=' ';e=a;
	ldi	' '
	xae
	lde
;	}
;	a=e;putc();
__el016:
	lde
	call	putc
;}
	ret
;
;//  大文字にする.
;uc()
;{
uc:
;	e=a;
	xae
	lde
;	if(e>='a') {
	lde
	scl
	cai	'a'
	jp	$+4
	jmp	__el017
;		if(e<0x7b) {  // <='z'
	lde
	scl
	cai	0x7b
	jp	__el018
;			a=e;
	lde
;			a-=0x20;
	scl
	cai	0x20
;			return;
	ret
;		}
;	}
__el018:
;	a=e;
__el017:
	lde
;}
	ret
;
;//  小文字にする.
;lc()
;{
lc:
;	e=a;
	xae
	lde
;	if(e>='A') {
	lde
	scl
	cai	'A'
	jp	$+4
	jmp	__el019
;		if(e<0x5b) {  // <='Z'
	lde
	scl
	cai	0x5b
	jp	__el020
;			a=e;
	lde
;			a+=0x20;
	ccl
	adi	0x20
;			return;
	ret
;		}
;	}
__el020:
;	a=e;
__el019:
	lde
;}
	ret
;
;inc_p2()
;{
inc_p2:
;	xpal(p2);
	xpal	p2
;	ccl ; adi(1);
	ccl
	adi	1
;	xpal(p2);
	xpal	p2
;
;	xpah(p2);
	xpah	p2
;	adi(0);
	adi	0
;	xpah(p2);
	xpah	p2
;}
	ret
;
;//  メモリーダンプ
;//  アドレスは p2
;mdump()
;{
mdump:
;	a=16;cnt1=a;
	ldi	16
	st	cnt1(P1)
;	do {
__do021:
;		mdump_16();
	call	mdump_16
;	} while(--cnt1);
	dld	cnt1(P1)
	jnz	__do021
__od021:
;
;	stptr(p2,p5);
	stptr	p2,p5
;}
	ret
;
;//  メモリーダンプ16byte
;//  アドレスは p2
;mdump_16()
;{
mdump_16:
;	stptr(p2,ea1)
	stptr	p2,ea1
;
;	a=ea2;a<>e;
	ld	ea2(P1)
	xae
;	a=ea1;
	ld	ea1(P1)
;	prhex4();
	call	prhex4
;	pr_spc();
	call	pr_spc
;
;	mdump_8();
	call	mdump_8
;	pr_spc();
	call	pr_spc
;	mdump_8();
	call	mdump_8
;
;	// ASCII DUMP
;	stptr(p2,ea1);
	stptr	p2,ea1
;	sub16(ea1);
	sub16	ea1
;	ascdump_16();
	call	ascdump_16
;
;	put_crlf();
	call	put_crlf
;}
	ret
;
;//  メモリーダンプ8byte
;mdump_8()
;{
mdump_8:
;	cnt2=8;
	ldi	8
	st	cnt2(P1)
;	do {
__do022:
;		a=*p2;
	ld	0(p2)
;		prhex2();
	call	prhex2
;		pr_spc();
	call	pr_spc
;		inc_p2();
	call	inc_p2
;	} while(--cnt2);
	dld	cnt2(P1)
	jnz	__do022
__od022:
;}
	ret
;
;//  EAレジスタを16進4桁表示
;prhex4()
;{
prhex4:
;	ea2=a;
	st	ea2(P1)
;	a<>e;
	xae
;	ea1=a;
	st	ea1(P1)
;
;prhex4ea1:
prhex4ea1:
;	a=ea1;
	ld	ea1(P1)
;	prhex2();
	call	prhex2
;	a=ea2;
	ld	ea2(P1)
;	prhex2();
	call	prhex2
;}
	ret
;
;//  Aレジスタを16進2桁表示
;prhex2()
;{
prhex2:
;	push_ea;
	push_ea
;	e=a;
	xae
	lde
;	a>>=1;
	sr
;	a>>=1;
	sr
;	a>>=1;
	sr
;	a>>=1;
	sr
;	prhex1();
	call	prhex1
;
;	a=e;
	lde
;	prhex1();
	call	prhex1
;	pop_ea;
	pop_ea
;}
	ret
;
;//  Aレジスタ下位4bitのみを16進1桁表示
;prhex1()
;{
prhex1:
;	push_ea;
	push_ea
;	a&=0x0f;
	ani	0x0f
;	e=a;
	xae
	lde
;	if( a >= 10) {
	scl
	cai	10
	jp	$+4
	jmp	__el023
;		a=e;a+=7;
	lde
	ccl
	adi	7
;	}else{
	jmp	__fi023
__el023:
;		a=e;
	lde
;	}
__fi023:
;	a += 0x30;
	ccl
	adi	0x30
;	putc();
	call	putc
;	pop_ea;
	pop_ea
;}
	ret
;//  空白文字を1つ出力
;pr_spc()
;{
pr_spc:
;	a=' ';putc();
	ldi	' '
	call	putc
;}
	ret
;
;//  改行コード出力
;put_crlf()
;{
put_crlf:
;	a=0x0d;putc();
	ldi	0x0d
	call	putc
;	a=0x0a;putc();
	ldi	0x0a
	call	putc
;}
	ret
;
;//  文字列入力( P2 ) 0x0a + ヌル終端.
;gets()
;{
gets:
;	do {
__do024:
;		getc();
	call	getc
;		*p2++=a;
	st	@1(p2)
;		e=a;
	xae
	lde
;		if(e==0x0a) break;
	lde
	scl
	cai	0x0a
	jz	__od024
;		if(e==0x0d) break;
	lde
	scl
	cai	0x0d
	jz	__od024
;	}while(1);	
	jmp	__do024
__od024:
;
;	a=0; *p2++=a;
	ldi	0
	st	@1(p2)
;}
	ret
;
;
;//  文字列出力( P2 )ヌル終端.
;puts()
;{
puts:
;	do {
__do025:
;		a=*p2++;
	ld	@1(p2)
;		if(a==0) break;
	jz	__od025
;		putc();
	call	putc
;	}while(1);	
	jmp	__do025
__od025:
;}
	ret
;
;/*
; ===========================================
; *	SC/MP-II Sample Program
; ===========================================
; セルフ逆アセンブラ
;
;
; ===========================================
; */
;
;//  メモリーダンプ
;//  アドレスは p2
;disasm()
;{
disasm:
;	sp_skip();
	call	sp_skip
;	readhex();
	call	readhex
;	if(a!=0) {
	jz	__el026
;		movew(p5,p4);
	movew	p5,p4
;	}
;	ldptr(p2,p5);
__el026:
	ldptr	p2,p5
;
;	// 16 命令逆アセンブルする.
;	a=16;cnt1=a;
	ldi	16
	st	cnt1(P1)
;	do {
__do027:
;		disasm_1();	//  逆アセンブル 1 命令分.
	call	disasm_1
;	} while(--cnt1);
	dld	cnt1(P1)
	jnz	__do027
__od027:
;
;	stptr(p2,p5);   // P2ptr を p5 ワークに保管.(注目アドレス)
	stptr	p2,p5
;}
	ret
;
;//  逆アセンブル 1 命令分.
;//  アドレスは p2
;disasm_1()
;{
disasm_1:
;	stptr(p2,ea1);
	stptr	p2,ea1
;
;	// アドレス 表示.
;	a=ea2;a<>e;
	ld	ea2(P1)
	xae
;	a=ea1;
	ld	ea1(P1)
;	prhex4();
	call	prhex4
;	pr_spc();
	call	pr_spc
;
;	// HEX + 逆アセンブル
;	disasm_11();
	call	disasm_11
;
;	put_crlf();
	call	put_crlf
;}
	ret
;
;// P2 が 0x0080 未満のとき Areg=0 を返す.
;sub_p2_128()
;{
sub_p2_128:
;	xpah(p2)
	xpah	p2
;	ea1=a;
	st	ea1(P1)
;	xpah(p2)
	xpah	p2
;
;	a=ea1;
	ld	ea1(P1)
;	if(a!=0) {
	jz	__el028
;		return;
	ret
;	}
;	xpal(p2)
__el028:
	xpal	p2
;	ea1=a;
	st	ea1(P1)
;	xpal(p2)
	xpal	p2
;	
;	a=ea1;
	ld	ea1(P1)
;	jp _ok1
	jp	_ok1
;
;	a=0xff;
	ldi	0xff
;	return;
	ret
;
;_ok1:
_ok1:
;	a=0;
	ldi	0
;}
	ret
;
;// HEX Print + 逆アセンブル
;disasm_11()
;{
disasm_11:
;	stptr(p2,pcl);
	stptr	p2,pcl
;	ld 2(p2);op3=a;
	ld	2(p2)
	st	op3(P1)
;	ld 1(p2);op2=a;
	ld	1(p2)
	st	op2(P1)
;
;	a=*p2   ;op1=a;
	ld	0(p2)
	st	op1(P1)
;	a &= 0x80;
	ani	0x80
;	if(a!=0) {
	jz	__el029
;		a=2;goto _ret2;
	ldi	2
	jmp	_ret2
;	}else{
	jmp	__fi029
__el029:
;		a=*p2;
	ld	0(p2)
;		if(a==0x3f) {       // XPPC3 は特別処理.
	scl
	cai	0x3f
	jnz	__el030
;			sub_p2_128();	// P2 < 0x0080 なら Areg=0
	call	sub_p2_128
;			if(a!=0) {
	jz	__el031
;				a=op2;
	ld	op2(P1)
;				if(a==0) {
	jnz	__el032
;					a=2;goto _ret2; // 2byte命令.
	ldi	2
	jmp	_ret2
;				}
;				a=3;goto _ret2; // 3byte命令.
__el032:
	ldi	3
	jmp	_ret2
;			}
;		}
__el031:
;		a=1;goto _ret2;
__el030:
	ldi	1
	jmp	_ret2
;	}
__fi029:
;_ret2:
_ret2:
;	opsize=a;
	st	opsize(P1)
;	hexdump_acnt();
	call	hexdump_acnt
;}
	ret
;
;// 逆アセンブラの オペコード HEX Print
;hexdump_acnt()
;{
hexdump_acnt:
;	cnt2=a;
	st	cnt2(P1)
;	cnt3=4;
	ldi	4
	st	cnt3(P1)
;	do {
__do033:
;		a=*p2;
	ld	0(p2)
;		prhex2();
	call	prhex2
;		pr_spc();
	call	pr_spc
;		inc_p2();
	call	inc_p2
;
;		dld(cnt3(p1));
	dld	cnt3(p1)
;	} while(--cnt2);
	dld	cnt2(P1)
	jnz	__do033
__od033:
;
;	do {
__do034:
;		pr_spc();
	call	pr_spc
;		pr_spc();
	call	pr_spc
;		pr_spc();
	call	pr_spc
;	} while(--cnt3);
	dld	cnt3(P1)
	jnz	__do034
__od034:
;
;	// オペコード名を検索して表示.
;	a=op1;if(a==0x3f) { // XPPC3だけ特別.
	ld	op1(P1)
	scl
	cai	0x3f
	jnz	__el035
;		a=opsize;e=a;
	ld	opsize(P1)
	xae
	lde
;		if(e==3) {      // CALL 判定.
	lde
	scl
	cai	3
	jnz	__el036
;			goto op_callret;
	jmp	op_callret
;		}
;		if(e==2) {      // RET 判定.
__el036:
	lde
	scl
	cai	2
	jnz	__el037
;			goto op_callret;
	jmp	op_callret
;		}
;	}
__el037:
;
;	// XPPC3以外のすべて.
;	push(p2);
__el035:
	push	p2
;	op_find();
	call	op_find
;	pop(p2);
	pop	p2
;}
	ret
;
;inc_op2_op3()
;{
inc_op2_op3:
;	ild(op3(p1))
	ild	op3(p1)
;	if(a==0) {
	jnz	__el038
;		ild(op2(p1))
	ild	op2(p1)
;	}
;}
__el038:
	ret
;
;op_callret()
;{
op_callret:
;	push(p2);
	push	p2
;	a=opsize;
	ld	opsize(P1)
;	if(a==3) {
	scl
	cai	3
	jnz	__el039
;		p2=#mn_call;
	lea	p2,mn_call
;		op_print();
	call	op_print
;		inc_op2_op3();
	call	inc_op2_op3
;		a=op2;prhex2();
	ld	op2(P1)
	call	prhex2
;		a=op3;prhex2();
	ld	op3(P1)
	call	prhex2
;	}else{
	jmp	__fi039
__el039:
;		p2=#mn_ret;
	lea	p2,mn_ret
;		op_print();
	call	op_print
;	}
__fi039:
;	pop(p2);
	pop	p2
;}
	ret
;
;mn_call:
mn_call:
;	db("CALL");
	db	"CALL"
;mn_ret:
mn_ret:
;	db("RET ");
	db	"RET "
;
;//
;// 逆アセンブラの オペコード HEX Print
;//   
;param_print()
;{
param_print:
;	ld 1(p2);  // OPTable のMaskフィールド.
	ld	1(p2)
;	a^=0xff;   // ビット反転させる
	xri	0xff
;	op1mask=a;
	st	op1mask(P1)
;
;	if(opsize==2) {
	ld	opsize(P1)
	scl
	cai	2
	jnz	__el040
;		param_ldop();
	call	param_ldop
;		return;
	ret
;	}
;
;	a=op1mask;
__el040:
	ld	op1mask(P1)
;	if(a!=0) { // Maskフィールドが 0xff (反転させると0) 以外なら,,,
	jz	__el041
;		a=op1mask;a&=op1;prhex2();
	ld	op1mask(P1)
	and	op1(P1)
	call	prhex2
;	}
;}
__el041:
	ret
;
;// ea2:1 = pcl:pch + (sign extend)op2 + 2
;param_addpc_ea1()
;{
param_addpc_ea1:
;	ea1=0;
	ldi	0
	st	ea1(P1)
;	a=op2;a&=0x80;
	ld	op2(P1)
	ani	0x80
;	if(a!=0) ea1=0xff;
	jz	__el042
	ldi	0xff
	st	ea1(P1)
;
;	ccl;
__el042:
	ccl
;	a=pcl;add op2(p1);ea2=a;
	ld	pcl(P1)
	add	op2(p1)
	st	ea2(P1)
;	a=pch;add ea1(p1);ea1=a;
	ld	pch(P1)
	add	ea1(p1)
	st	ea1(P1)
;
;	ccl;
	ccl
;	a=ea2;adi(2);ea2=a;
	ld	ea2(P1)
	adi	2
	st	ea2(P1)
;	a=ea1;adi(0);ea1=a;
	ld	ea1(P1)
	adi	0
	st	ea1(P1)
;}
	ret
;param_jmpop()
;{
param_jmpop:
;	param_addpc_ea1();
	call	param_addpc_ea1
;	prhex4ea1();
	call	prhex4ea1
;}
	ret
;
;param_pcrel:
param_pcrel:
;	a='C';e=a;
	ldi	'C'
	xae
	lde
;param_ptrel()
;{
param_ptrel:
;	a='(';putc();
	ldi	'('
	call	putc
;	a='P';putc();
	ldi	'P'
	call	putc
;	a=e  ;putc();
	lde
	call	putc
;	a=')';putc();
	ldi	')'
	call	putc
;}
	ret
;
;param_ldop()
;{
param_ldop:
;	a=op1;a&=0xf0;
	ld	op1(P1)
	ani	0xf0
;	if(a==0x90) {  // JMP
	scl
	cai	0x90
	jnz	__el043
;		goto param_jmpop;	
	jmp	param_jmpop
;	}
;
;	a=op2;prhex2();
__el043:
	ld	op2(P1)
	call	prhex2
;
;	a=op1;a&=0xc0;
	ld	op1(P1)
	ani	0xc0
;	if(a==0xc0) { 		// LD,ST,AND,...
	scl
	cai	0xc0
	jnz	__el044
;		a=op1;a&=7;e=a; // PC,P1,P2,P3,Imm,@P1,@P2,@P3
	ld	op1(P1)
	ani	7
	xae
	lde
;		if(e==4) {return;} // Imm
	lde
	scl
	cai	4
	jnz	__el045
	ret
;		if(e==0) {goto param_pcrel;}
__el045:
	lde
	jnz	__el046
	jmp	param_pcrel
;		a=e;a&=4;
__el046:
	lde
	ani	4
;		if(a!=0) {a='@';putc();}
	jz	__el047
	ldi	'@'
	call	putc
;		a=e;a&=3;a+='0';e=a;goto param_ptrel;
__el047:
	lde
	ani	3
	ccl
	adi	'0'
	xae
	lde
	jmp	param_ptrel
;	}
;
;}
__el044:
	ret
;
;// オペコード名を検索して表示.
;op_find()
;{
op_find:
;	p2=#mnemonics;
	lea	p2,mnemonics
;	do {
__do048:
;		ld 1(p2);          // OPCODE Mask
	ld	1(p2)
;		a&=op1;
	and	op1(P1)
;		if(a==*p2) {       // OPCODE マッチングした: HALTはここでマッチングするので、問題ない.
	scl
	cad	0(p2)
	jnz	__el049
;			ld @2(p2);     // OPTable ポインタを2byte進める
	ld	@2(p2)
;			op_print();
	call	op_print
;			ld @-6(p2);    // OPTable ポインタを6byte戻す.
	ld	@-6(p2)
;			param_print(); //
	call	param_print
;			return;
	ret
;		}
;		// OPTable終端 (HALT) に来た.
;		a=*p2;
__el049:
	ld	0(p2)
;		if(a==0) {
	jnz	__el050
;			return;
	ret
;		}
;		ld @6(p2);
__el050:
	ld	@6(p2)
;	}while(1);
	jmp	__do048
__od048:
;}
	ret
;
;
;// オペコード名 4 文字と 空白1文字を表示.
;op_print()
;{
op_print:
;	cnt3=4;
	ldi	4
	st	cnt3(P1)
;	do {
__do051:
;		a=*p2++;putc();
	ld	@1(p2)
	call	putc
;	}while(--cnt3);
	dld	cnt3(P1)
	jnz	__do051
__od051:
;	a=' ';putc();
	ldi	' '
	call	putc
;}
	ret
;
;//
;//
;mnemonics:
mnemonics:
;	db(0xc4);
	db	0xc4
;	 db(0xff);
	db	0xff
;	 db("LDI ");
	db	"LDI "
;	db(0xd4);
	db	0xd4
;	 db(0xff);
	db	0xff
;	 db("ANI ");
	db	"ANI "
;	db(0xdc);
	db	0xdc
;	 db(0xff);
	db	0xff
;	 db("ORI ");
	db	"ORI "
;	db(0xe4);
	db	0xe4
;	 db(0xff);
	db	0xff
;	 db("XRI ");
	db	"XRI "
;	db(0xec);
	db	0xec
;	 db(0xff);
	db	0xff
;	 db("DAI ");
	db	"DAI "
;	db(0xf4);
	db	0xf4
;	 db(0xff);
	db	0xff
;	 db("ADI ");
	db	"ADI "
;	db(0xfc);
	db	0xfc
;	 db(0xff);
	db	0xff
;	 db("CAI ");
	db	"CAI "
;	db(0xc0);
	db	0xc0
;	 db(0xf8);
	db	0xf8
;	 db("LD  ");
	db	"LD  "
;	db(0xc8);
	db	0xc8
;	 db(0xf8);
	db	0xf8
;	 db("ST  ");
	db	"ST  "
;	db(0xd0);
	db	0xd0
;	 db(0xf8);
	db	0xf8
;	 db("AND ");
	db	"AND "
;	db(0xd8);
	db	0xd8
;	 db(0xf8);
	db	0xf8
;	 db("OR  ");
	db	"OR  "
;	db(0xe0);
	db	0xe0
;	 db(0xf8);
	db	0xf8
;	 db("XOR ");
	db	"XOR "
;	db(0xe8);
	db	0xe8
;	 db(0xf8);
	db	0xf8
;	 db("DAD ");
	db	"DAD "
;	db(0xf0);
	db	0xf0
;	 db(0xf8);
	db	0xf8
;	 db("ADD ");
	db	"ADD "
;	db(0xf8);
	db	0xf8
;	 db(0xf8);
	db	0xf8
;	 db("CAD ");
	db	"CAD "
;	db(0x90);
	db	0x90
;	 db(0xfc);
	db	0xfc
;	 db("JMP ");
	db	"JMP "
;	db(0x94);
	db	0x94
;	 db(0xfc);
	db	0xfc
;	 db("JP  ");
	db	"JP  "
;	db(0x98);
	db	0x98
;	 db(0xfc);
	db	0xfc
;	 db("JZ  ");
	db	"JZ  "
;	db(0x9c);
	db	0x9c
;	 db(0xfc);
	db	0xfc
;	 db("JNZ ");
	db	"JNZ "
;	db(0x08);
	db	0x08
;	 db(0xff);
	db	0xff
;	 db("NOP ");
	db	"NOP "
;	db(0xa8);
	db	0xa8
;	 db(0xfc);
	db	0xfc
;	 db("ILD ");
	db	"ILD "
;	db(0xb8);
	db	0xb8
;	 db(0xfc);
	db	0xfc
;	 db("DLD ");
	db	"DLD "
;	db(0x40);
	db	0x40
;	 db(0xff);
	db	0xff
;	 db("LDE ");
	db	"LDE "
;	db(0x01);
	db	0x01
;	 db(0xff);
	db	0xff
;	 db("XAE ");
	db	"XAE "
;	db(0x50);
	db	0x50
;	 db(0xff);
	db	0xff
;	 db("ANE ");
	db	"ANE "
;	db(0x58);
	db	0x58
;	 db(0xff);
	db	0xff
;	 db("ORE ");
	db	"ORE "
;	db(0x60);
	db	0x60
;	 db(0xff);
	db	0xff
;	 db("XRE ");
	db	"XRE "
;	db(0x68);
	db	0x68
;	 db(0xff);
	db	0xff
;	 db("DAE ");
	db	"DAE "
;	db(0x70);
	db	0x70
;	 db(0xff);
	db	0xff
;	 db("ADE ");
	db	"ADE "
;	db(0x78);
	db	0x78
;	 db(0xff);
	db	0xff
;	 db("CAE ");
	db	"CAE "
;	db(0x30);
	db	0x30
;	 db(0xfc);
	db	0xfc
;	 db("XPAL");
	db	"XPAL"
;	db(0x34);
	db	0x34
;	 db(0xfc);
	db	0xfc
;	 db("XPAH");
	db	"XPAH"
;	db(0x3c);
	db	0x3c
;	 db(0xfc);
	db	0xfc
;	 db("XPPC");
	db	"XPPC"
;	db(0x19);
	db	0x19
;	 db(0xff);
	db	0xff
;	 db("SIO ");
	db	"SIO "
;	db(0x1c);
	db	0x1c
;	 db(0xff);
	db	0xff
;	 db("SR  ");
	db	"SR  "
;	db(0x1d);
	db	0x1d
;	 db(0xff);
	db	0xff
;	 db("SRL ");
	db	"SRL "
;	db(0x1e);
	db	0x1e
;	 db(0xff);
	db	0xff
;	 db("RR  ");
	db	"RR  "
;	db(0x1f);
	db	0x1f
;	 db(0xff);
	db	0xff
;	 db("RRL ");
	db	"RRL "
;	db(0x02);
	db	0x02
;	 db(0xff);
	db	0xff
;	 db("CCL ");
	db	"CCL "
;	db(0x03);
	db	0x03
;	 db(0xff);
	db	0xff
;	 db("SCL ");
	db	"SCL "
;	db(0x05);
	db	0x05
;	 db(0xff);
	db	0xff
;	 db("IEN ");
	db	"IEN "
;	db(0x04);
	db	0x04
;	 db(0xff);
	db	0xff
;	 db("DINT");
	db	"DINT"
;	db(0x06);
	db	0x06
;	 db(0xff);
	db	0xff
;	 db("CSA ");
	db	"CSA "
;	db(0x07);
	db	0x07
;	 db(0xff);
	db	0xff
;	 db("CAS ");
	db	"CAS "
;	db(0x8f);
	db	0x8f
;	 db(0xff);
	db	0xff
;	 db("DLY ");
	db	"DLY "
;	db(0x20);
	db	0x20
;	 db(0xff);
	db	0xff
;	 db("PUTC");
	db	"PUTC"
;	db(0x21);
	db	0x21
;	 db(0xff);
	db	0xff
;	 db("GETC");
	db	"GETC"
;	db(0x00);
	db	0x00
;	 db(0xff);
	db	0xff
;	 db("HALT");
	db	"HALT"
;//----------------
;	db(0x00);
	db	0x00
;	 db(0x00);
	db	0x00
;	 db(0x00);
	db	0x00
;//
;//
;
;
