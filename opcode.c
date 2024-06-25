/** *********************************************************************************
 *	オペコードの解釈実行.
 ************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define Extern /* */

#include "opcode.h"
//#include "hash.h"


#define OP_TEST 0

int   ea_dump = 1;     // 実効アドレスの表示On : 1
void  eadump(int ea);  // 実効アドレスをトレースログに追加する.

extern int opcode2;
extern char  opt_q;


/* Binary */
static void CPUadd(uint8_t b)
{
    int ov = 0;
    int r  = reg.a + b;
    if (reg.sr & sr_CY) {
        r++;
    }

	if(( (reg.a & 0x80)==(b & 0x80) )&&( (reg.a & 0x80)!=(r & 0x80) )) {
		ov = sr_OV;
	}

    reg.sr &= ~(sr_CY|sr_OV);

	if( r & 0x100 ) reg.sr |= sr_CY;
	
	reg.sr |= ov;

	reg.a = r;
}

/* BCD */
static void CPUdad(uint8_t b)
{
    uint8_t ln = (reg.a & 0x0F) + (b & 0x0F);
    uint8_t hn = (reg.a & 0xF0) + (b & 0xF0);

    /* Add carry in to low nibble */
    if (reg.sr & sr_CY)
        ln++;
    /* Carry between nibbles */
    if (ln > 0x09) {
        ln -= 0x0A;
        hn += 0x10;
    }
    /* Carry from the high nibble int CL */
    if (hn > 0x90) {
        hn -= 0xA0;
        reg.sr |= sr_CY;
    } else
        reg.sr &= ~ sr_CY;

    reg.a = hn + ln;
}

/** *********************************************************************************
 *
 ************************************************************************************
 */
//int	 str_cmpi(char *t,char *s);
inline int get_operand(void)
{
	return opcode2;
}


//
//	文字列(s)に文字(a)が含まれていれば、その位置を文字数(>=0)で返す.
//	含まれていなければ -1 を返す.
int	str_index(char *s,int a)
{
	int c;
	int idx=0;
	while(1) {
		c = *s;
		if(c==0) return -1;
		if(c==a) return idx;
		s++;idx++;
	}
}


/** *********************************************************************************
 *	１文字(c)が１６進数なら、数値に変換する.
 ************************************************************************************
 *	エラーの場合は (-1)
 */
int	is_hex(int c)
{
	if((c>='0')&(c<='9')) return c-'0';
	if((c>='A')&(c<='F')) return c-'A'+10;
	if((c>='a')&(c<='f')) return c-'a'+10;
	return -1;
}
/** *********************************************************************************
 *	文字列(src)を数値、もしくは１６進数値に変換して(*val)に入れる.
 ************************************************************************************
 *	最後に'H'が付いているものだけ１６進数値に変換する.
 *
 *	成功すれば(1) 失敗すれば(0)を返す.
 */
int	is_numhex(char *src,int *val)
{
	int d=0,hex=0;
	int c,x,hexf=0;

	c = *src++;
	d = is_hex(c);hex=d;
	if(d>=10) hexf=1;

	if(d==(-1)) return 0;	// Error
	while(1) {
		c = *src++;
		if(c==0) {
			if(hexf==0) {
				*val = d;	// 10進.
				return 1;	// OK
			}
			return 0;		// Error
		}
		if((c=='h')||(c=='H')) {
			if(*src==0) {
				*val = hex;
				return 1;	// OK
			}
		}
		x = is_hex(c);
		if(x == (-1)) {
			return 0;		// Error
		}
		if(x>=10) hexf=1;
		d=d*10+x;
		hex=hex*16+x;
	}
}

/* The weird 12bit pointer maths */
inline uint16_t add12(uint16_t a, char off)
{
    uint16_t top = a & 0xF000;
    a += off;
    a &= 0x0FFF;
    a |= top;
    return a;
}
inline int get_op_offset(int ptr)
{
	int off = get_operand();
	if( off == 0x80) { off = reg.e; }
	
	if( off >= 0x80) { off = off - 0x100;}
	return add12(ptr,off);
}
inline int get_op_offset_ai(WORD *pptr)
{
	int off = get_operand();
	int ptr = *pptr;
	int postinc=1; // 後でincrementする.
	if( off == 0x80) { off = reg.e; }
	
	if( off >= 0x80) { off = off - 0x100; postinc=0;} // Pre Decrimentになる.
	*pptr = add12(ptr,off); // Indexレジスタを更新.
	
	if(postinc) {    // 後でincrementする場合.
		return ptr;  // 増減する前のポインタを返す.
	}else{
		return *pptr;// 更新されたポインタを返す.
	}
}
/** *********************************************************************************
 *	オペコードを解析して、実効アドレス(ea) を求める.
 ************************************************************************************
 *  reg.pc をインクリメントすること!!
 * 
 *	実効アドレスの評価結果 ea を返す.
 */
