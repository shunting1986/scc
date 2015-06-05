#include <inc/util.h>

void *mallocz(size_t sz) {
	void *v = malloc(sz);
	memset(v, 0, sz);
	return v;
}
