#ifndef _INC_TYPE_H
#define _INC_TYPE_H

#include <inc/syntree.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	T_INT,
	T_ARRAY,
};

struct type {
	int tag;
	int size;
	struct type *subtype; // used for ptr and array

	union {
		int unused; // placeholder
	};
};

void type_destroy(struct type *type);
struct type *parse_type_from_decl_specifiers(struct declaration_specifiers *decl_specifiers);
struct type *parse_array_type(struct type *base_type, struct dynarr *sufflist);

#ifdef __cplusplus
}
#endif

#endif
