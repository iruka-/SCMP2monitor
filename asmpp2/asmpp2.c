/*
 *  =====================================
 *  SC/MP-II Asmpp2 (Asm PreProcessor/2)
 *  =====================================
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#ifndef	_LINUX_
#include <dos.h>
#endif


#define MAXLEV  16
#define MAXFOR  4096
#define MAXLINE 256
#define INCLUDE_NESTS 8
#define _IF_	1
#define _ELSE_	2
#define _CASE_  3
#define _WHILE_ 4
#define _DO_	5
#define _FOR_	6
#define _SWITCH_ 7

#define EOL  	0
#define SEMI    ';'
#define COLON   ':'
#define LABELC	'::'
#define PUBLAB	':|'
#define CALLER  '()'
#define ID      'a'
#define IND     'i'
#define NUM     '0'
#define LITERAL 'l'
#define PLUS    '+'
#define MINUS   '-'
#define DIV	    '/'
#define MUL	    '*'
#define ASSIGN  '='
#define AMPER   '&'
#define OR      '|'
#define XOR	    '^'
#define NOT	    '!'
#define EQUAL   '=='
#define NEQUAL  '!='
#define GREAT   '>'
#define GEQUAL  '>='
#define LEQUAL  '<='
#define LESS    '<'
#define SHR	    '>>'
#define SHL	    '<<'
#define XCHG    '<>'
#define ASPLUS  '+='
#define ASMINUS '-='
#define ASCPLUS '+<'
#define ASCMINUS '-<'
#define ASDIV   '/='
#define ASMUL   '*='
#define ASSHR	'>|'
#define ASSHL	'<|'
#define IPLUS   '++'
#define IMINUS  '--'
#define ASAMPER '&='
#define ASOR    '|='
#define ASXOR	'^='
#define REMLIN	'//'
#define REMSTART '/*'
#define PTROP2   '*p'

#define CONDFLAGS 12

#define MAXOBJ  32
#define MAXNAME 256
#define NON   	0
#define EXE   	1
#define COM   	2
#define BASE  	0
#define MASM  	1
#define LINK  	2
#define EXE2BIN 3
#define delfile(fname) if ( midfile == 0 ) unlink(fname)
#define OP0(inst) {noop (inst) 	   ; 			return(1);}
#define OP1(inst) {oneop(inst,dst) ;			return(1);}
#define OP2(inst,opi) {c=getstoken(src);LdaImm(inst,opi,src);return(1);}

// ターゲットがAreg決め打ち.
#define OP2A(inst) {c=getstoken(src);oneopA(inst,dst);return(1);}   // SR SL.

#define ret(i)    {id=i;goto gret;}
#define	spskip(p) {while((*p==' ')||(*p=='\t')) p++;}

static	char *srcname;
static	char *asmname;
static  char *brklab[]= {"??","??","??","ca","ew","od","of","sw"};

static int getln(void);
#define	ZZ	printf("%s:%d: ZZ\n",__FILE__,__LINE__);

#define USE_SYSCALL  1  // XPPC P3の後ろにインラインで呼び出しアドレスを置く.


enum jnam {
	JZ ,JNZ,
	JP ,JM ,
	JAE,JB ,
	JA ,JBE,
	BRA,BRN,
	JMP,JMN,
};

static char *jcontab[CONDFLAGS]= {
	"jz" ,"jnz",
	"jp" ,"_jm" ,
	"jp" ,"_jm" ,
	"_ja","_jbe" ,
	"jmp","_brn",
	"jmp","_brn",
};

static char *flagtab[CONDFLAGS]= {
	"z" ,"nz",
	"p" ,"m" ,
	"p" ,"m" ,
	"a" ,"be",
	"1"  ,"0",
	"true","false",
};

static 	char lnbuf[MAXLINE];
static  FILE *ifp,*ofp;
static  FILE *ifpbak[INCLUDE_NESTS];
static  int   includef=0;
static  int  tokenid;
static  char token[MAXLINE];
static 	char *lp;
static 	int eoflg;
static  int lpos;
static 	int synerr;
static char obuf2[1024]="";
static char obuf[1024]="";
static char *obufp;
static int  ifstackp   =0;
static int  ifstack  [MAXLEV];
static int  ifstackid[MAXLEV];
static int  ifstackfor[MAXLEV];
static int  ifcount    =0;
static int  toplabel;
static int  topmark;

//static char iflabel[]="__if000";
static char ellabel[]="__el000";
static char filabel[]="__fi000";
static char whlabel[]="__wh000";
static char hwlabel[]="__ew000";
static char dolabel[]="__do000";
static char odlabel[]="__od000";
static char swlabel[]="__sw000";
static char calabel[]="__ca000";
static char folabel[]="__fo000";
static char oflabel[]="__of000";
static char  llabel[]="__??000";

static char switchvar[64]="";
static char formem[MAXFOR];
static char *formema[32];
static int  formemap;
static char *formemp;
static int  formemf;
static int  linenum;
static char nambuf[64];
static char numbuf[32];

static int  dflag = 0 ;
static int  rflag = 0 ;
static int  lflag = 0 ;
static int  tflag = 1 ;

static	char pname[MAXNAME];
static  char aname[MAXNAME];
static	char oname[MAXOBJ][MAXNAME];
static  char ename[MAXNAME];
static  char cname[MAXNAME];
static	int	objcnt = 0;
static	int	errcnt = 0;

static	int	asmode = 0; // アセンブラにそのままパス.
static	int	ifzero = 1; // #if 0 開始なら0


/** *********************************************************************************
 *
 ************************************************************************************
 */

