#ifndef _INC_TYPE_H
#define _INC_TYPE_H

#include <inc/syntree.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	T_INT,
	T_ARRAY,
	T_NONE, // this means no type specified. Different to T_VOID
	T_VOID,
};

struct type {
	int tag; // T_INT etc.
	int size;
	struct type *subtype; // used for ptr and array

	union {
		int dim; // for array
	};
};

void type_destroy(struct type *type);
struct type *parse_type_from_decl_specifiers(struct declaration_specifiers *decl_specifiers);
struct type *parse_array_type(struct type *base_type, struct dynarr *sufflist);

#ifdef __cplusplus
}
#endif

#endif
