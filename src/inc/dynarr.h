#ifndef _INC_DYNARR_H
#define _INC_DYNARR_H

#ifdef __cplusplus
extern "C" {
#endif

struct dynarr;
struct dynarr *dynarr_init();
int dynarr_size(struct dynarr *darr);
void dynarr_destroy(struct dynarr *darr);
void *dynarr_get(struct dynarr *darr, int ind);
void dynarr_add(struct dynarr *darr, void *item);
void *dynarr_first(struct dynarr *darr);
void *dynarr_last(struct dynarr *darr);
void dynarr_clear(struct dynarr *darr);

/* NO append struct to the type */
#define DYNARR_FOREACH_PLAIN_BEGIN(darr, type, each) do { \
	int _i; \
	type each; \
	for (_i = 0; _i < dynarr_size(darr); _i++) { \
		each = dynarr_get(darr, _i)

#define DYNARR_FOREACH_BEGIN(darr, type, each) do { \
	int _i; \
	struct type *each; \
	for (_i = 0; _i < dynarr_size(darr); _i++) { \
		each = dynarr_get(darr, _i)

#define DYNARR_FOREACH_END() } \
} while (0)

// intstack.c
#define intstack dynarr
#define intstack_init dynarr_init
#define intstack_destroy dynarr_destroy
#define intstack_size dynarr_size
void intstack_push(struct intstack *stk, int v);
int intstack_pop(struct intstack *stk);
int intstack_top(struct intstack *stk);
int intstack_subtop(struct intstack *stk);

#ifdef __cplusplus
}
#endif

#endif
