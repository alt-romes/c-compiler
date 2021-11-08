CC=clang
CFLAGS=`llvm-config --cflags`
CXX=clang++
CXXFLAGS=`llvm-config --cxxflags`
LD=clang++
LDFLAGS=`llvm-config --ldflags --system-libs --libs core`

COMMON_SRCS=lex.yy.c y.tab.c ast.c parse_utils.c environment.c dcpuIR.c
COMMON_OBJS=$(COMMON_SRCS:%.c=%.o)

INTERPRETER_SRCS=interpreter.c
INTERPRETER_OBJS=$(INTERPRETER_SRCS:%.c=%.o)

DCPU_SRCS=dcpuCompiler.c dcpuIR.c
DCPU_OBJS=$(DCPU_SRCS:%.c=%.o)

COMPILER_SRCS=compiler.c llvm.c
COMPILER_OBJS=$(COMPILER_SRCS:%.c=%.o)

all: compiler interpreter dcpu

compiler: $(COMPILER_OBJS) $(COMMON_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

interpreter: $(INTERPRETER_OBJS) $(COMMON_OBJS)
	$(LD) $^ -o $@

dcpu: $(DCPU_OBJS) $(COMMON_OBJS)
	$(LD) $^ -o $@

y.tab.c y.tab.h: parser.y
	yacc -d $<

lex.yy.c: lexer.l y.tab.h
	lex $<

.PHONY: clean
clean:
	-rm *.o
	-rm y.tab.c
	-rm y.tab.h
	-rm lex.yy.c
	-rm compiler
	-rm interpreter
	-rm dcpu

# In fact, these are close to the default rules, no need to write them, the default rules specificy the same
# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@ 

