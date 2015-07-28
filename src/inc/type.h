#ifndef _INC_TYPE_H
#define _INC_TYPE_H

#include <inc/syntree.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_DEBUG 1
#define TYPE_MAGIC 0x55565758

enum {
	// basic type
	T_CHAR,
	T_SHORT,
	T_INT,
	T_LONG_LONG,
	T_VOID,
	T_FLOAT,
	T_DOUBLE,

	T_ARRAY,
	T_PTR,
	T_STRUCT,
	T_UNION,
	T_FUNC,

	T_NONE, // this means no type specified. Different to T_VOID
};

#define TYPE_FLAG_STATIC 1

struct cgasm_context;
struct symtab;

// also used for union
struct struct_field {
	const char *name; // no need to free name when destroying this struct, since the name
		// will finally get released when we destroy the syntax tree
	struct type *type;
	int offset;
	int width; 
};

struct struct_field *struct_field_init(struct cgasm_context *ctx, const char *name, struct type *type, int offset);

// NOTE: we will apply type_specifier and type_qualifier to type
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
		struct dynarr *field_list; // be null for everything except struct/union. 
			// be null for noncomplete struct type as well

		struct {
			struct type *retype;
			struct dynarr *param_type_list;
			bool has_ellipsis;
		} func;
	};
};

struct cgasm_context;

void type_destroy(struct type *type);
struct type *parse_type_from_decl_specifiers(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers);
struct type *parse_type_from_specifier_qualifier_list(struct cgasm_context *ctx, struct specifier_qualifier_list *list);
struct type *parse_array_type(struct cgasm_context *ctx, struct type *base_type, struct dynarr *sufflist);
int type_get_size(struct type *type);
struct type *type_get_elem_type(struct type *parent_type);
struct type *type_deref(struct type *type);
struct type *get_ptr_type(struct type *elem_type);
struct type *get_int_type();
struct type *get_char_type();
struct type *get_short_type();
struct type *get_long_long_type();
struct type *get_double_type();
struct type *get_float_type();
struct type *get_array_type(struct type *elem_type, int dim);
struct type *get_noparam_func_type(struct type *retype);
struct type *get_func_type(struct type *retype, struct dynarr *param_type_list, bool has_ellipsis);
int type_get_tag(struct type *type);
struct type *type_get(struct type *type);
void type_put(struct type *type);
void register_type_ref(struct cgasm_context *ctx, struct type *type);
void free_type_ref_in_list(struct symtab *stab);
void verify_type_memory_release();
void type_dump(struct type *type, int ind);

struct struct_field *get_struct_field(struct type *type, const char *name);

#ifdef __cplusplus
}
#endif

#endif
