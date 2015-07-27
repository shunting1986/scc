#ifndef _INC_SYNTREE_H
#define _INC_SYNTREE_H

#include <stdlib.h>
#include <stdio.h>
#include <inc/token.h>
#include <inc/syntree-node.h>
#include <inc/util.h>

#ifdef __cplusplus
extern "C" {
#endif

// this an abstract representation for the entire syntax tree
struct syntree {
	struct translation_unit *trans_unit;
};

struct syntree *syntree_init(struct translation_unit *trans_unit);

// handy function to dump the syntax tree in C
void syntree_dump(struct syntree *tree);
void syntree_destroy(struct syntree *tree);

// syntree-check.c
int is_func_decl_init_declarator_list(struct init_declarator_list *init_declarator_list);
struct dynarr *extract_id_list_from_init_declarator_list(struct init_declarator_list *init_declarator_list);
int add_return_cond(struct compound_statement *compound_stmt);
int has_typedef(struct declaration_specifiers *decl_specifiers);
bool is_abstract_declarator(struct declarator *declarator);

#ifdef __cplusplus
}
#endif

#endif

