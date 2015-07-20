#include <inc/util.h>
#include <inc/dynarr.h>

void intstack_push(struct intstack *stk, int v) {
	dynarr_add(stk, (void *) (long) v);
}

int intstack_pop(struct intstack *stk) {
	panic("ni");
}

int intstack_top(struct intstack *stk) {
	panic("ni");
}

int intstack_subtop(struct intstack *stk) {
	panic("ni");
}