//inline 
int	opadrs(int code)
{
	int mode = code & a_MASK;
	int ea  = 0;

	// アドレッシング修飾子(xx)
	switch(mode) {
	 case a_PCR:   ea = get_op_offset(reg.pc); break;
	 case a_PTR1:  ea = get_op_offset(reg.p1); break;
	 case a_PTR2:  ea = get_op_offset(reg.p2); break;
	 case a_PTR3:  ea = get_op_offset(reg.p3); break;
		
	 case a_IMM:   ea = reg.pc; break;
		
	 case a_APTR1: ea = get_op_offset_ai(&reg.p1); break;
	 case a_APTR2: ea = get_op_offset_ai(&reg.p2); break;
	 case a_APTR3: ea = get_op_offset_ai(&reg.p3); break;

	 default:	break;
	}

	// 操作対象メモリーをPrint (ST系はこのPrintをさらに上書きするので注意)
	if(mode != a_IMM) {
		if(ea_dump) {
			eadump(ea);
		}
	}

	return ea;
}

int	operand(int code)
{
	int	ea = opadrs(code);
	return memory[ea];
}

int	jpadrs(int iptr)
{
	WORD *ptr= &reg.pc;
	int off = get_operand();
	if(off>=0x80) {
		off = off - 0x100;
	}
//	return (reg.pc + off) & 0xffff;
	int page = ptr[iptr] & 0xf000;
	
	return ((ptr[iptr] + off) & 0x0fff)|page;
}

/** *********************************************************************************
 *	こっから下は、命令の実行を行なう.
 ************************************************************************************
 {"JMP","JMP always"  ,"00xxdd00"  ,f_JMP,d_JMP},//-,-,-,無条件に分岐する
 */
int f_JMP (int code,OPCODE *tab)
{
	int ea = jpadrs(code & 3);
	reg.pc = ea;
	return 0;
}
int f_JZ  (int code,OPCODE *tab)
{
	int ea = jpadrs(code & 3);
	if(	reg.a == 0 ) {
		reg.pc = ea;
	}
	return 0;
}
int f_JNZ (int code,OPCODE *tab)
{
	int ea = jpadrs(code & 3);
	if(	reg.a != 0 ) {
		reg.pc = ea;
	}
	return 0;
}
int f_JP  (int code,OPCODE *tab)
{
	int ea = jpadrs(code & 3);
	if(	(reg.a & 0x80)==0 ) {
		reg.pc = ea;
	}
	return 0;
}

int f_LD (int code,OPCODE *tab)
{
	int acc = operand(code);
	reg.a = acc;
	return 0;
}

int f_LDI (int code,OPCODE *tab)
{
	int acc = operand(code);
	reg.a = acc;
	return 0;
}


//#define Wfence 0x1000
#define Wfence 0x40

int f_ST (int code,OPCODE *tab)
{
	int ea = opadrs(code);
	if(ea>=Wfence) {
		memory[ea] = reg.a;
	}else{
		printf("** WRITE VIOLATIONS ** %x: ST 0x%x\n",reg.pc,ea);
		exit(1);
	}
	if(ea_dump) {
		eadump(ea);
	}
	return 0;
}

int f_ADD (int code,OPCODE *tab)
{
	CPUadd( operand(code) );
	return 0;
}

int f_ADI (int code,OPCODE *tab)
{
	CPUadd( operand(code) );
	return 0;
}

int f_DAD (int code,OPCODE *tab)
{
	CPUdad( operand(code) );
	return 0;
}

int f_DAI (int code,OPCODE *tab)
{
	CPUdad( operand(code) );
	return 0;
}

int f_CAD (int code,OPCODE *tab)
{
	CPUadd(0xff ^  operand(code) );
	return 0;
}

int f_CAI (int code,OPCODE *tab)
{
	CPUadd(0xff ^  operand(code) );
	return 0;
}

int f_AND (int code,OPCODE *tab)
{
	int acc = reg.a & operand(code);
	reg.a = acc;
	return 0;
}

int f_ANI (int code,OPCODE *tab)
{
	int acc = reg.a & operand(code);
	reg.a = acc;
	return 0;
}

int f_OR (int code,OPCODE *tab)
{
	int acc = reg.a | operand(code);
	reg.a = acc;
	return 0;
}

int f_ORI(int code,OPCODE *tab)
{
	int acc = reg.a | operand(code);
	reg.a = acc;
	return 0;
}

int f_XOR (int code,OPCODE *tab)
{
	int acc = reg.a ^ operand(code);
	reg.a = acc;
	return 0;
}

int f_XRI (int code,OPCODE *tab)
{
	int acc = reg.a ^ operand(code);
	reg.a = acc;
	return 0;
}

int f_NOP (int code,OPCODE *tab)
{
	return 0;
}

int f_ILD (int code,OPCODE *tab)
{
	int	ea = opadrs(code);
	int a  = memory[ea];
	a++;
	reg.a = a;

	if(ea>=Wfence) {
		memory[ea] = reg.a;
	}else{
		printf("** WRITE VIOLATIONS ** ILD 0x%x\n",ea);
		exit(1);
	}
	if(ea_dump) {
		eadump(ea);
	}
	return 0;
}
int f_DLD (int code,OPCODE *tab)
{
	int	ea = opadrs(code);
	int a  = memory[ea];
	a--;
	reg.a = a;

	if(ea>=Wfence) {
		memory[ea] = reg.a;
	}else{
		printf("** WRITE VIOLATIONS ** DLD 0x%x\n",ea);
		exit(1);
	}
	if(ea_dump) {
		eadump(ea);
	}
	return 0;
}

