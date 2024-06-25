/** *********************************************************************************
 *	
 ************************************************************************************
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "opcode.h"

#define USE_GUI 0

/*
 *	オプション文字列チェック
 *		optstring に含まれるオプション文字は、
 *				  後続パラメータ必須とみなす。
 */
#define Getopt(argc,argv,optstring)           		\
 {int i;int c;for(i=0;i<128;i++) opt[i]=NULL; 		\
   while( ( argc>1 )&&( *argv[1]=='-') ) {    		\
	 c = argv[1][1] & 0x7f;   						\
       opt[c] = &argv[1][2] ; 						\
       if(( *opt[c] ==0 )&&(strchr(optstring,c))) {	\
         opt[c] = argv[2] ;argc--;argv++;          	\
       }                        					\
     argc--;argv++;           						\
 } }

#define IsOpt(c) ((opt[ c & 0x7f ])!=NULL)
#define   Opt(c)   opt[ c & 0x7f ]
char *opt[256];
char  opt_q=0;
char  opt_t=0;
extern int  ea_dump;

int  opcode2;
typedef long long int64;

OPCODE opcode_init_tab[]={
 //ニモ,意味					,機械語,MASK,f_emu,d_dis --------------------
 {"LDI","Load Imm"              ,0xc4  ,0x00,f_LDI,0},//
 {"ANI","AND Imm"               ,0xd4  ,0x00,f_ANI,0},//
 {"ORI","OR Imm"                ,0xdc  ,0x00,f_ORI,0},//
 {"XRI","eXcusive OR Imm"       ,0xe4  ,0x00,f_XRI,0},//
 {"DAI","ADD Decimal Imm"       ,0xec  ,0x00,f_DAI,0},//
 {"ADI","ADD Imm"               ,0xf4  ,0x00,f_ADI,0},//
 {"CAI","Compliment ADD Imm"    ,0xfc  ,0x00,f_CAI,0},//
	
 {"LD" ,"Load"                  ,0xc0  ,0x07,f_LD ,0},//
 {"ST" ,"Store"                 ,0xc8  ,0x07,f_ST ,0},//
 {"AND","AND"                   ,0xd0  ,0x07,f_AND,0},//
 {"OR" ,"OR"                    ,0xd8  ,0x07,f_OR ,0},//
 {"XOR","eXcusive OR"           ,0xe0  ,0x07,f_XOR,0},//
 {"DAD","ADD Decimal"           ,0xe8  ,0x07,f_DAD,0},//
 {"ADD","ADD"                   ,0xf0  ,0x07,f_ADD,0},//
 {"CAD","Compliment ADD"        ,0xf8  ,0x07,f_CAD,0},//

 {"JMP","JUMP"                  ,0x90  ,0x03,f_JMP,0},//
 {"JP","JUMP if plus"           ,0x94  ,0x03,f_JP ,0},//
 {"JZ","JUMP if zero"           ,0x98  ,0x03,f_JZ ,0},//
 {"JNZ","JUMP if not zero"      ,0x9c  ,0x03,f_JNZ,0},//
 {"NOP",""                      ,0x08  ,0x00,f_NOP,0},    // 5 1      No operation
 {"HALT",""                     ,0x00  ,0x00,f_HALT,0},    // 8 1      Output Halt pulse

 {"ILD","",   0xA8 ,0x03,f_ILD ,0},    //22 2      AC, EA <- (EA) + 1
 {"DLD","",   0xB8 ,0x03,f_DLD ,0},    //22 2      AC, EA <- (EA) - 1
 {"LDE","",   0x40 ,0x00,f_LDE ,0},    // 6 1      AC <- (E)
 {"XAE","",   0x01 ,0x00,f_XAE ,0},    // 7 1      (AC) <-> (E)
 {"ANE","",   0x50 ,0x00,f_ANE ,0},    // 6 1      AC <- (AC) & (E)
 {"ORE","",   0x58 ,0x00,f_ORE ,0},    // 6 1      AC <- (AC) | (E)
 {"XRE","",   0x60 ,0x00,f_XRE ,0},    // 6 1      AC <- (AC) ^ (E)
 {"DAE","",   0x68 ,0x00,f_DAE ,0},    //11 1  *   AC <- (AC) + (E) + (CY/L) {BCD format}
 {"ADE","",   0x70 ,0x00,f_ADE ,0},    // 7 1  **  AC <- (AC) + (E) + (CY/L)
 {"CAE","",   0x78 ,0x00,f_CAE ,0},    // 8 1  **  AC <- (AC) + !(E) + (CY/L)
 {"XPAL","",  0x30 ,0x03,f_XPAL,0},    // 8 1      (AC) <-> (PL)
 {"XPAH","",  0x34 ,0x03,f_XPAH,0},    // 8 1      (AC) <-> (PH)
 {"XPPC","",  0x3C ,0x03,f_XPPC,0},    // 7 1      (PC) <-> (PTR)

 {"SIO","",   0x19 ,0x00,f_SIO,0},    // 5 1      Serial Input/Output
 {"SR" ,"Shift Right",   0x1C ,0x00,f_SR ,0},    // 5 1      Shift Right
 {"SRL","Shift Right with Link",   0x1D ,0x00,f_SRL,0},    // 5 1      Shift Right with Link
 {"RR" ,"Rotate Right",   0x1E ,0x00,f_RR ,0},    // 5 1      Rotate Right
 {"RRL","Rotate Right with Link",   0x1F ,0x00,f_RRL,0},    // 5 1  *   Rotate Right with Link
 {"CCL","CY/L <- 0",   0x02 ,0x00,f_CCL,0},    // 5 1      CY/L <- 0
 {"SCL","CY/L <- 1",   0x03 ,0x00,f_SCL,0},    // 5 1      CY/L <- 1
 {"IEN","IE <- 1",   0x05 ,0x00,f_IEN,0},    // 6 1      IE <- 1
 {"DINT","IE <- 0",  0x04 ,0x00,f_DINT,0},    // 6 1      IE <- 0
 {"CSA","AC <- SR",   0x06 ,0x00,f_CSA,0},    // 5 1      AC <- (SR)
 {"CAS","SR <- AC",   0x07 ,0x00,f_CAS,0},    // 6 1      SR <- (AC)
 {"DLY","",   0x8F ,0x00,f_DLY,0},    //?? 2      Delay
 {"PUTC","",   0x20 ,0x00,f_PUTC ,0}, 
 {"GETC","",   0x21 ,0x00,f_GETC ,0}, 

 //ニモ,意味					,機械語		 ,f_emu,d_dis  //z,c,m,動作 --------------------
 { NULL,NULL,0,0}
};

