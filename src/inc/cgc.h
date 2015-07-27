#ifndef _INC_CGC_H
#define _INC_CGC_H

#include <stdio.h>
#include <inc/syntree.h>

#ifdef __cplusplus
extern "C" {
#endif

struct syntree;
struct cgc_context;

struct cgc_context *cgc_context_init(FILE *fp, int index);
void cgc_context_destroy(struct cgc_context *ctx);
void cgc_tree(struct cgc_context *ctx, struct syntree *tree);
void cgc_declaration(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list);

const char *cgc_get_op_str(int tok_tag);

#ifdef __cplusplus
}
#endif

#endif