int		str_cmpi(char *t,char *s);
void	usage(void);
int		main( int argc,char *argv[]);
int		as(void);
int		cc(void);
int		as(void);
int		lk(void);
int		child( char *path );
void	renext(char *s,char *ext);
void	delext(char *s);
enum	TYPEXT ckext(char *s);
void	prologue(void);
void	epilogue(void);
void	mx(void);
int		asmpp3( char *infile, char *outfile );
void	exprb(void);
void	expr1(void);
void	asmout(char *t,char *s);
int		expr(void);
int		isreg(char *s);
int		iszero(char *s);
int		conditional(int f);
void	twoopImm(char *op,char *dst,char *src);
void	twoop(char *op,char *dst,char *src);
void	oneop(char *op,char *dst);
void	oneopA(char *op,char *dst);
void	noop(char *op);
void	outlab(char *s);
void	underflow(void);
void	makelabel(char *s);
void	makellabel(char *s);
void	genlabel(char *s,int id);
void	stackdrop(void);
void	errsyntax(char *s1,char *s2);
void	crx(void);
void	cr(void);
void	tab(void);
void	pop_for_phrase(void);
void	push_for_phrase(char *s);
void	outs(char *s);
void	outc(int c);
void	outrem(char *s);
int		getstoken(char *s);
int		getstoken2(char *s);
int		getptoken(char *s);
int		getnptoken(char *s);
int		getbtoken(char *s);
int		peektoken(char *s);
int		gettoken(char *s);
int		remskips(int f);
void	outasm(void);
int		getid(char *s);
static	int getln(void);
int		getlnx(void);
void	seq(char *s);
int		nump(int *n);
int		isnum(int c);
int		isal(int c);
int		isan(int c);

int	isreg(char *s);
int	is_areg(char *s);
int	is_ereg(char *s);
int	is_zero(char *s);
int	is_ptr(char *s);
void LdaImm(char *op,char *opi,char *src);
void LeaImm(char *op,char *opi,char *dst,char *src);

/** *********************************************************************************
 *
 ************************************************************************************
 */
int	str_cmpi(char *t,char *s)
{
	while(*t) {
		if(tolower(*t)!=tolower(*s)) return 1;
		t++;
		s++;
	}
	if(*s) return 1;
	return 0;
}

int	str_chki(char *t,char *s)
{
	while(*s) {
		if(tolower(*t)!=tolower(*s)) return 1;
		t++;
		s++;
	}
	return 0;
}

enum TYPEXT {
	fNULL	= 0,
	fBASE	= 1,
	fASM	= 2,
	fOBJ	= 3,
	fEXE	= 4,
	fCOM	= 5,
	fELSE	= 6,
};
static char *extnam[6] = {
	"",
	".m",
	".asm",
	".obj",
	".exe",
	".com",
};
static int  pro_model= 0;
static int  pro_exec = MASM;
static int  midfile=0;

void usage(void)
{
	printf(
	    "Syntax is: Asmpp3 [ options ] file[s]\n"
	    "	-S	generate assembly\n"
	    "	-r	Insert remarks\n"
	    "	-v	View output\n"
	    "	-t	noTab\n"
	    "	-l	Linenumber\n"
	);
	exit( 1 );
}


int	main( int argc,char *argv[])
{
	int  i;
	printf("*** Asmpp3 SC/MP-III Asm PreProcessor 3 ***\n");
	ename[0] = cname[0] = '\0';
	while (1) {
		if ( argc < 2 ) break;
		if ( argv[1][0] != '-' ) break;
		switch ( toupper( argv[1][1] ) ) {
		case 'S' :
			pro_model = NON;
			pro_exec = BASE;
			break;
		case 'V' :
			dflag^=1;
			break;
		case 'R' :
			rflag^=1;
			lflag=0;
			break;
		case 'L' :
			lflag^=1;
			break;
		case 'T' :
			tflag^=1;
			break;
		default:
			usage();
		}
		argc--;
		argv++;
	}
	if ( argc < 2 ) usage();
	if ( ename[0] == '\0' ) strcpy( ename, argv[1] );
	if ( cname[0] == '\0' ) strcpy( cname, argv[1] );
	for ( i = 1; i < argc; i++ ) {
		strcpy( pname, argv[i] );
		switch ( ckext( pname ) ) {
		case fASM:
		case fOBJ:
			if ( pro_exec >= MASM ) {
				strcpy( aname, pname );
				errcnt += as();
			}
			break;
		case fNULL:
			strcat( pname, extnam[fBASE] );
		default:
			if ( cc() != 0 ) {
				errcnt++;
				break;
			}
			if ( pro_exec >= MASM ) {
				if ( as() == 0 ) {
					delfile( aname );
				} else {
					errcnt++;
				}
				break;
			}
		}
	}
	exit( errcnt );
}
int as(void)
{
	int errlvl = 0;
	static int objover = 0;
	if (objcnt >= MAXOBJ ) {
		objover++;
		printf( "Too many .obj files.(%d)\n", objcnt + objover );
		return( 1 );
	}
	strcpy( oname[objcnt], aname );
	renext( oname[objcnt], ".obj" );
	objcnt++;
	return( errlvl );
}


int cc(void)
{
	int errlvl = 0;
	printf( "%s\n", pname );
	if ( dflag == 0 ) {
		strcpy( aname, pname );
		renext( aname, extnam[fASM] );
	} else {
		strcpy( aname, "con" );
	}
	if( strcmp( pname, aname ) == 0 ) {
		printf( "Both file are same.\n" );
		errlvl = 1;
	} else {
		errlvl = ( asmpp3( pname, aname ) != 0 );
	}
	return( errlvl );
}