OPCODE UNDEFINED_OPCODE=
 {"???","???",0    ,0,f_UND,0};

OPCODE code_table[256];		// 構築したい表 code_table[256];

FILE *ifp;

#define Ropen(name) {ifp=fopen(name,"rb");if(ifp==NULL) \
{ printf("Fatal: can't open file:%s\n",name);exit(1);}  \
}
#define Read(buf,siz)   fread (buf,1,siz,ifp)
#define Rclose()  fclose(ifp)


void memdump(int adr,int len);
void VRAM_output(int adrs,int data);
int  disasm(char *buf,int code,OPCODE *tab);
int64_t  get_cputime();


struct   timespec pastTime;
void     print_vcount();
int64_t  icount   = 0;  // Instruction count
int64_t  clocksum = 0;  // Instruction clocks

//#define  MAX_ICOUNT 1000

int64_t get_cputime() {
	struct timespec currentTime;
	int64_t diffn;
	int64_t diffs;
	int64_t diffu;
	clock_gettime(CLOCK_REALTIME, &currentTime);
	
//	printf("Current Time: %lu Sec + %lu nanoSec.\n",
//			              (long)currentTime.tv_sec, currentTime.tv_nsec);

	diffn = currentTime.tv_nsec - pastTime.tv_nsec;
	diffs = currentTime.tv_sec  - pastTime.tv_sec;
	
	diffu  = diffn / 1000;
	diffu += diffs * 1000 * 1000;
	
	pastTime = currentTime;
	return diffu;
}

#ifdef _MSDOS_

void print_vcount(int64_t usec)
{
	fprintf(stderr,	"\n");
	fprintf(stderr,	"icount = %I64d\n",icount);
//	fprintf(stderr,	"clocks = %I64d\n",clocksum);
	clocksum = icount * 18;
	fprintf(stderr,	"  usec = %I64d\n",usec  );
	fprintf(stderr,	"  MIPS = %I64d\n",icount   / usec);
	fprintf(stderr,	"CPU MHz= %I64d\n",clocksum / usec);
}
#else
void print_vcount(int64_t usec)
{
	fprintf(stderr,	"\n");
	fprintf(stderr,	"icount = %ld\n",icount);
//	fprintf(stderr,	"clocks = %ld\n",clocksum);
	clocksum = icount * 18;
	fprintf(stderr,	"  usec = %ld\n",usec  );
	fprintf(stderr,	"  MIPS = %ld\n",icount   / usec);
	fprintf(stderr,	"CPU MHz= %ld\n",clocksum / usec);
}
#endif

/** *********************************************************************************
 *	命令コード(code) が、OPCODE表の１要素(*s)  にマッチするか判定.
 ************************************************************************************
 *	マッチしたら 0 を返す.
 */
int match_table(OPCODE *s,int code)
{
	int pat  = s->pattern;
	int mask = s->bitmask ^ 0xff;
	code &= mask;
	if( pat == code ) {
		return 0;	// Matching OK!
	}
	return -1;
}

/** *********************************************************************************
 *	命令コード code がマッチするOPCODEをopcode_init_tab[]から探す. 
 ************************************************************************************
 *	見つかったら、それを *table に丸っとコピーする.
 *	見つからなかったら、UNDEFINED_OPCODEを *table に丸っとコピーする.
 */
