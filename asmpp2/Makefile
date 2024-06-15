#################################################################
#	コンパイラは MinGw gccを使用します。
#################################################################
# REMOVE DEFAULT MAKE RULE
MAKEFLAGS = -r

.SUFFIXES:

.SUFFIXES:	.c .o


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
EXE = .exe
WIN32LIB= -lkernel32 -luser32 -lgdi32 -lsetupapi 
 else
DOSFLAGS = -D_LINUX_
EXE =
WIN32LIB= 
 endif

#CFLAGS	= -g $(DOSFLAGS) $(CDEFS) -O2 -Wall
CFLAGS	= -s $(DOSFLAGS) $(CDEFS) -O2 -Wall
CFLAGS	+= -Wno-multichar
LIBS	=

TARGET  = asmpp2.exe
#
#
#

CC = gcc
RC = windres

.c.o:
	$(CC) $(CFLAGS) -c $<
#
#
files	= asmpp2.o
#files	= main.o opcode.o disasm.o gr.o led.o debug.o linux.o
#
#
#
#
$(TARGET) : $(files)
	$(CC) $(CFLAGS) -o $@ $(files) $(WIN32LIB) -lm $(LIBS)
#
#
test:	$(TARGET)
	./asmpp2.exe -r -S moni2.m
	asl -L moni2.asm
	cat moni2.lst

#
test2:	$(TARGET)
	./asmpp2.exe -S infile.m
	asl -L infile.asm
	cat infile.lst
#
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
