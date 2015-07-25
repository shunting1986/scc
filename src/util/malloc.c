#include <inc/util.h>
#include <stdio.h>

static int tot_alloc_bytes;

void *mallocz(size_t sz) {
	void *v = malloc(sz);
	assert(v != NULL);
	memset(v, 0, sz);
	tot_alloc_bytes += sz;
	return v;
}