void make_table(OPCODE *table,int code)
{
	OPCODE *s = opcode_init_tab;
	while(s->mnemonic) {
		if(match_table(s,code)==0) {
			*table = *s;
			return;
		}
		s++;
	}
	*table = UNDEFINED_OPCODE;
}

/** *********************************************************************************
 *	命令コード code がマッチするOPCODEをopcode_init_tab[]から探す. 
 ************************************************************************************
 *	見つかったら、それを *table に丸っとコピーする.
 *	見つからなかったら、UNDEFINED_OPCODEを *table に丸っとコピーする.
 */
void dump_table()
{
	OPCODE *s = opcode_init_tab;
	while(s->mnemonic) {
//		printf("%s %x %x\n",s->mnemonic,s->pattern,s->bitmask);
		printf("	db(0x%02x);\n",s->pattern);
		printf("	 db(0x%02x);\n",0xff ^ s->bitmask);
		printf("	 db(\"%-4s\");\n",s->mnemonic);
		s++;
	}
	exit(0);
}

/** *********************************************************************************
 *	opcode_init_tab[]の情報をもとに、早引き表の code_table[256] を構築する.
 ************************************************************************************
 */
void init_table()
{
	OPCODE *table = code_table;		// 構築したい表 code_table[256]; の先頭アドレス.
	int code;
	for(code=0;code<256;code++,table++) {
		make_table(table,code);
	}
}

extern char  memreport_buf[];
/** *********************************************************************************
 *	8bitの命令を１つフェッチして実行する.
 ************************************************************************************
 *  SC/MP-IIの場合のみ、reg.pc はフェッチ直前にインクリメントする.
 *  すなわち、最後に実行した命令のオペランドバイトを指したままである.
 */
inline int	execute_pc()
{
	int rc=0;
	char buf[256];
	
	reg.pc_bak = reg.pc;
	memreport_buf[0]=0;
	
	int opcode = memory[ ++reg.pc ];
	OPCODE *table = &code_table[ opcode ];
	if(opcode >= 0x80) {
		opcode2 = memory[ ++reg.pc ];
	}
	rc = table->emufunc(opcode,table);	//命令実行.
	if(opt_q) {
		return rc;
	}

	disasm(buf,opcode,table);
	printf("%s",buf);
	return rc;
	
//	if(rc) return rc;
//	return LED_draw(reg.pc,led_trace);

}

//	JOY入力.
int	 JOY_input()
{
	return 0;
}

int	load_binary(char *fname,int addr,int size)
{
	Ropen(fname);
	int rc = Read(memory+addr,size);
	Rclose();
	
	(void)rc;
	return 0;
}

#if 0
static struct termios saved_term, term;
static volatile unsigned done;
static void term_init()
{
	if (tcgetattr(0, &term) == 0) {
		saved_term = term;
		atexit(exit_cleanup);
		signal(SIGINT, cleanup);
		signal(SIGQUIT, cleanup);
		signal(SIGPIPE, cleanup);
		term.c_lflag &= ~(ICANON | ECHO);
		term.c_cc[VMIN] = 0;
		term.c_cc[VTIME] = 1;
		term.c_cc[VINTR] = 0;
		term.c_cc[VSUSP] = 0;
		term.c_cc[VSTOP] = 0;
		tcsetattr(0, TCSADRAIN, &term);
	}
}

static void cleanup(int sig)
{
	tcsetattr(0, TCSADRAIN, &saved_term);
	done = 1;
}

static void exit_cleanup(void)
{
	tcsetattr(0, TCSADRAIN, &saved_term);
}
#endif


/** *********************************************************************************
 *	メインルーチン.
 ************************************************************************************
 */
int main(int argc,char **argv)
{
//	int maxstep=0;
	int rc;

	Getopt(argc,argv,"");
	if(IsOpt('q')) {
		opt_q   = 1;	// Quiet RUN
		ea_dump = 0;
	}
	if(IsOpt('t')) {
		opt_t   = 1;	// Trace Log
	}
	
	init_table();
	if(IsOpt('D')) {
		dump_table();
	}
	
	
//	term_init();

//	gr_init(SCREEN_W,SCREEN_H,32,0);

//	load_binary(argv[1],0xd000,0x3000);
	load_binary(argv[1],0,0x3000);

	reg.pc = 0;
	reg.sr = 0x20;

	
//	reg.pc = 0xd000;
	get_cputime();

	while(1) {
		rc = execute_pc();
		if(rc) break;
		icount++;
#if 0
		// for debug.
		if ( cpu->pc == 0x00d5 ) { dumpreg(cpu);}
		if ( cpu->pc == 0x03c0 ) { dumpreg(cpu);}
		if(( cpu->pc >= 0x0090 ) &&( cpu->pc <= 0x009f ) ){ dumpreg(cpu);}
		if ( cpu->pc == 0x0046 ) { dumpreg(cpu); exit(1);}
#endif		

#ifdef  MAX_ICOUNT
		if(icount >= MAX_ICOUNT) break;
#endif
	}
//	gr_close();

	int64_t t = get_cputime();
	print_vcount(t);
	return 0;
}


/** *********************************************************************************
 *
 ************************************************************************************
 */
