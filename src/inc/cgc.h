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
const char *cgc_get_op_str_noabort(int tok_tag);

void cgc_expression(struct cgc_context *ctx, struct expression *expr);
void cgc_translation_unit(struct cgc_context *ctx, struct translation_unit *trans_unit);
void cgc_external_declaration(struct cgc_context *ctx, struct external_declaration *external_decl);
void cgc_function_definition(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt);
void cgc_declaration_specifiers(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers);
void cgc_specifier_qualifier_list(struct cgc_context *ctx, struct specifier_qualifier_list *list);
void cgc_type_specifier(struct cgc_context *ctx, struct type_specifier *type_specifier);
void cgc_type_qualifier(struct cgc_context *ctx, struct type_qualifier *qual);
void cgc_storage_class_specifier(struct cgc_context *ctx, struct storage_class_specifier *sc);
void cgc_declarator(struct cgc_context *ctx, struct declarator *declarator);
void cgc_direct_declarator(struct cgc_context *ctx, struct direct_declarator *direct_declarator);
void cgc_compound_statement(struct cgc_context *ctx, struct compound_statement *compound_stmt);
void cgc_statement(struct cgc_context *ctx, struct syntreebasenode *stmt);
void cgc_init_declarator_list(struct cgc_context *ctx, struct init_declarator_list *init_declarator_list);
void cgc_init_declarator(struct cgc_context *ctx, struct init_declarator *init_declarator);
void cgc_cast_expression(struct cgc_context *ctx, struct cast_expression *expr);
void cgc_assignment_expression(struct cgc_context *ctx, struct assignment_expression *expr);
void cgc_struct_declarator_list(struct cgc_context *ctx, struct dynarr *struct_declarator_list);
void cgc_struct_declaration(struct cgc_context *ctx, struct struct_declaration *field);

void cgc_conditional_expression(struct cgc_context *ctx, struct conditional_expression *cond_expr);
void cgc_unary_expression(struct cgc_context *ctx, struct unary_expression *unary_expr);
void cgc_initializer_list(struct cgc_context *ctx, struct initializer_list *initz_list);
void cgc_initializer(struct cgc_context *ctx, struct initializer *initializer);



void cgc_print(struct cgc_context *ctx, const char *fmt, ...);
void cgc_println(struct cgc_context *ctx, const char *fmt, ...);

#define cgc_dump_and_quit(type, val) do { \
	struct cgc_context *ctx = cgc_context_init(stderr, 2); \
	cgc_ ## type(ctx, val); \
	cgc_print(ctx, "\n"); \
	panic("debug"); \
} while(0)

#define cgc_dump(type, val) do { \
	struct cgc_context *ctx = cgc_context_init(stderr, 2); \
	cgc_ ## type(ctx, val); \
	cgc_print(ctx, "\n"); \
	cgc_context_destroy(ctx); \
} while(0)


#ifdef __cplusplus
}
#endif

#endif