void renext(char *s,char *ext)
{
	delext( s );
	strcat( s, ext );
}
void delext(char *s)
{
	char *p;
	p = NULL;
	while( *s != '\0' ) {
		if ( *s == '.' ) p = s;
		s++;
	}
	if( ( p != NULL ) && ( ( s - p ) < 5 ) ) {
		*p = '\0';
	}
}
enum TYPEXT ckext(char *s)
{
	char *p;
	enum TYPEXT i;
	p = NULL;
	while( *s != '\0' ) {
		if ( *s == '.' ) p = s;
		s++;
	}
	if ( p == NULL ) return( fNULL );
	for ( i = fNULL; i < fELSE; i++ ) {
		if ( str_cmpi( p, extnam[i] ) == 0 ) break;
	}
	return( i );
}
void prologue(void)
{
	obufp=obuf;
//	outs("\r\n");
}
void epilogue(void)
{
//	outs("\r\n");
}
void mx(void)
{
	while(!eoflg) expr();
}
int asmpp3( char *infile, char *outfile )
{
	srcname = infile;
	asmname = outfile;
	eoflg = synerr = 0;
	tokenid = -1;
	token[0] = '\0';
	formemp = formem;
	linenum = formemf = formemap= 0;
	ifp = fopen( srcname, "rb" );
	if(includef>=INCLUDE_NESTS) {
		printf( "Asmpp3:include nesting over (%d)\n",includef);
		exit( 2 );
	}
	if ( ifp == NULL ) {
		printf( "cannot open '%s'.\n", srcname );
		return( 2 );
	}
	ofp = fopen( asmname,"wb");
	if( ofp == NULL ) {
		printf( "cannot open '%s'.\n", asmname );
		return( 2 );
	}
	strcpy( nambuf, ";~" );
	strcat( nambuf, srcname );
	strcat( nambuf, "(" );
	prologue();
	mx();
	epilogue();
	fclose( ifp );
	fclose( ofp );
	if ( synerr ) {
		printf( "\n %d Error(s)\n" , synerr );
		return( 1 );
	}
	return( 0 );
}

