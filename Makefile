GTEST_ROOT=/media/shunting/disk/compiler/3rdParty/gtest-1.7.0
GTEST_CFLAGS=-isystem $(GTEST_ROOT)/include
GTEST_LDFLAGS=$(GTEST_ROOT)/make/gtest-all.o -lpthread

CFLAGS+=-Isrc

run:
	find src -name "*.c" | xargs gcc $(CFLAGS)

build-test:
	find test -name "*.cc" | xargs g++ $(CFLAGS) $(GTEST_CFLAGS) $(GTEST_LDFLAGS) -o test/runtest 
	./test/runtest
