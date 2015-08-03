#ifndef _INC_LL_SUPPORT_H
#define _INC_LL_SUPPORT_H

#ifdef __cplusplus
extern "C" {
#endif

struct cgasm_context;
struct symbol;
struct type;

void cgasm_push_sym_ll(struct cgasm_context *ctx, struct symbol *sym);

#ifdef __cplusplus
}
#endif

#endif