void exprb(void)
{
	static char buf[MAXLINE];
	if(peektoken(buf)=='{') {
		gettoken(buf);
		while(!eoflg) {
			if(peektoken(buf)=='}') {
				gettoken(buf);
				return;
			}
			expr();
		}
	} else expr1();
}
void expr1(void)
{
	static char buf[MAXLINE];
	while(!eoflg) {
		if(expr()) {
			if(eoflg)return;
			if(peektoken(buf)==';') expr();
			return;
		}
	}
}
void asmout(char *t,char *s)
{
	outs(t);
	outs(s);
	crx();
	while(*lp)lp++;
}
//
//  式評価.
//
int	expr(void)
{
	int c;
	int jcond;
	char dst[MAXLINE],src[MAXLINE];
	if(lp==NULL) {
		if(getln()==0) return(0);
	}
	spskip(lp);
	if(*lp==0) {
		if(getln()==0) return(0);
		spskip(lp);
	}
	c=peektoken(dst) ;
	c=getstoken(dst) ;
	if (c==';') return(1);
	if((c==ID)||(c==PTROP2)) {
		if(strcmp("goto",dst)==0) {
			c=getstoken(src);
			oneop("jmp",src);
			return(1);
		}
		if(strcmp("return",dst)==0) {
			strcpy(dst,"ret");
		}
		if(strcmp("if",dst)==0) {
			seq("(");
			jcond=conditional(0);
			seq(")");
			c=peektoken(src);
			if(strcmp("break",src)==0) {
				c=gettoken(src);
				makellabel(llabel);
				jcond ^=1;
				oneop(jcontab[jcond],llabel);
				return(1);
			}
			if(strcmp("goto",src)==0) {
				c=gettoken(src);
				c=getstoken(src);
				jcond ^=1;
//printf(".jcond=%d ellabel=%s %s\n",jcond,ellabel,jcontab[jcond]);
				oneop(jcontab[jcond],src);
				return(1);
			}
			genlabel(ellabel,_IF_);
			oneop(jcontab[jcond],ellabel);
			exprb();
			if(peektoken(dst)==ID) {
				if(strcmp(dst,"else")==0) {
					gettoken(dst) ;
					makelabel(filabel);
					oneop("jmp",filabel);
					makelabel(ellabel);
					outlab(ellabel);
					ifstackid[ifstackp]=_ELSE_;
					exprb();
					makelabel(filabel);
					outlab(filabel);
					stackdrop();
					return(1);
				}
			}
			makelabel(ellabel);
			outlab(ellabel);
			stackdrop();
			return(1);
		}
		if(strcmp("while",dst)==0) {
			genlabel(whlabel,_WHILE_);
			outlab(whlabel);
			seq("(");
			jcond=conditional(0);
			seq(")");
			makelabel(hwlabel);
			if(jcond!=BRN) oneop(jcontab[jcond],hwlabel);
			exprb();
			makelabel(whlabel);
			oneop("jmp",whlabel);
			makelabel(hwlabel);
			outlab(hwlabel);
			stackdrop();
			return(1);
		}
		if(strcmp("for",dst)==0) {
			seq("(");
			exprb();
			genlabel(folabel,_FOR_);
			outlab(folabel);
			jcond=conditional(0);
			seq(";");
			makelabel(oflabel);
			if(jcond!=BRN) oneop(jcontab[jcond],oflabel);
			formemf=1;
			exprb();
			formemf=0;
			seq(")");
			exprb();
			pop_for_phrase();
			makelabel(folabel);
			oneop("jmp",folabel);
			makelabel(oflabel);
			outlab(oflabel);
			stackdrop();
			return(1);
		}
		if(strcmp("do",dst)==0) {
			genlabel(dolabel,_DO_);
			outlab(dolabel);
			exprb();
			seq("while");
			seq("(");
			jcond=conditional(1);
			seq(")");
			makelabel(dolabel);
			jcond^=1;
			if(jcond!=BRN) oneop(jcontab[jcond],dolabel);
			makelabel(odlabel);
			outlab(odlabel);
			stackdrop();
			return(1);
		}
		if(strcmp("switch",dst)==0) {
			genlabel(calabel,_SWITCH_);
			genlabel(calabel,_CASE_);
			seq("(");
			getstoken(switchvar);
			seq(")");
			exprb();
			if(ifstackid[ifstackp]==_CASE_) {
				makelabel(calabel);
				outlab(calabel);
				stackdrop();
			}
			makelabel(swlabel);
			outlab(swlabel);
			stackdrop();
			return(1);
		}
		if(strcmp("case",dst)==0) {
			makelabel(calabel);
			outlab(calabel);
			stackdrop();
			genlabel(calabel,_CASE_);
			c=getptoken(src);
			seq(":");
			twoop("cp",switchvar,src);
			oneop("bnz",calabel);

			return(1);
		}
		if(strcmp("default",dst)==0) {
			makelabel(calabel);
			outlab(calabel);
			stackdrop();
			seq(":");
			return(1);
		}
		if(strcmp("break",dst)==0) {
			makellabel(llabel);
			oneop("jmp",llabel);
			return(1);
		}
		spskip(lp);
		if(*lp==0) {
			noop(dst);
			return(1);
		}
		
		c=getstoken2(src);
		//
		//  代入文 DST = SRC
		//
//printf("1) c=0x%x %s\n",c,src);
		if(c==ASSIGN) {
			c=getstoken(src);
			// 代入先が A reg.
			if(is_areg(dst)) {
			  // ソースもレジスタ(reg,reg)
			  if(is_ereg(src)) {
				noop("lde");
				return(1);
			  }else{
				LdaImm("ld","ldi",src);
				return(1);
			  }
			}
			// ソースが A レジスタ(STORE)
			if(is_areg(src)) {
				if(is_ereg(dst)) {
					// 代入先が E reg.
					// E=A
					noop("xae");
					noop("lde");
					return(1);
				}
				// 代入先が メモリー( STore )
				LdaImm("st","st",dst);
				return(1);
			}
			// 代入先が PTR reg.
			if(is_ptr(dst)) {
				LeaImm("ldptr","lea",dst,src);
				return(1);
			}else{
				// 代入先が メモリー( STore )で、ソースがImmを仮定.
				LdaImm("ld","ldi",src);
				LdaImm("st","st",dst);
				return(1);
			}

			errsyntax("Assign Rule Error",src);
			return(0);
		} // 代入ここまで.

		if(c==XCHG) {
			c=getstoken(src);
			// 代入先がレジスタ.
			if(isreg(dst)) {
			  // ソースもレジスタ(reg,reg)
			  if(is_areg(src) && is_ereg(dst)) {
				  noop("xae");
				  return(1);
			  }
			  if(is_ereg(src) && is_areg(dst)) {
				  noop("xae");
				  return(1);
			  }
			  if(isreg(src)) {
				twoop("xch",dst,src);
				return(1);
			  }
			}
		}
		if((c==ID)||(c==NUM)||(c==LITERAL)||(c=='<')) {

			if ( toplabel==0) tab();
			else toplabel=0;

			outs(dst);
			tab();
			outs(src);

			while(1) {
				if(*lp==' ') outs(" ");
				if(*lp=='\t') outs("\t");
				spskip(lp);
				if(*lp==0) {
					cr();
					return(1);
				}
				c=gettoken(src);
				if((c==0)||(c==EOL)||(c==SEMI)) {
					cr();
					return(1);
				}
				outs(src);
			}
		}
		switch(c) {
		case PUBLAB:
			oneop("public",dst);
		case LABELC:
			outlab(dst);
			toplabel=0;
			return(1);
		case SEMI:
			lp--;
			OP0( dst )
		case ASPLUS:
			noop("ccl");   // CY=0
			OP2("add","adi")
		case ASMINUS:
			noop("scl");   // CY=1
			OP2("cad","cai")
		case ASCPLUS:
			OP2("adc","aci")
//		case ASCMINUS:
//			OP2("sbc",)
		case ASAMPER:
			OP2("and","ani")
		case ASOR:
			OP2("or" ,"ori")
		case ASXOR:
			OP2("xor","xri")
		case IPLUS:
			OP1("inc")
		case IMINUS:
			OP1("dec")
		case CALLER:	/*OP1("call")*/
			if(toplabel) {
				//oneop("public",dst);
				if(gettoken(src)!='{') {
					errsyntax("Missing func","{");
				}
				outlab(dst);

				while(!eoflg) {
					c=peektoken(src);
					if(c=='}') break;
					expr();
				}
				gettoken(src);
				noop("ret");
				return(1);
			} else {
#if USE_SYSCALL    // XPPC P3の後ろにインラインで呼び出しアドレスを置く.
				oneop("call",dst) ;
#else
				oneop("jsr",dst) ;
#endif
				return(1);
			}
		case SHR:
			seq("=");
			OP2A("sr")
		case SHL:
			seq("=");
			OP2A("sl")
		case ASMUL:
			c=getstoken(src);
			oneop("mul",src);
			return(1);
		case ASDIV:
			c=getstoken(src);
			oneop("div",src);
			return(1);
		default:
			errsyntax("Illegal Operator",src);
			return(0);
		}
	} else {
		switch(c) {
		case 0:
			return(0);
		case '.': {
			asmout("\t.",lp);
			return(0);
		}
		default:
			//printf("c=0x%x\n",c);
			errsyntax("Missing Identifier",dst);
			return(0);
		}
	}
}
int	isreg(char *s)
{
	if(str_cmpi(s,"a")==0) return 1;
	if(str_cmpi(s,"e")==0) return 1;
	if(str_cmpi(s,"s")==0) return 1;
	if(str_cmpi(s,"ea")==0) return 1;
	if(str_cmpi(s,"p1")==0) return 1;
	if(str_cmpi(s,"p2")==0) return 1;
	if(str_cmpi(s,"p3")==0) return 1;
	if(str_cmpi(s,"pc")==0) return 1;
	return 0;
}
int	is_areg(char *s)
{
	if(str_cmpi(s,"a")==0) return 1;
	return 0;
}
int	is_ereg(char *s)
{
	if(str_cmpi(s,"e")==0) return 1;
	return 0;
}
int	is_zero(char *s)
{
	if(str_cmpi(s,"0")==0) return 1;
	return 0;
}
int	is_ptr(char *s)
{
	if(str_cmpi(s,"p1")==0) return 1;
	if(str_cmpi(s,"p2")==0) return 1;
	if(str_cmpi(s,"p3")==0) return 1;
	return 0;
}
int	iszero(char *s)
{
	if( (s[0]=='0')&&(s[1]==0) ) return(1);
	return(0);
}

