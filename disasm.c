/** *********************************************************************************
 *	オペコードの逆アセンブル
 ************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcode.h"
//	後続する16bit即値が存在する命令なら 1 を返す.
int	is_imm16(int code);

int   ea_dump = 1;
char  memreport_buf[256];
char *memreport(void)
{
	return memreport_buf;
}

void eadump(int ea)
{
	if(ea_dump) {
		sprintf(memreport_buf,"adr = %04x %02x",ea,memory[ea]);
	}
}


/** *********************************************************************************
//	ディスティネーション（オペランド）を文字列で出力.
 ************************************************************************************
 */
int	gen_dst_string(int code,int code2,char *dst)
{
	int ea = 0;
	int mode = code & a_MASK;
	char *pre="";
	char *rel="";
	char val[256]="";
	if(code2<0x80) {
		sprintf(val,"%d",code2);
	}else{
		sprintf(val,"%d",code2 - 256);
	}
	
	// アドレッシング修飾子(mode)
	switch(mode) {
	 case a_PCR:   rel="(pc)";break;
	 case a_PTR1:  rel="(p1)";break;
	 case a_PTR2:  rel="(p2)";break;
	 case a_PTR3:  rel="(p3)";break;
		
	 case a_IMM:   pre="#";
	     sprintf(val,"0x%02x",code2);
		 break;
	 case a_APTR1: pre="@";rel="(p1)";break;
	 case a_APTR2: pre="@";rel="(p2)";break;
	 case a_APTR3: pre="@";rel="(p3)";break;

	 default:	break;
		
	}
	sprintf(dst,"%s%s%s",pre,val,rel);
	
	return ea;
}


/** *********************************************************************************
//	ディスティネーション（オペランド）を文字列で出力.
 ************************************************************************************
 */
int	gen_jmp_string(int code,int code2,char *dst)
{
	int off = code2;
	if(off>=0x80) {
		off = off - 0x100;
	}
	int ea = (reg.pc_bak + 1 + 2 + off) & 0xffff;
	sprintf(dst,"%04x",ea);
	
	return ea;
}


/* Complement and add (ie sub) - a ones complement machine at heart */
static char *cpu_flags(uint8_t s)
{
    static char buf[9];
    char *p = buf;
    char *x = "COBAI210";
    
    while(*x) {
        if (s & 0x80)
            *p++ = *x;
        else
            *p++ = '-';
        x++;
        s <<= 1;
    }
    return buf;
}

   

int disasm(char *buf,int code,OPCODE *tab)
{
	int  pc = (reg.pc_bak + 1) & 0xffff;
	int  code2=memory[reg.pc_bak + 2];
	char opr[80]="";
	char dst[80]="";
	
	opr[0] = 0;
	dst[0] = 0;
	
	if(code >= 0x80) {
		sprintf(opr,"%02x",code2);

		if((code >= 0x90)&&(code < 0x9f)) {
			gen_jmp_string(code,code2,dst);
		}else{
			gen_dst_string(code,code2,dst);
		}
	}else{
		strcpy(opr,"  ");

		if( (code & 0xf0)==0x30) {
			sprintf(dst,"P%d",code & 3);
		}
	}
	
	sprintf(buf,"%04x: %02x %2s    %-5s %-9s : %s EA:%02x%02x P1:%04x P2:%04x P3:%04x %s\n"
		,pc
		,code
		,opr
		,tab->mnemonic
		,dst

		,cpu_flags(reg.sr)
		,reg.e
		,reg.a
		,reg.p1
		,reg.p2
		,reg.p3
//		,tab->comment
		,memreport()
	);
	return 0;
}
#if 0
/** *********************************************************************************
 *	
 ************************************************************************************
 */
int d_JMP (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_JBP (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_JM  (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_JNM (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_JF  (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_JNF (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_JE  (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_JNE (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_JC  (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_JNC (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_LD  (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_ADD (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_SUB (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_ADC (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_SBB (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_AND (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_OR  (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_XOR (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_LDP (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_LDV (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_OUT (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
/*
int d_STP (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
*/
int d_SFR (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_SFL (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_IN  (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_SCN (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_HLT (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_XPAL (int code,OPCODE *tab)
{
	return disasm(code,tab);
}
int d_UND (int code,OPCODE *tab)
{
	return disasm(code,tab);
}

#endif

/** *********************************************************************************
 *
 ************************************************************************************
 */
