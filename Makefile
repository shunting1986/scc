# setting for gtest
GTEST_ROOT=/media/shunting/disk/compiler/3rdParty/gtest-1.7.0
GTEST_CFLAGS=-isystem $(GTEST_ROOT)/include
GTEST_LDFLAGS=$(GTEST_ROOT)/make/gtest-all.o -lpthread

CFLAGS+=-Isrc


# NOTE: should not add quote around each path
LIB_SRC_LIST := util/file_reader.c \
	util/util.c \
	lex/lexer.c

LIB_SRC_LIST := $(patsubst %,src/%,$(LIB_SRC_LIST))
LIB_SRC_LIST := $(wildcard $(LIB_SRC_LIST))
LIB_OBJ_LIST := $(patsubst src/%.c,obj/%.o,$(wildcard $(LIB_SRC_LIST)))

LIB_SCC := libscc.a

TEST_SRC_LIST := main.cc \
	test-file-reader.cc \
	test-lexer.cc

TEST_SRC_LIST := $(patsubst %,test/%,$(TEST_SRC_LIST))

obj/%.o: src/%.c
	@echo build $@
	@mkdir -p $(@D)
	@gcc -c $^ -o $@ $(CFLAGS)

build-lib: $(LIB_SCC)

$(LIB_SCC): $(LIB_OBJ_LIST)
	@ar -r $(LIB_SCC) $^

build-test: $(LIB_SCC)
	g++ $(CFLAGS) $(GTEST_CFLAGS) $(GTEST_LDFLAGS) -o test/runtest $(TEST_SRC_LIST) $(LIB_SCC)
	./test/runtest

clean:
	rm -rf obj
	rm $(LIB_SCC)
	rm test/runtest