//  ================================================
//  if (   )  内の条件を処理
//  ================================================
//  do { } while( COND. ) のときのみf=1;
int	conditional(int f)
{
	int c1,c2;
	int jc,i,revcon;
	char op1[MAXLINE],op2[MAXLINE],op3[MAXLINE];
	revcon=0;
	c1=peektoken(op1);
	if(c1=='{') exprb();
	else if(c1==NOT) {
		c1=gettoken(op1);
		revcon ^=1;
	};

	//
	// いきなり '--'
	//
	c1=peektoken(op1);
	if(c1==IMINUS) {
		gettoken(op1);
		getstoken(op1);
		LdaImm("dld","dld",op1);
		c2=peektoken(op2);
		if( (c2== ')')||(c2== ';') ) {
//			oneop("sub","1");			//
			return(JZ ^revcon);			// JF!
		} else {
			c2=gettoken(op2);
			getstoken(op3);
			switch(c2) {
			case EQUAL :
				jc=JNZ;
				goto _or3;
			case NEQUAL:
				jc=JZ ;
				goto _or3;
			default:
				errsyntax("Illegal compare operator",op2);
				return(BRN^revcon);
			}
_or3:
			if( iszero(op3) ) {
				return(jc^revcon);
			}
			errsyntax("Illegal compare operator 3",op2);
		}
	}
	getstoken(op1);
	c2=peektoken(op2);
	if( (c2 == ')') || (c2 == ';') ) {
		for(i=0; op1[i]; i++) {
			op1[i]=tolower(op1[i]);
		}
		for(i=0; i<CONDFLAGS; i++) {
			if(strcmp(op1,flagtab[i])==0) {
				return(i ^ 1 ^revcon);
			}
		}
		errsyntax("Illegal condition",op1);
		return(BRN^revcon);
	}
//_label2:
	//
	// if( op1 c2 op3 ) { で、 c2 は比較演算子.
	//
	c2=gettoken(op2);
	getstoken(op3);
	switch(c2) {
	case EQUAL :
		jc=JNZ;
		goto _or;
	case NEQUAL:
		jc=JZ ;
		goto _or;
	case GEQUAL:
		jc=JB ;
		goto _cmp;
	case LEQUAL:
		jc=JA ;
		goto _cmp;
	case GREAT :
		jc=JBE;
		goto _cmp;
	case LESS  :
		jc=JAE;
		goto _cmp;
	case AMPER :
		jc=JZ ;
		goto _test;
	default:
		errsyntax("Illegal compare operator 2",op2);
		return(BRN^revcon);
	}
_or:
	//
	//  op1がレジスタ  op3が ゼロ 、つまり JZかJNZ
	//
	if( isreg(op1) && iszero(op3) ) {
		if( is_ereg(op1) ) {
			noop("lde"); // A=E
		}
//		twoop("or",op1,op1);  OR命令要らない.
		return(jc^revcon);
	}
_cmp:
	//
	//  引き算する.
	//
	if( is_ereg(op1) ) {
		noop("lde"); // A=E
		if(!is_zero(op3)) {
			noop("scl");   // CY=1
			LdaImm("cad","cai",op3);
		}
		//	printf("_sub jc=%d revcon=%d\n",jc,revcon);
		return(jc^revcon);
	}
	if(!is_areg(op1) ) {
		LdaImm("ld","ldi",op1);
	}
	if(!is_zero(op3)) {
		noop("scl");   // CY=1
		LdaImm("cad","cai",op3);
	}
//	printf("_sub jc=%d revcon=%d\n",jc,revcon);
	return(jc^revcon);

_test:
	//
	// & でビット演算.
	//
	if( is_ereg(op1) ) {
		noop("lde"); // A=E
		twoopImm("and","a",op3);
		return(jc^revcon);
	}
	twoopImm("and",op1,op3);
	return(jc^revcon);
}
void twoop(char *op,char *dst,char *src)
{
	tab();
	outs(op);
	tab();
	outs(dst),outs(",");
	outs(src);
	cr();
}
int is_ptrRel(char *src)
{
	if(str_chki(src,"p1[")==0) {
		return 1;
	}
	if(str_chki(src,"p2[")==0) {
		return 1;
	}
	if(str_chki(src,"p3[")==0) {
		return 1;
	}
	return 0;
}
char *ptrRel(char *src)
{
	static char retp[256];
	char *s = src+3;
	int len = strlen(s);
	if(s[len-1]!=']') {
		errsyntax("ptrRel error",src);
	}
	s[len-1]=0;
	strcpy(retp,s);
	strcat(retp,",");
	retp[len+0] = src[0];
	retp[len+1] = src[1];
	retp[len+2] = 0;
	return retp;
}

// 廃止予定関数(scmp3用)
void twoopImm(char *op,char *dst,char *src)
{
	tab();
	outs(op);
	tab();
	outs(dst);

	
	//	printf("src=>%s<\n",src);

	int s1 = src[1] | 0x20;
	if((src[0]=='*')&&(s1=='p')) {
		//  ============================================
		// *P2
		// *P2++
		//  を処理する  0,P2 または @1,P2 などを出力する
		//  ============================================
		if(src[3]=='+') {
			src[3]=0;
			if(str_cmpi(dst,"ea")==0) {
				outs(", @2,");outs(src+1);
			}else{
				outs(", @1,");outs(src+1);
			}
		}else{
			outs(", 0,"); outs(src+1);
		}
	}else{
		//  ============================================
		//  ZeroPage、もしくは即値を出力する.
		//  ============================================
		if(is_ptrRel(src)) {
			outs(",");outs(ptrRel(src));
		}else if(isal(*src)) {
			outs(",");outs(src);
		}else{
			if(*src == '#') src++;
			outs(", =");outs(src);
		}
	}
	cr();
}

// LD もしくは LDI dst=Areg
void LdaImm(char *op,char *opi,char *src)
{
	//	printf("src=>%s<\n",src);

	int s1 = src[1] | 0x20;
	if((src[0]=='*')&&(s1=='p')) {
		tab();
		outs(op);
		tab();
		//  ============================================
		// *P2
		// *P2++
		//  を処理する  0,P2 または @1,P2 などを出力する
		//  ============================================
		if(src[3]=='+') {
			src[3]=0;
			outs("@1(");outs(src+1);outs(")");
		}else{
			outs("0("); outs(src+1);outs(")");
		}
	}else{
		//  ============================================
		//  ZeroPage、もしくは即値を出力する.
		//  ============================================
		if(is_ptrRel(src)) {
			// p2[n]
			tab();
			outs(op);
			tab();
			outs(ptrRel(src));
		}else if(isal(*src)) {
			// Symbol だと、Directとみなしたい.
			tab();
			outs(op);
			tab();
			outs(src);
			outs("(P1)");
		}else{
			// # 即値
			if(*src == '#') src++;
			
			tab();
			outs(opi);
			tab();
			outs(src);
		}
	}
	cr();
}

