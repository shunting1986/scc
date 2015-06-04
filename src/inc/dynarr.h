#ifndef _INC_DYNARR_H
#define _INC_DYNARR_H

#ifdef __cplusplus
extern "C" {
#endif

struct dynarr;
struct dynarr *dynarr_init();
int dynarr_size(struct dynarr *darr);
void dynarr_destroy(struct dynarr *darr);

#ifdef __cplusplus
}
#endif

#endif