int f_XPAL (int code,OPCODE *tab)
{
	int regno = code & 3;
	char *ptr= (char*) &reg.pc;
	ptr = ptr + (regno * 2);
	
	int c = ptr[0];
	ptr[0] = reg.a;
	reg.a = c;
	
	return 0;
}

int f_XPAH (int code,OPCODE *tab)
{
	int regno = code & 3;
	char *ptr= (char*) &reg.pc;
	ptr = ptr + (regno * 2) + 1;
	
	int c = ptr[0];
	ptr[0] = reg.a;
	reg.a = c;
	
	return 0;
}

int f_XPPC (int code,OPCODE *tab)
{
	int regno = code & 3;
	WORD *ptr= (WORD*) &reg.pc;
	ptr = ptr + regno;

	int pn = ptr[0];
	ptr[0] = reg.pc;
	reg.pc = pn;
	
	return 0;
}

int f_DLY (int code,OPCODE *tab)
{
	int acc = operand(code);
	reg.a = acc;
	return 0;
}




int f_UND (int code,OPCODE *tab)
{
	printf("# f_UND\n");
	return 1;
}

int f_HALT (int code,OPCODE *tab)
{
	printf("# f_HALT\n");
	return 1;
}

int f_GETC (int code,OPCODE *tab)
{
	int c = getchar();
	if((c>='a')&&(c<='z')) c=c-0x20; // ToLower!
	if(c==0x0a) c=0x0d; // LF--> CR
	
	reg.a = reg.e = c;
	return 0;
}

int f_PUTC (int code,OPCODE *tab)
{
	
	if(opt_q) {
		putchar(reg.a & 0x7f);
		LogPrint("\nPUTC():%c\n",reg.a & 0x7f);
	}else{
		printf("\nPUTC():%c\n",reg.a & 0x7f);
	}
	return 0;
}

int f_LDE (int code,OPCODE *tab)
{
	reg.a = reg.e;
	return 0;
}
int f_XAE (int code,OPCODE *tab)
{
	int t = reg.a;
	reg.a = reg.e;
	reg.e = t;
	return 0;
}
int f_ANE (int code,OPCODE *tab)
{
	reg.a &= reg.e;
	return 0;
}
int f_ORE (int code,OPCODE *tab)
{
	reg.a |= reg.e;
	return 0;
}
int f_XRE (int code,OPCODE *tab)
{
	reg.a ^= reg.e;
	return 0;
}
int f_DAE (int code,OPCODE *tab)
{
	CPUdad(reg.e);
	return 0;
}
int f_ADE (int code,OPCODE *tab)
{
	CPUadd(reg.e);
	return 0;
}
int f_CAE (int code,OPCODE *tab)
{
	CPUadd(0xff ^ reg.e);
	return 0;
}
int f_SIO (int code,OPCODE *tab)
{
	reg.a >>=1 ; // Serial In がMSBに入り、LSBがSerial Outに出る
	return 0;
}
int f_SR (int code,OPCODE *tab)
{
	reg.a >>=1 ;
	return 0;
}
int f_SRL (int code,OPCODE *tab)
{
	reg.a >>=1 ;
	reg.a |= (reg.sr & sr_CY) ;
	return 0;
}
int f_RR (int code,OPCODE *tab)
{
	int t = reg.a & 1;
	reg.a >>=1 ;
	if(t) reg.a |= 0x80; // LSB->MSB
	return 0;
}
int f_RRL (int code,OPCODE *tab)
{
	int acc = reg.a;
	int sr  = reg.sr;
	
	reg.a >>=1;
	if(acc & 1) {
		reg.sr |= sr_CY;
	}else{
		reg.sr &= ~sr_CY;
	}
	
	if(sr & sr_CY) {
		reg.a |= 0x80;
	}else{
		reg.a &= 0x7f;
	}
	
#if 0
	int t = (reg.a & 1)<<7;
	reg.a >>=1 ;
	reg.a |= (reg.sr & sr_CY) ;
	reg.sr = (reg.sr & 0x7f) | t;
#endif
	return 0;
}
int f_CCL (int code,OPCODE *tab)
{
	reg.sr = reg.sr & (0xff ^ sr_CY);
	return 0;
}
int f_SCL (int code,OPCODE *tab)
{
	reg.sr = reg.sr | (sr_CY);
	return 0;
}
int f_IEN (int code,OPCODE *tab)
{
	reg.sr = reg.sr & (0xff ^ sr_IE);
	return 0;
}
int f_DINT (int code,OPCODE *tab)
{
	reg.sr = reg.sr | (sr_IE);
	return 0;
}
int f_CSA (int code,OPCODE *tab)
{
	reg.a = reg.sr;
	return 0;
}
int f_CAS (int code,OPCODE *tab)
{
	reg.sr = (reg.a & 0xef) | (reg.sr & 0x10);
	return 0;
}