void LeaImm(char *op,char *opi,char *dst,char *src)
{
	int s1 = src[1] | 0x20;
	if((src[0]=='*')&&(s1=='p')) {
		tab();
		outs(op);
		tab();
		//  ============================================
		// *P2
		// *P2++
		//  を処理する  0,P2 または @1,P2 などを出力する
		//  ============================================
		if(src[3]=='+') {
			src[3]=0;
			outs("@1(");outs(src+1);outs(")");
		}else{
			outs("0("); outs(src+1);outs(")");
		}
	}else{
		//  ============================================
		//  ZeroPage、もしくは即値を出力する.
		//  ============================================
		if(is_ptrRel(src)) {
			// p2[n]
			tab();
			outs(op);
			tab();
			outs(ptrRel(src));
		}else if(isal(*src)) {
			// Symbol だと、Directとみなしたい.
			tab();
			outs(op);
			tab();
			outs(src);
			outs("(P1)");
		}else{
			// # 即値
			if(*src == '#') src++;
			
			tab();
			outs(opi);
			tab();
			outs(dst);
			outs(",");
			outs(src);
		}
	}
	cr();
}

void oneop_s(char *op,char *dst)
{
	tab();
	outs(op);
	tab();
	outs(dst);
	cr();
}

void cond_skip(char *op,char *dst,char *op2,char *dst2)
{
	oneop_s(op,dst);
	oneop_s(op2,dst2);
}

void macro_op(char *op,char *dst)
{
	if(strcmp(op,"_jm")==0) {
		cond_skip("jp","$+4","jmp",dst);return;
	}
	if(strcmp(op,"_ja")==0) {
		cond_skip("jz","$+4","jp",dst);return;
	}
	if(strcmp(op,"_jbe")==0) {
		cond_skip("jp","$+4","jz",dst);return;
	}
	if(strcmp(op,"_brn")==0) {
		                    return;
	}
	errsyntax("Internal Macro Error",op);
}

// One OP Hook!!! (Macro)
void oneop(char *op,char *dst)
{
	if(op[0]=='_') {
		macro_op(op,dst);
	}else{
		oneop_s(op,dst);
	}
}
// dst = Areg Only
void oneopA(char *op,char *dst)
{
	if(is_areg(dst)) {
		noop(op);
	}else{
		errsyntax("operand must be Areg",dst);
	}
}



void noop(char *op)
{
	tab();
	outs(op);
	cr();
}

void outlab(char *s)
{
	outs(s);
	outs(":");
	cr();
}

void underflow(void)
{
	errsyntax(" } stack underflow.","");
}
void makelabel(char *s)
{
	sprintf(s+4,"%03d",ifstack[ifstackp]);
}
void makellabel(char *s)
{
	int i;
	i=ifstackp;
	while(i>0) {
		if(ifstackid[i]>=_WHILE_) {
			strcpy(s+2,brklab[ifstackid[i]]);
			sprintf(s+4,"%03d",ifstack[i]);
			return;
		}
		i--;
	}
	errsyntax("Misplaced" ,"break;");
}
void genlabel(char *s,int id)
{
	if(ifstackp >=MAXLEV) {
		errsyntax(" FATAL> ","NESTING LEVEL OVERFLOW(16)");
		exit(9);
	}
	ifstackp++;
	ifstackid[ifstackp]=id;
	ifstack  [ifstackp]=ifcount;
	ifstackfor[ifstackp]=formemap;
//	makelabel(s,ifcount);
	makelabel(s);
	ifcount++;
}
void stackdrop(void)
{
	ifstackp--;
	if(ifstack<0) {
		errsyntax(" FATAL> ","NESTING LEVEL UNDERFLOW");
		exit(9);
	}
}
void errsyntax(char *s1,char *s2)
{
	printf("%s(%d):*** Error %s (%s)***\n",srcname,linenum,s1,s2);
	while(*lp) lp++;
	synerr++;
}
//
//   改行出力
//
void crx(void)
{
	outs("\r\n");
}
//
//   改行出力
//
void cr(void)
{
	if(!lflag) {
		outs("\r\n");
		return;
	}
	if(tflag) {
		while(lpos<32) outs("\t");
	}
	//
	// ;~ ファイル名 行番号
	//
	outs(nambuf);
	sprintf(numbuf,"%d)\r\n",linenum);
	outs(numbuf);
}
void tab(void)
{
	if(tflag) outs("\t");
	else outs(" ");
}
void pop_for_phrase(void)
{
	int i;
	for(i=ifstackfor[ifstackp]; i<formemap; i++) {
		outs(formema[i]);
	}
	formemap=ifstackfor[ifstackp];
	formemp =formema[formemap];
}
void push_for_phrase(char *s)
{
//	int i;
	char *p;
	p=formemp;
	formema[formemap++]=formemp;
	formemp += (strlen(s)+1);
	if(formemp< &formem[MAXFOR] )
		strcpy(p,s);
	else {
		errsyntax(" FATAL> "," 'FOR' STACK OVERFLOW");
		exit(9);
	}
}
void outs(char *s)
{
	while(*s) outc(*s++);
}
void outc(int c)
{
	if(c!='\n') {
		*obufp++ = c;
		if(c=='\t') {
			lpos &= 0xff8;
			lpos +=     8;
		} else lpos++;
		return;
	}
	*obufp++ =c;
	*obufp   =0;
	lpos    =0;
	obufp	 =obuf;
	if(formemf) {
		push_for_phrase(obuf);
		return;
	}
	if(obuf2[0]) fprintf(ofp,"%s",obuf2);
	strcpy(obuf2,obuf);
}
void outrem(char *s)
{
	if(obuf2[0]) fprintf(ofp,"%s",obuf2);
	strcpy(obuf2,s);
}

