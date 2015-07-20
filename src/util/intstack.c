#include <inc/util.h>
#include <inc/dynarr.h>

void intstack_push(struct intstack *stk, int v) {
	dynarr_add(stk, (void *) (long) v);
}

int intstack_pop(struct intstack *stk) {
	assert(dynarr_size(stk) > 0);
	return (int) (long) stk->list[--stk->size];
}

int intstack_top(struct intstack *stk) {
	assert(dynarr_size(stk) > 0);
	return (int) (long) stk->list[stk->size - 1];
}

int intstack_subtop(struct intstack *stk) {
	assert(dynarr_size(stk) >= 2);
	return (int) (long) stk->list[stk->size - 2];
}
