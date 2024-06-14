/************************************************************************************
 *
 ************************************************************************************
 */
#ifndef	opcode_h_
#define	opcode_h_

//#include "hash.h"

typedef	unsigned short WORD;
typedef	unsigned char  BYTE;
typedef	unsigned short uint16_t;
typedef	unsigned char  uint8_t;

struct _OPCODE ;		// ちょこっと宣言.

#ifndef Extern
#define Extern extern
#endif

//	命令実行関数のプロトタイプ型.
typedef int (*EMUFUNC) (int code,struct _OPCODE *tab);
//	命令逆アセンブル関数のプロトタイプ型.
typedef int (*DISFUNC) (int code,struct _OPCODE *tab);


typedef	struct _OPCODE {
	char *mnemonic;			// ニモ
	char *comment;			// 意味
	int   pattern;			// 機械語
	int   bitmask;			// 
	EMUFUNC emufunc;		// 命令実行関数
	DISFUNC disfunc;		// 命令逆アセンブル関数
	int	  data;				// 
} OPCODE;

#define a_MASK  7
#define a_PCR   0
#define a_PTR1  1
#define a_PTR2  2
#define a_PTR3  3
#define a_IMM   4
#define a_APTR1 5
#define a_APTR2 6
#define a_APTR3 7

#define	MEMSIZE		0x10000			// 実装メモリーサイズ (WORD)

/** *********************************************************************************
 *	メモリーコンテキスト
 ************************************************************************************
 */
Extern BYTE  memory[MEMSIZE];

/** *********************************************************************************
 *	レジスタコンテキスト
 ************************************************************************************
 */
typedef	struct _SCMP_CONTEXT {
	WORD	pc;
	WORD	p1;
	WORD	p2;
	WORD	p3;
	BYTE	a;  // Acc
	BYTE	e;  // E
	BYTE    sr; // CY/OV/SB/SA/IE/F2/F1/F0
	            // SB = Sense B (Input)
	            // SA = Sense A (Input)
				// IE = Interrupt Enable(1)
				// F2 = Flag2 (Output)
				// F1 = Flag1 (Output)
				// F0 = Flag0 (Output)
	
	WORD	pc_bak;	// jumpする前のPC.	(逆アセンブル時に必要)
} SCMP_CONTEXT;

Extern SCMP_CONTEXT	reg;


#define	EMUFUNC_(x_)	int x_(int code,struct _OPCODE *tab)
#define	DISFUNC_(x_)	int x_(int code,struct _OPCODE *tab)

#define sr_CY 0x80
#define sr_OV 0x40
#define sr_IE 0x08


//	命令実行関数のプロトタイプ宣言.
EMUFUNC_( f_LD  );
EMUFUNC_( f_ST  );
EMUFUNC_( f_AND );
EMUFUNC_( f_OR  );
EMUFUNC_( f_XOR );
EMUFUNC_( f_DAD );
EMUFUNC_( f_ADD );
EMUFUNC_( f_CAD );

EMUFUNC_( f_LDI );

EMUFUNC_( f_ANI );
EMUFUNC_( f_ORI );
EMUFUNC_( f_XRI );
EMUFUNC_( f_DAI );
EMUFUNC_( f_ADI );
EMUFUNC_( f_CAI );

EMUFUNC_( f_JMP );
EMUFUNC_( f_JP  );
EMUFUNC_( f_JZ  );
EMUFUNC_( f_JNZ );

EMUFUNC_( f_ILD);
EMUFUNC_( f_DLD);
EMUFUNC_( f_LDE);
EMUFUNC_( f_XAE);
EMUFUNC_( f_ANE);
EMUFUNC_( f_ORE);
EMUFUNC_( f_XRE);
EMUFUNC_( f_DAE);
EMUFUNC_( f_ADE);
EMUFUNC_( f_CAE);


EMUFUNC_( f_DLY );
EMUFUNC_( f_NOP );
EMUFUNC_( f_HLT );
EMUFUNC_( f_XPAL );
EMUFUNC_( f_XPAH );
EMUFUNC_( f_XPPC );


EMUFUNC_( f_SIO);
EMUFUNC_( f_SR );
EMUFUNC_( f_SRL);
EMUFUNC_( f_RR );
EMUFUNC_( f_RRL);
EMUFUNC_( f_CCL);
EMUFUNC_( f_SCL);
EMUFUNC_( f_IEN);
EMUFUNC_( f_DINT);
EMUFUNC_( f_CSA);
EMUFUNC_( f_CAS);

EMUFUNC_( f_ORG );
EMUFUNC_( f_EQU );
EMUFUNC_( f_DW  );
EMUFUNC_( f_DOT );
EMUFUNC_( f_END );

EMUFUNC_( f_HALT );
EMUFUNC_( f_GETC );
EMUFUNC_( f_PUTC );
EMUFUNC_( f_UND );
EMUFUNC_( d_XPAL );
EMUFUNC_( d_UND );


// アドレスモード.
enum {
	d_PC,
	d_P1,
	d_P2,
	d_P3,
	d_IMM,
	d_P1A,
	d_P2A,
	d_P3A,
};

//
//	入出力関数.
//
void LED_output(int acc , int ea);
int	 SW_input();
int	 JOY_input();

#define	ZZ	printf("%s:%d: ZZ\n",__FILE__,__LINE__);

void LogPrint(char *fmt,...);

#endif	//opcode_h_
