CC=clang
CFLAGS=`llvm-config --cflags`
CXX=clang++
CXXFLAGS=`llvm-config --cxxflags`
LD=clang++
LDFLAGS=`llvm-config --ldflags --system-libs --libs core`

COMMON=lex.yy.c y.tab.c ast.c parse_utils.c environment.c dcpuIR.c
COMMON_OBJS=$(COMMON:%.c=%.o)

all: compiler interpreter

compiler: compiler.o llvm.o $(COMMON_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

interpreter: interpreter.o $(COMMON_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

y.tab.c y.tab.h: parser.y
	yacc -d $<

lex.yy.c: lexer.l y.tab.h
	lex $<

.PHONY: clean
clean:
	rm *.o
	rm y.tab.c
	rm y.tab.h
	rm lex.yy.c
	rm compiler
	rm interpreter

# In fact, these are close to the default rules, no need to write them, the default rules specificy the same
# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@ 

