/*
 ===========================================
 *	SC/MP-II Sample Program
 ===========================================
 セルフ逆アセンブラ


 ===========================================
 */

//  メモリーダンプ
//  アドレスは p2
disasm()
{
	sp_skip();
	readhex();
	if(a!=0) {
		movew(p5,p4);
	}
	ldptr(p2,p5);

	// 16 命令逆アセンブルする.
	a=16;cnt1=a;
	do {
		disasm_1();	//  逆アセンブル 1 命令分.
	} while(--cnt1);

	stptr(p2,p5);   // P2ptr を p5 ワークに保管.(注目アドレス)
}

//  逆アセンブル 1 命令分.
//  アドレスは p2
disasm_1()
{
	stptr(p2,ea1);

	// アドレス 表示.
	a=ea2;a<>e;
	a=ea1;
	prhex4();
	pr_spc();

	// HEX + 逆アセンブル
	disasm_11();

	put_crlf();
}

// P2 が 0x0080 未満のとき Areg=0 を返す.
sub_p2_128()
{
	xpah(p2)
	ea1=a;
	xpah(p2)

	a=ea1;
	if(a!=0) {
		return;
	}
	xpal(p2)
	ea1=a;
	xpal(p2)
	
	a=ea1;
	jp _ok1

	a=0xff;
	return;

_ok1:
	a=0;
}

// HEX Print + 逆アセンブル
disasm_11()
{
	stptr(p2,pcl);
	ld 2(p2);op3=a;
	ld 1(p2);op2=a;

	a=*p2   ;op1=a;
	a &= 0x80;
	if(a!=0) {
		a=2;goto _ret2;
	}else{
		a=*p2;
		if(a==0x3f) {       // XPPC3 は特別処理.
			sub_p2_128();	// P2 < 0x0080 なら Areg=0
			if(a!=0) {
				a=op2;
				if(a==0) {
					a=2;goto _ret2; // 2byte命令.
				}
				a=3;goto _ret2; // 3byte命令.
			}
		}
		a=1;goto _ret2;
	}
_ret2:
	opsize=a;
	hexdump_acnt();
}

// 逆アセンブラの オペコード HEX Print
hexdump_acnt()
{
	cnt2=a;
	cnt3=4;
	do {
		a=*p2;
		prhex2();
		pr_spc();
		inc_p2();

		dld(cnt3(p1));
	} while(--cnt2);

	do {
		pr_spc();
		pr_spc();
		pr_spc();
	} while(--cnt3);

	// オペコード名を検索して表示.
	a=op1;if(a==0x3f) { // XPPC3だけ特別.
		a=opsize;e=a;
		if(e==3) {      // CALL 判定.
			goto op_callret;
		}
		if(e==2) {      // RET 判定.
			goto op_callret;
		}
	}

	// XPPC3以外のすべて.
	push(p2);
	op_find();
	pop(p2);
}

inc_op2_op3()
{
	ild(op3(p1))
	if(a==0) {
		ild(op2(p1))
	}
}

op_callret()
{
	push(p2);
	a=opsize;
	if(a==3) {
		p2=#mn_call;
		op_print();
		inc_op2_op3();
		a=op2;prhex2();
		a=op3;prhex2();
	}else{
		p2=#mn_ret;
		op_print();
	}
	pop(p2);
}

mn_call:
	db("CALL");
mn_ret:
	db("RET ");

//
// 逆アセンブラの オペコード HEX Print
//   
param_print()
{
	ld 1(p2);  // OPTable のMaskフィールド.
	a^=0xff;   // ビット反転させる
	op1mask=a;

	if(opsize==2) {
		param_ldop();
		return;
	}

	a=op1mask;
	if(a!=0) { // Maskフィールドが 0xff (反転させると0) 以外なら,,,
		a=op1mask;a&=op1;prhex2();
	}
}

// ea2:1 = pcl:pch + (sign extend)op2 + 2
param_addpc_ea1()
{
	ea1=0;
	a=op2;a&=0x80;
	if(a!=0) ea1=0xff;

	ccl;
	a=pcl;add op2(p1);ea2=a;
	a=pch;add ea1(p1);ea1=a;

	ccl;
	a=ea2;adi(2);ea2=a;
	a=ea1;adi(0);ea1=a;
}
param_jmpop()
{
	param_addpc_ea1();
	prhex4ea1();
}

param_pcrel:
	a='C';e=a;
param_ptrel()
{
	a='(';putc();
	a='P';putc();
	a=e  ;putc();
	a=')';putc();
}

param_ldop()
{
	a=op1;a&=0xf0;
	if(a==0x90) {  // JMP
		goto param_jmpop;	
	}

	a=op2;prhex2();

	a=op1;a&=0xc0;
	if(a==0xc0) { 		// LD,ST,AND,...
		a=op1;a&=7;e=a; // PC,P1,P2,P3,Imm,@P1,@P2,@P3
		if(e==4) {return;} // Imm
		if(e==0) {goto param_pcrel;}
		a=e;a&=4;
		if(a!=0) {a='@';putc();}
		a=e;a&=3;a+='0';e=a;goto param_ptrel;
	}

}

// オペコード名を検索して表示.
op_find()
{
	p2=#mnemonics;
	do {
		ld 1(p2);          // OPCODE Mask
		a&=op1;
		if(a==*p2) {       // OPCODE マッチングした: HALTはここでマッチングするので、問題ない.
			ld @2(p2);     // OPTable ポインタを2byte進める
			op_print();
			ld @-6(p2);    // OPTable ポインタを6byte戻す.
			param_print(); //
			return;
		}
		// OPTable終端 (HALT) に来た.
		a=*p2;
		if(a==0) {
			return;
		}
		ld @6(p2);
	}while(1);
}


// オペコード名 4 文字と 空白1文字を表示.
op_print()
{
	cnt3=4;
	do {
		a=*p2++;putc();
	}while(--cnt3);
	a=' ';putc();
}