//セグメント付きトークン.
/*int	getstoken(char *s)
{
	int c;
	c=getnptoken(s);
	return(c);
}*/

//  オフセット付きトークン.
int	getstoken(char *s)
{
	int c,c1,c2;
	c=getnptoken(s);
	c1=*lp;
	c2=lp[1];
	if( c==IND ) return(ID);
	if( (c==ID) && (c1==COLON) ) {
		if( (c2>' ')&&(c2!=COLON) ) {
			while(*s)s++;
			getbtoken(s);
			while(*s)s++;
			getptoken(s);
			c=ID;
		}
	}
	c1=*lp;
	if(( (c==ID) || (c==NUM) ) && (c1=='[') ) {
		while(*s)s++;
		getptoken(s);
		return(ID);
	}
	return(c);
}
//  オフセット付きトークン.
//  （左辺値）
int	getstoken2(char *s)
{
	int c,c1;
	c=getptoken(s);
	if( c==IND ) return(ID);

	c1=*lp;
	if(( (c==ID) || (c==NUM) ) && (c1=='[') ) {
		while(*s)s++;
		getptoken(s);
		return(ID);
	}
	return(c);
}

// ( expr ) を得る  (結果はexprになる)
int	getptoken(char *s)
{
	int c;
	c=getbtoken(s);
	if(c=='(') {
		while(*lp) {
			c=getnptoken(s);
			if(c==')') {
				*s=0;
				return(IND);
			}
			while(*s)s++;
			if((*lp==' ')||(*lp=='\t')) {
				*s++=*lp++;
				*s=0;
			}
		}
		errsyntax(" Missing ",")");
		return(0);
	}
	return(c);
}


//                                     ？(hl) を得る.
//  '(' で始まり ')'で終わる場合を含むトークンを得る.
int	getnptoken(char *s)
{
	int c;
	c=getbtoken(s);
	if(c=='(') {
		while(*lp) {
			while(*s)s++;
			c=getnptoken(s);
			while(*s)s++;
			if(c==')') {
				*s=0;
				return(IND);
			}
			while(*s)s++;
			if((*lp==' ')||(*lp=='\t')) {
				*s++=*lp++;
				*s=0;
			}
		}
		errsyntax(" Missing ",")");
		return(0);
	}
	return(c);
}
//
//  [ ] 付きトークン ===> IND （インダイレクト）　を返す.
//  それ以外 ==> トークンを返す.
//
int	getbtoken(char *s)
{
	int c;
	c=gettoken(s);
	if(c=='[') {
		while(*lp) {
			while(*s)s++;
			c=gettoken(s);
			while(*s)s++;
			if((*lp==' ')||(*lp=='\t')) {
				*s++=*lp++;
				*s=0;
			}
			if(c==']') return(IND);
		}
		errsyntax(" Missing ","]");
		return(0);
	}
	return(c);
}
int	peektoken(char *s)
{
	int id;
	if(tokenid!=-1) {
		strcpy(s,token);
		id=tokenid;
		return(id);
	}
	tokenid=gettoken(token);
	strcpy(s,token);
	return(tokenid);
}
//
//  トークンを切り出す
//  
int	gettoken(char *s)
{
	int c,t,id;
	if(tokenid!=-1) {
		strcpy(s,token);
		id=tokenid;
		tokenid=-1;
		return(id);
	}
GT0:
	c=getid(s);
	while(c==0) {
		if (getln()==0) return(0);
		c=getid(s);
	}
	t=*lp;
	switch (c) {
	case PLUS:
		if(t == '+') ret(IPLUS);
		if(t == '=') ret(ASPLUS);
		if(t == '<') ret(ASCPLUS);
		break;
	case MINUS:
		if(t == '-') ret(IMINUS);
		if(t == '=') ret(ASMINUS);
		if(t == '<') ret(ASCMINUS);
		break;
	case ASSIGN:
		if(t == '=') ret(EQUAL);
		break;
	case AMPER:
		if(t == '=') ret(ASAMPER);
		break;
	case OR:
		if(t == '=') ret(ASOR);
		break;
	case XOR:
		if(t == '=') ret(ASXOR);
		break;
	case GREAT:
		if(t == '=') ret(GEQUAL);
		if(t == '>') ret(SHR);
		break;
	case LESS:
		if(t == '=') ret(LEQUAL);
		if(t == '<') ret(SHL);
		if(t == '>') ret(XCHG);
		break;
	case NOT:
		if(t == '=') ret(NEQUAL);
		break;
	case '(':
		if( t ==')') ret(CALLER);
	case DIV:
		if(t == '/') {
			remskips(REMLIN)  ;
			goto GT0;
		}
		if(t == '*') {
			remskips(REMSTART);
			goto GT0;
		}
		break;
	case COLON:
		if( (t == ' ') || (t == '\t') ||(t == 0) ) {
			return(LABELC);
		}
		if(  t == ':'  ) ret(PUBLAB);
		break;

	case MUL:  // (*)
	  {
		int ptrlen=0;
		int p2;
		if((t=='p')||(t=='P')) {
			int t2=lp[1];
			if((t2=='2')||(t2=='3')) {
				int t3=lp[2];
				int t4=lp[3];
				//ptrid= 2 + (t2-'2');
				if((t3=='+')&&(t4=='+')) {
					ptrlen=4;
				}else{
					ptrlen=2;
				}
				s++;
				for(p2=0;p2<ptrlen;p2++) {*s++=*lp++;}
				*s=0;
				return PTROP2;
			}
		}
	  }
		break;
	case '{':
		if( t =='*') {
			outasm();
			return(0);
		}
	default:
		break;
	}
	return(c);
gret:
	s++;
	*s++=*lp++;
	*s=0;
	return(id);
}

