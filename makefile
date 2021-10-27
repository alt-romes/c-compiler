all:
	yacc -d parser.y
	lex lexer.l
	gcc lex.yy.c y.tab.c ast.c main.c -o prog

clean:
	rm y.tab.c
	rm y.tab.h
	rm lex.yy.c
	rm prog
