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

#define DYNARR_FOREACH_BEGIN(darr, type, each) do { \
	int _i; \
	struct type *each; \
	for (_i = 0; _i < dynarr_size(darr); _i++) { \
		each = dynarr_get(darr, _i)

#define DYNARR_FOREACH_END() } \
} while (0)

#ifdef __cplusplus
}
#endif

#endif
