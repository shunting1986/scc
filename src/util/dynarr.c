#include <stdlib.h>
#include <assert.h>

#include <inc/dynarr.h>
#include <inc/util.h>

struct dynarr *dynarr_init() {
	struct dynarr *darr = calloc(1, sizeof(*darr));
	return darr;
}

int dynarr_size(struct dynarr *darr) {
	return darr->size;
}

void dynarr_add(struct dynarr *darr, void *item) {
	if (darr->size >= darr->capa) {
		assert(darr->size == darr->capa);
		if (darr->list == NULL) {
			darr->capa = 8; // initial size
		} else {
			darr->capa <<= 1;
		}
		darr->list = realloc(darr->list, darr->capa * sizeof(*darr->list));
	}
	darr->list[darr->size++] = item;
}

void *dynarr_get(struct dynarr *darr, int ind) {
	if (ind < 0 || ind >= darr->size) {
		panic("index out of range");
	}
	return darr->list[ind];
}

void dynarr_set(struct dynarr *darr, int ind, void *val) {
	if (ind < 0 || ind >= darr->size) {
		panic("index out of range");
	}
	darr->list[ind] = val;
}

void *dynarr_first(struct dynarr *darr) {
	if (darr->size == 0) {
		panic("index out of range");
	}
	return darr->list[0];
}

void *dynarr_last(struct dynarr *darr) {
	if (darr->size == 0) {
		panic("index out of range");
	}
	return darr->list[darr->size - 1];
}

void dynarr_destroy(struct dynarr *darr) {
	if (darr->list) {
		free(darr->list);
	}
	free(darr);
}

void dynarr_clear(struct dynarr *darr) {
	darr->size = 0;
}

struct dynarr *dynarr_shallow_dup(struct dynarr *darr) {
	struct dynarr *ret = dynarr_init();
	DYNARR_FOREACH_PLAIN_BEGIN(darr, void *, each);
		dynarr_add(ret, each);
	DYNARR_FOREACH_END();
	return ret;
}
