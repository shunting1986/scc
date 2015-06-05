#ifndef _INC_SYNTREE_H
#define _INC_SYNTREE_H

#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct syntree;
struct dynarr;

void syntree_dump(struct syntree *tree);

enum syntree_node_type {
	TRANSLATION_UNIT,
	EXTERNAL_DECL,
	TYPE_SPECIFIER,
	DIRECT_DECLARATOR,
	DECLARATOR,
	COMPOUND_STATEMENT,
};

struct syntreebasenode {
	int nodeType;
};

struct translation_unit_node {
	int nodeType;
	struct dynarr *external_decl_list;
};

struct external_decl_node {
	int nodeType;
};

struct declaration_specifiers {
	int nodeType;
	struct dynarr *darr;
};

struct declaration_specifiers *declaration_specifiers_init(struct dynarr *darr);

struct direct_declarator {
	int nodeType;
	char *id;
};

struct direct_declarator *direct_declarator_init();

struct declarator {
	int nodeType;
	struct direct_declarator *directDeclarator;
};

struct declarator *declarator_init();

struct type_specifier {
	int nodeType;
	int tok_tag; // specify type
	union syntreenode *extra;
};

struct type_specifier *type_specifier_init(int tok_tag);

struct compound_statement {
	int nodeType;
	struct dynarr *declList;
	struct dynarr *stmtList;
};

// I decide to put the syntreenode definition in .h file.
// May revise to put in .c file later
union syntreenode {
	struct syntreebasenode base;
	struct translation_unit_node translation_unit;
	struct external_decl_node external_decl;
};

#ifdef __cplusplus
}
#endif

#endif

