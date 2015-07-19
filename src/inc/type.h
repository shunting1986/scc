#ifndef _INC_TYPE_H
#define _INC_TYPE_H

#include <inc/syntree.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_DEBUG 1
#define TYPE_MAGIC 0x55565758

enum {
	T_INT,
	T_ARRAY,
	T_NONE, // this means no type specified. Different to T_VOID
	T_PTR,
	T_VOID,
};

#define TYPE_FLAG_STATIC 1

struct type {
	int tag; // T_INT etc.
	int flags;
	int size;
	struct type *subtype; // used for ptr and array
	int ref_cnt;

#if TYPE_DEBUG
	int magic;
#endif

	union {
		int dim; // for array
	};
};

struct cgasm_context;

void type_destroy(struct type *type);
struct type *parse_type_from_decl_specifiers(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers);
struct type *parse_type_from_specifier_qualifier_list(struct cgasm_context *ctx, struct specifier_qualifier_list *list);
struct type *parse_array_type(struct type *base_type, struct dynarr *sufflist);
int type_get_size(struct type *type);
struct type *type_get_elem_type(struct type *parent_type);
struct type *type_deref(struct type *type);
struct type *get_ptr_type(struct type *elem_type);
int type_get_tag(struct type *type);
struct type *type_get(struct type *type);
void type_put(struct type *type);
void register_type_ref(struct cgasm_context *ctx, struct type *type);
void free_type_ref_in_list(struct cgasm_context *ctx);
void verify_type_memory_release();

#ifdef __cplusplus
}
#endif

#endif
