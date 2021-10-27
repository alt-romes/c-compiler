all: main

main: main.c ast.c lex.yy.c y.tab.c
	gcc lex.yy.c y.tab.c ast.c main.c -o main

y.tab.c y.tab.h: parser.y
	yacc -d parser.y

lex.yy.c: y.tab.h lexer.l
	lex lexer.l


.PHONY: clean
clean:
	rm y.tab.c
	rm y.tab.h
	rm lex.yy.c
	rm main
