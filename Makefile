#################################################################
#	コンパイラは MinGw gccを使用します。
#	試してはいませんがcygwinでも大丈夫なはずです。
#	(cygwinでは、コンパイルオプション -mno-cygwinを付けます)
#	DEBUGの時には、-gをつけ、-sを外します。
#################################################################
# REMOVE DEFAULT MAKE RULE
MAKEFLAGS = -r

.SUFFIXES:

.SUFFIXES:	.c .o

.PHONY: all0 moni2 run asmpp scmp2

#============================
# DOSかどうかチェック.
 ifdef ComSpec
MSDOS=1
 endif

 ifdef COMSPEC
MSDOS=1
 endif
#============================

 ifdef MSDOS
DOSFLAGS = -D_MSDOS_
EXE_SUFFIX = .exe
WIN32LIB= -lkernel32 -luser32 -lgdi32 -lsetupapi 
 else
DOSFLAGS = -D_LINUX_
EXE_SUFFIX = .exe
WIN32LIB= 
 endif

CFLAGS	= $(DOSFLAGS) $(CDEFS) -O2 -Wall
LIBS	=

TARGET  = scmp2.exe
#
#
#

CC = gcc
RC = windres

.c.o:
	$(CC) $(CFLAGS) -c $<
#
#
files	= main.o opcode.o disasm.o debug.o linux.o
#
all0:	moni2
#
#
scmp2: $(TARGET)
#
$(TARGET) : $(files)
	$(CC) -s -o $@ $(files) $(WIN32LIB) -lm $(LIBS)
#
moni2:	asmpp
	asmpp2/asmpp2.exe -r -S moni2.m
	asl -L moni2.asm
	p2bin  moni2.p
	cat    moni2.lst
#
run: moni2 scmp2
	./$(TARGET) -q moni2.bin
#
trace: moni2 scmp2
	./$(TARGET) moni2.bin
#
# Test (SC/MP-II)
#
#test:
#	./$(TARGET) -q NIBL.bin
#
asmpp:
	make -C asmpp2
#
clean:
	-rm *.o
	-rm *.asm
	-rm *.lst
	-rm *.p
	-rm *~
	-rm $(TARGET)
#
#