//
//  コメントを読み飛ばす
//  
int remskips(int f)
{
	lp++;
	{
		while(1) {
			if(*lp==0) {
				if (getln()==0) return(0);
				if (f==REMLIN)  return(0);
			}
			if( (*lp=='*') && (lp[1]=='/') ) {
				lp+=2;
				break;
			}
			lp++;
		}
		if(*lp==0) getln();
	}
	return 0;
}
//
//  アセンブラ行出力
//
void outasm(void)
{
	while(1) {
		if(getln()==0) return;
		if( (*lp=='*')&&(lp[1]=='}') ) {
			lp+=2;
			return;
		}
		outs(lp);
		crx();
	}
}
//
//  識別子.
//
int getid(char *s)
{
	char *p,c;
	p=s;
	*p=0;
	spskip(lp);
	c= *lp;
	if(c==0) return(0);
	//
	//  文字列リテラル（クォート文字列の始まり）
	//
	if((c=='\'')||(c=='\"')) {
		while( *lp ) {
			*p++ = *lp++;
			if(*lp==c) {
				lp++;
				break;
			}
		}
		*p++=c;
		*p=0;
		return(LITERAL);
	}
	//
	//  昔の表記 $ で始まる16進.
	//
	if(c=='$') {
		lp++;
		*p++='0';
		*p++='x';
		while( *lp ) {
			if(!isan(*lp)) break;
			*p++ = *lp++;
		}
		*p=0;
		return(NUM);
	}
	//
	//  # による 即値表現 (ラベル値を即値として扱いたいとき)
	//
	if(c=='#') {
		*p++ = *lp++;
		while( *lp ) {
			if(!isan(*lp)) break;
			*p++ = *lp++;
		}
		*p=0;
		return(ID);
	}
	//
	// 英数字でない.
	//
	if(!isan(c)) {
		*p++ = *lp++;
		*p=0 ;
		return(c);
	}
	//
	// 英数字である.
	//
	*p++ = *lp++;
	if(isnum(c) && ( ( (*lp) | 0x20)=='x' ) ) {
		lp++;
		*p++='x';
		while( *lp ) {
			if(!isan(*lp)) break;
			*p++ = *lp++;
		}
		*p=0;
		return(NUM);
	}
	//
	// 英数字で無くなるまで.
	//
	while( *lp ) {
		if(!isan(*lp)) break;
		*p++ = *lp++;
	}
	*p=0;
	if(isal(c))	return(ID);
	else		return(NUM);
}

int include_src(char *s);


static int pre_pro(char *s)
{
	if(str_chki(s,"#if")==0) {
		s=s+strlen("#if");spskip(s);
		//printf("s=>%s<",s);
			  
		sscanf(s,"%d",&ifzero);
		return 0;
	}
	if(str_chki(s,"#endif")==0) {
		ifzero = 1;
		return 0;
	}

	if(str_chki(s,"#include")==0) {
		s=s+strlen("#include");spskip(s);
		include_src(s);
		return 0;
	}

	if(ifzero==0) return 0;
	
	if(str_chki(s,"#asm")==0) {
		asmode=1;return 0;
	}
	if(str_chki(s,"#endasm")==0) {
		asmode=0;return 0;
	}
	asmout("",s+1);
    return 1;
}
//
//  １行入力
//
static int getln(void)
{
	int rc;
	char buf[MAXLINE];

redo_:
	rc=getlnx();
	if(rc==0) return(rc);

	if(*lnbuf=='#') { // 行頭の '#' のみ、プリプロセスに使用する.
		pre_pro(lnbuf);
		goto redo_;
	}

	if(ifzero==0) {
		goto redo_;
	}

	if(asmode==1) {
		asmout("",lnbuf);
		goto redo_;
	}

	
	topmark= *lnbuf;
	toplabel=0;
	if(*lnbuf>='@') toplabel=1;/* 行頭の識別子をラベルとみなす */

	
	// 元ソースをコメント文でasm出力する.
	if(rflag) {
		strcpy(buf,";");
		if(!tflag) {
			sprintf(buf+1,"%d",linenum);
			strcat(buf,":\t\t\t\t");
		}
		strcat(buf,lnbuf);
		strcat(buf,"\r\n");
		outrem(buf);
	}
	
	return(rc);
}
//
//  １行入力
//
int getlnx(void)
{
	int i,c;
	lp=lnbuf;
	for(i=0; i<MAXLINE; i++)lnbuf[i]=0;

	if(eoflg) return(0);
	i=0;
	while((c=getc(ifp))!=EOF) {
		if(c=='\r') c=getc(ifp);
		if(c== EOF) break;
		if(c==0x1a) break;
		if(c=='\n') {
			linenum ++;
			return(1);
		}
		lnbuf[i++]=c;
	}
    if(includef) {
    	fclose(ifp);
    	ifp=ifpbak[--includef];
    	
    	return getlnx();
    }
	eoflg=1;
	return(0);
}

//
//  #include "ソース"
//
int include_src(char *s)
{
	int c;
	if( (*s == '\'') || (*s == '\"') ) {
		s++;
		c=s[strlen(s)-1];
		if( (c =='\'') || (c =='\"') ) {
			s[strlen(s)-1]=0;
		}
	}
	ifpbak[includef++]=ifp;

	ifp = fopen( s, "rb" );
	if ( ifp == NULL ) { 
		printf( "Asmpp3:cannot open include file :'%s'.\n", s );
		return( 2 );
	}
	/*includef=1;*/
//	ugflg=0;
	return 0;
}
//
//  識別子 (s)が来ることを仮定。来ない場合はエラー.
//
void seq(char *s)
{
	char buf[MAXLINE];
	gettoken(buf);
	if(strcmp(s,buf)!=0) {
		errsyntax("Missing ",s);
	}
}
int nump(int *n)
{
	spskip(lp);
	if(isnum(*lp)==0)return(0);
	*n = 0;
	while(isnum(*lp)) {
		*n *= 10;
		*n += (*lp - '0');
		lp++;
	}
	return(1);
}
//
//  数字
//
int isnum(int c)
{
	return((c>='0') && (c<='9'));
}
//
//  英字
//
int isal(int c)
{
	if( (c>='A') && (c<='Z') )return (1);
	if( (c>='a') && (c<='z') )return (1);
	if( (c == '_' ) || (c == '@' ) ) return (1);
	return(0);
}
//
//  英数字.
//
int isan(int c)
{
	if(isnum(c))return (1);
	return(isal(c));
}
