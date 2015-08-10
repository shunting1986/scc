MEMCACHED_SRC_ROOT=memcached/memcached-1.4.17
MEMCACHED_OUT_DIR=memcached/build

memcached-rebuild: rebuild-$(PROG_NAME) memcached

memcached:
	./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/util.c > $(MEMCACHED_OUT_DIR)/util.s || true
	./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/daemon.c > $(MEMCACHED_OUT_DIR)/daemon.s || true
	./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/cache.c > $(MEMCACHED_OUT_DIR)/cache.s || true
	./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/assoc.c > $(MEMCACHED_OUT_DIR)/assoc.s || true
	./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/stats.c > $(MEMCACHED_OUT_DIR)/stats.s || true
	./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/hash.c > $(MEMCACHED_OUT_DIR)/hash.s || true
	./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/items.c > $(MEMCACHED_OUT_DIR)/items.s || true
	./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/thread.c > $(MEMCACHED_OUT_DIR)/thread.s || true
	./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/slabs.c > $(MEMCACHED_OUT_DIR)/slabs.s || true
	./scc -DHAVE_CONFIG_H $(MEMCACHED_SRC_ROOT)/memcached-modified.c > $(MEMCACHED_OUT_DIR)/memcached-modified.s || true
	cd $(MEMCACHED_OUT_DIR); gcc -g -m32 assoc.s cache.s daemon.s hash.s items.s memcached-modified.s thread.s slabs.s stats.s util.s -levent -lpthread
	$(MEMCACHED_OUT_DIR)/a.out

.PHONY: memcached
