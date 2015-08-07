MEMCACHED_SRC_ROOT=memcached/memcached-1.4.17
OUT_DIR=memcached/build

memcached-rebuild: rebuild-$(PROG_NAME) memcached

memcached:
	./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/util.c

.PHONY: memcached
