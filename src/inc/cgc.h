#ifndef _INC_CGC_H
#define _INC_CGC_H

#ifdef __cplusplus
extern "C" {
#endif

struct syntree;
struct cgc_context;

struct cgc_context *cgc_context_init(FILE *fp, int index);
void cgc_context_destroy(struct cgc_context *ctx);
void cgc_tree(struct cgc_context *ctx, struct syntree *tree);

#ifdef __cplusplus
}
#endif

#endif
