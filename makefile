CC=clang
CXX=clang++
LD=clang++

BUILD_DIR=./build
SRC_DIR=./src
INCLUDE_DIR=./include

CFLAGS=`llvm-config --cflags`
CXXFLAGS=`llvm-config --cxxflags`
LDFLAGS=`llvm-config --ldflags --system-libs --libs core`
CPPFLAGS=-Wall -I$(INCLUDE_DIR)

HEADERS := $(shell find $(INCLUDE_DIR) -name '*.h')
SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(BUILD_DIR)/lex.yy.o $(BUILD_DIR)/y.tab.o $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)


all: compiler # interpreter # dcpu

$(SRC_DIR)/y.tab.c $(INCLUDE_DIR)/y.tab.h: $(SRC_DIR)/parser.y
	yacc -o $(SRC_DIR)/y.tab.c -d $<
	mv $(SRC_DIR)/y.tab.h $(INCLUDE_DIR)/

$(SRC_DIR)/lex.yy.c: $(SRC_DIR)/lexer.l $(INCLUDE_DIR)/y.tab.h
	lex -o $@ $<

# Build object files from C files in the project src directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build object files from C files in the project root directory
$(BUILD_DIR)/%.o: %.c $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

compiler: $(BUILD_DIR)/compiler.o $(OBJS)
	$(LD) $(CPPFLAGS) $(LDFLAGS) $^ -o $@

interpreter: $(BUILD_DIR)/interpreter.o $(OBJS)
	$(LD) $(CPPFLAGS) $(LDFLAGS) $^ -o $@

.PHONY: clean test
clean:
	rm -f $(BUILD_DIR)/*
	rm -f $(SRC_DIR)/y.tab.c
	rm -f $(INCLUDE_DIR)/y.tab.h
	rm -f $(SRC_DIR)/lex.yy.c
	rm -f compiler
	rm -f interpreter
	rm -f dcpu

test: all
	# ./run-tests.sh interpreter
	./run-tests.sh compiler

