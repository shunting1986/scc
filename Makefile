# setting for gtest
GTEST_ROOT=/media/shunting/disk/compiler/3rdParty/gtest-1.7.0
GTEST_CFLAGS=-isystem $(GTEST_ROOT)/include -g
GTEST_LDFLAGS=$(GTEST_ROOT)/make/gtest-all.o -lpthread

CFLAGS += -Isrc -g $(MCFLAGS) -Wall -Werror -DDEBUG

# NOTE: should not add quote around each path
LIB_SRC_LIST := util/file_reader.c \
	util/util.c \
	util/cbuf.c \
	util/dynarr.c \
	util/malloc.c \
	util/htab.c \
	lex/lexer.c \
	lex/keyword.c \
	lex/token.c \
	parser/parser.c \
	parser/type-parser.c \
	parser/stmt-parser.c \
	parser/expr-parser.c \
	parser/syntree.c \
	parser/syntree-node.c \
	parser/syntree-check.c \
	parser/syntree-visitor.c \
	cgc/cgc.c \
	cgc/cgc_opstr.c \
	cgasm/cgasm.c \
	cgasm/cgasm-func.c \
	cgasm/cgasm-emit.c \
	cgasm/cgasm-symbol.c \
	cgasm/cgasm-expr.c \
	cgasm/cgasm-expr-val.c \
	cgasm/asm-label.c \
	cgasm/handle-op.c \
	cgasm/cgasm-stmt.c \
	symtab/symtab.c 

LIB_SRC_LIST := $(patsubst %,src/%,$(LIB_SRC_LIST))
LIB_SRC_LIST := $(wildcard $(LIB_SRC_LIST))
LIB_OBJ_LIST := $(patsubst src/%.c,obj/%.o,$(wildcard $(LIB_SRC_LIST)))

LIB_SCC := libscc.a

TEST_SRC_LIST := main.cc \
	test-file-reader.cc \
	test-cbuf.cc \
	test-htab.cc \
	test-parser.cc 

	# XXX ignore the cases temporarily to make the output less
	# test-lexer.cc \

TEST_SRC_LIST := $(patsubst %,test/%,$(TEST_SRC_LIST))

PROG_NAME := scc

all: handy

handy: rebuild-$(PROG_NAME)
	./scc integration-test/syntax_if/main.c

rebuild-$(PROG_NAME): clean $(PROG_NAME)

$(PROG_NAME): $(LIB_SCC) obj/main.o
	gcc $(CFLAGS) -o $@ obj/main.o $(LIB_SCC)

obj/%.o: src/%.c
	@echo build $@
	@mkdir -p $(@D)
	@gcc -c $^ -o $@ $(CFLAGS)

build-lib: $(LIB_SCC)

$(LIB_SCC): $(LIB_OBJ_LIST)
	@ar -r $(LIB_SCC) $^

rebuild-test: clean build-test

build-test: $(LIB_SCC)
	g++ $(CFLAGS) $(GTEST_CFLAGS) $(GTEST_LDFLAGS) -o test/runtest $(TEST_SRC_LIST) $(LIB_SCC)
	./test/runtest

clean:
	rm -rf obj
	rm -f $(LIB_SCC)
	rm -f test/runtest
	rm -f scc

btest:
	@integration-test/batch-test.sh
