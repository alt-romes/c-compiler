COMMON=ast.c lex.yy.c y.tab.c parse_utils.c

all: interpreter

interpreter: interpreter.c $(COMMON)
	gcc $^ -o $@

compiler: compiler.c $(COMMON)
	gcc $^ -o $@

y.tab.c y.tab.h: parser.y
	yacc -d $<

lex.yy.c: lexer.l y.tab.h
	lex $<


.PHONY: clean
clean:
	rm y.tab.c
	rm y.tab.h
	rm lex.yy.c
	rm interpreter
