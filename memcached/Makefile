MEMCACHED_SRC_ROOT=memcached/memcached-1.4.17
MEMCACHED_OUT_DIR=memcached/build

memcached-rebuild: rebuild-$(PROG_NAME) memcached

memcached:
	@#./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/util.c
	@#./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/daemon.c
	@#./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/cache.c
	./scc -E -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/slabs.c

.PHONY: memcached
