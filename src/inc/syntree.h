#ifndef _INC_SYNTREE_H
#define _INC_SYNTREE_H

#ifdef __cplusplus
extern "C" {
#endif

struct syntree;
struct dynarr;

void syntree_dump(struct syntree *tree);

enum syntree_node_type {
	TRANSLATION_UNIT,
	EXTERNAL_DECL,
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

