LLVMCONFIGFLAGS=`llvm-config --cxxflags --ldflags --system-libs --libs core`

COMMON=ast.c lex.yy.c y.tab.c parse_utils.c environment.c
COMMON_OBJS=$(COMMON:%.c=%.o)

all: llvm

interpreter.o $(COMMON_OBJS): interpreter.c $(COMMON)
	gcc -c $^ # -o $@

compiler: compiler.c $(COMMON)
	gcc $^ -o $@

y.tab.c y.tab.h: parser.y
	yacc -d $<

lex.yy.c: lexer.l y.tab.h
	lex $<

llvm: llvm.cpp interpreter.o $(COMMON_OBJS)
	g++ $(LLVMCONFIGFLAGS) $^ -o $@ 

.PHONY: clean
clean:
	rm *.o
	rm y.tab.c
	rm y.tab.h
	rm lex.yy.c
	rm interpreter
