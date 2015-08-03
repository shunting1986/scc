#include <inc/type.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/dynarr.h>
#include <inc/cgasm.h>
#include <inc/symtab.h>
#include <inc/cgc.h>
#include <inc/lexer.h>

/*
 * When we parse the array dimension, we still need symtab, because there may
 * by item like
 *    sizeof(var), sizeof(type)
 */

#define DEBUG 1

static int type_allocated, type_freed;

static struct type char_type = {
	.tag = T_CHAR,
	.flags = TYPE_FLAG_STATIC, // statically allocated
	INIT_MAGIC
	.size = 1,
};

static struct type char_ptr_type = {
	.tag = T_PTR,
	.flags = TYPE_FLAG_STATIC, // statically allocated
	.subtype = &char_type,
	INIT_MAGIC
	.size = 4,
};

static struct type int_type = {
	.tag = T_INT,
	.flags = TYPE_FLAG_STATIC, // statically allocated
	INIT_MAGIC
	.size = 4,
};

static struct type double_type = {
	.tag = T_DOUBLE,
	.flags = TYPE_FLAG_STATIC, // statically allocated
	INIT_MAGIC
	.size = 8,
};

static struct type float_type = {
	.tag = T_FLOAT,
	.flags = TYPE_FLAG_STATIC, // statically allocated
	INIT_MAGIC
	.size = 4,
};

static struct type long_long_type = {
	.tag = T_LONG_LONG,
	.flags = TYPE_FLAG_STATIC, // statically allocated
	INIT_MAGIC
	.size = 8,
};

static struct type short_type = {
	.tag = T_SHORT,
	.flags = TYPE_FLAG_STATIC, // statically allocated
	INIT_MAGIC
	.size = 2,
};

static struct type void_type = {
	.tag = T_VOID,
	.flags = TYPE_FLAG_STATIC, // statically allocated
	INIT_MAGIC
	.size = -1,
};

static void struct_field_destroy(struct struct_field *field);

void register_type_ref(struct cgasm_context *ctx, struct type *type) {
	dynarr_add(ctx->top_stab->type_ref_list, type_get(type));
}

void free_type_ref_in_list(struct symtab *stab) {
	DYNARR_FOREACH_BEGIN(stab->type_ref_list, type, each);
		type_put(each);
	DYNARR_FOREACH_END();
}

void verify_type_memory_release() {
#if DEBUG
	fprintf(stderr, "type statistic: allocated %d, freed %d\n", type_allocated, type_freed);
#endif
	assert(type_allocated == type_freed);
}

// for struct type, need free up the field list (also type_put for field type)
// the type itself is freed in caller
static void destroy_struct_union_type_nofree_itself(struct type *type) {
	assert(type->tag == T_STRUCT || type->tag == T_UNION);
	assert(type->subtype == NULL);

	if (type->field_list != NULL) {
		DYNARR_FOREACH_BEGIN(type->field_list, struct_field, each);
			struct_field_destroy(each);
		DYNARR_FOREACH_END();
	
		dynarr_destroy(type->field_list);
	}
}

static void destroy_func_type_nofree_itself(struct type *type) {
	assert(type->tag == T_FUNC);
	assert(type->subtype == NULL);

	type_put(type->func.retype);
	DYNARR_FOREACH_BEGIN(type->func.param_type_list, type, each);
		type_put(each);
	DYNARR_FOREACH_END();

	dynarr_destroy(type->func.param_type_list);
}

void type_destroy(struct type *type) {
	CHECK_MAGIC(type);
	assert(type->ref_cnt == 0);
	struct type *tofree = NULL;

	switch (type->tag) {
	case T_ARRAY:
		type_put(type->subtype);
		tofree = type;
		break;
	case T_PTR:
		type_put(type->subtype);
		tofree = type;
		break;
	case T_STRUCT: case T_UNION:
		destroy_struct_union_type_nofree_itself(type);
		tofree = type;
		break;
	case T_FUNC:
		destroy_func_type_nofree_itself(type);
		tofree = type;
		break;
	default: 
		panic("ni %p, %d", type, type->tag);
	}

	if (tofree) {
		type_freed++;
#if DEBUG && 0
	fprintf(stderr, "\033[31mdebug type memory free addr %p, %d\033[0m\n", tofree, tofree->tag); 
#endif
		free(tofree);
	}
}

static struct type *alloc_type(int tag, int size) {
	struct type *type = mallocz(sizeof(*type));
#if DEBUG && 0
	fprintf(stderr, "\033[31mdebug type memory allocate addr %p, %d\033[0m\n", type, tag); 
#endif
	type->tag = tag;
	type->size = size;
	SET_MAGIC(type);
	type_allocated++;
	return type;
}

struct type *type_get(struct type *type) {
	CHECK_MAGIC(type);
	// no reference count handling for static allocated type object
	if (type->flags & TYPE_FLAG_STATIC) {
		return type;
	}
	type->ref_cnt++;
	return type;
}

static void struct_field_list_dump(struct dynarr *list, int ind) {
	fprintf(stderr, "\033[31m");
	fprintf(stderr, "%*s", ind, "");
	fprintf(stderr, "struct fields ....\n");
	fprintf(stderr, "\033[0m");
}

static void type_list_dump(struct dynarr *list, int ind) {
	DYNARR_FOREACH_BEGIN(list, type, each);
		type_dump(each, ind);
	DYNARR_FOREACH_END();
}

void type_dump(struct type *type, int ind) {
	fprintf(stderr, "\033[31m");
	fprintf(stderr, "%*s", ind, "");
	fprintf(stderr, "T %d F 0x%x S %d CNT %d\n", type->tag, type->flags, type->size, type->ref_cnt);
	fprintf(stderr, "\033[0m");

	if (type->tag == T_ARRAY) {
		fprintf(stderr, "\033[31m");
		fprintf(stderr, "%*s", ind + 2, "");
		fprintf(stderr, "dim %d\n", type->dim);
		fprintf(stderr, "\033[0m");
		type_dump(type->subtype, ind + 2);
	} else if (type->tag == T_PTR) {
		type_dump(type->subtype, ind + 2);
	} else if (type->tag == T_STRUCT || type->tag == T_UNION) {
		struct_field_list_dump(type->field_list, ind + 2);
	} else if (type->tag == T_FUNC) {
		fprintf(stderr, "\033[31m");
		fprintf(stderr, "%*s", ind + 2, "");
		fprintf(stderr, "has ellipsis %d\n", type->func.has_ellipsis);
		type_dump(type->func.retype, ind + 2);
		fprintf(stderr, "\033[0m");
		fprintf(stderr, "\n");
		
		type_list_dump(type->func.param_type_list, ind + 2);
	}
}

/*
 * Destroy the type if it's reference count is 0; do nothing otherwise
 */
void type_check_ref(struct type *type) {
	CHECK_MAGIC(type);

	if (type->flags & TYPE_FLAG_STATIC) {
		return;
	}

	assert(type->ref_cnt >= 0);
	if (type->ref_cnt == 0) {
		type_destroy(type);
	}
}

void type_put(struct type *type) {
	CHECK_MAGIC(type);
	// no reference count handling for static allocated type object
	if (type->flags & TYPE_FLAG_STATIC) {
		return;
	}

#if TYPE_DEBUG
	if (type->ref_cnt <= 0) {
		type_dump(type, 0);
	}
#endif

	assert(type->ref_cnt > 0);
	if (--type->ref_cnt == 0) {
		type_destroy(type);
	} 
}

/*
 * XXX We may pre-created all the singletons for base type.
 * XXX think how to reclaim memory for types
 */
struct type *get_int_type() {
	return &int_type;
}

struct type *get_double_type() {
	return &double_type;
}

struct type *get_float_type() {
	return &float_type;
}

struct type *get_long_long_type() {
	return &long_long_type;
}

struct type *get_short_type() {
	return &short_type;
}

struct type *get_char_ptr_type() {
	return &char_ptr_type;
}

struct type *get_char_type() {
	return &char_type;
}

bool is_func_ptr(struct type *type) {
	CHECK_MAGIC(type);
	return type->tag == T_PTR && type->subtype->tag == T_FUNC;
}

bool is_void_ptr(struct type *type) {
	CHECK_MAGIC(type);
	return type->tag == T_PTR && type->subtype->tag == T_VOID;
}

struct type *get_void_type() {
	return &void_type;
}

struct type *get_ptr_type(struct type *elem_type) {
	struct type *ret_type = alloc_type(T_PTR, 4);
	ret_type->subtype = type_get(elem_type);
	return ret_type;
}

struct type *get_array_type(struct type *elem_type, int dim) {
	struct type *ret_type = alloc_type(T_ARRAY, dim * elem_type->size);
	ret_type->subtype = type_get(elem_type);
	ret_type->dim = dim;
	return ret_type;
}

void complete_array_dim(struct type *type, int dim) {
	CHECK_MAGIC(type);
	assert(type->tag == T_ARRAY);
	type->dim = dim;
	type->size = dim * type->subtype->size;
}

struct type *get_noparam_func_type(struct type *retype) {
	return get_func_type(retype, dynarr_init(), false);
}

struct type *get_func_type(struct type *retype, struct dynarr *param_type_list, bool has_ellipsis) {
	assert(!has_ellipsis || dynarr_size(param_type_list) > 0);
	struct type *ret = alloc_type(T_FUNC, -1);

	ret->func.retype = type_get(retype);
	DYNARR_FOREACH_BEGIN(param_type_list, type, each);
		type_get(each); // add ref count
	DYNARR_FOREACH_END();
	ret->func.param_type_list = param_type_list;
	ret->func.has_ellipsis = has_ellipsis;
	return ret;
}

static struct type *create_struct_type(bool is_struct, int size, struct dynarr *field_list) {
	struct type *ret = alloc_type(is_struct ? T_STRUCT : T_UNION, size);
	ret->field_list = field_list;
	return ret;
}

/* 
 * XXX: Use linear scan since the list suppose to be relatively short
 * TODO handle union
 */
struct struct_field *get_struct_field(struct type *type, const char *name) {
	assert(type != NULL && (type->tag == T_STRUCT || type->tag == T_UNION) && type->field_list != NULL);
	assert(name != NULL);
	CHECK_MAGIC(type);
	DYNARR_FOREACH_BEGIN(type->field_list, struct_field, each);
		if (strcmp(name, each->name) == 0) {
			return each;
		}
	DYNARR_FOREACH_END();
	return NULL;
}

// this method will free the field itself
static void struct_field_destroy(struct struct_field *field) {
	// type_put(field->type); // refer to the comments in struct_field_init
	free(field);
}

/* 
 * We can not use struct field to refer to the field type! Consider the following
 * mutually referring case:
 *   struct Tree {
 *     struct Node *root;
 *   };
 *
 *   struct Node {
 *     struct Tree *tree;
 *     struct Node *left, *right;
 *   };
 *
 * If we let struct field refer to field type, then neither of the types can be freed.
 *
 * Another simpler example is:
 *   struct Node {
 *     struct Node *next;
 *   };
 *
 * We break the mutual dependency by a trick:
 *   Instead of let struct field maintain the reference count, we put the field type
 *   to symtab.type_ref_list
 */
struct struct_field *struct_field_init(struct cgasm_context *ctx, const char *name, struct type *type, int offset) {
	struct struct_field *field = mallocz(sizeof(*field));
	field->name = name;
	// field->type = type_get(type);
	field->type = type;
	register_type_ref(ctx, type);
	field->offset = offset;

	return field;
}

static int parse_struct_field_list_by_decl(struct cgasm_context *ctx, int is_struct, struct struct_declaration *decl, struct dynarr *field_list, int offset) {
	struct type *type = parse_type_from_specifier_qualifier_list(ctx, decl->sqlist);
	int size = 0;
	char *id;
	(void) type;

	if (!is_struct) {
		assert(offset == 0);
	}
	/* TODO handle the special case like:
	 * struct st {
	 *   int a;
	 *   union {
	 *     int b;
	 *     int c;
	 *   };
	 * };
	 */
	// for type without declarator, we should register it so that the memory
	// can be reclaimed
	if (dynarr_size(decl->declarator_list) == 0) {
		register_type_ref(ctx, type);
	}

	DYNARR_FOREACH_BEGIN(decl->declarator_list, struct_declarator, each);
		struct type *final_type;

		if (each->declarator == NULL) {
			id = NULL;
			final_type = type;
		} else {
			id = NULL;
			final_type = parse_type_from_declarator(ctx, type, each->declarator, &id);
		}

		// id can be NULL if the struct declaration contains no declarator like
		// unsigned int  : 16;

		if (final_type->size < 0) {
			panic("The size of symbol is undefined: %s", id ? id : "<nil>");
		}

		struct struct_field *field = struct_field_init(ctx, id, final_type, offset);
		dynarr_add(field_list, field);

		if (each->const_expr != NULL) { 
			field->width = cgasm_interpret_const_expr(ctx, each->const_expr);
		}

		if (is_struct) {
			offset += final_type->size;
			size += final_type->size;
		} else {
			size = max(size, final_type->size);
		}
	DYNARR_FOREACH_END();
	return size;
}

static struct dynarr *parse_struct_field_list_by_decl_list(struct cgasm_context *ctx, bool is_struct, struct struct_declaration_list *decl_list, int *size_ret) {
	int size = 0;
	struct dynarr *field_list = dynarr_init();
	int offset;
	int delta;

	DYNARR_FOREACH_BEGIN(decl_list->decl_list, struct_declaration, each);
		offset = is_struct ? size : 0;
		delta = parse_struct_field_list_by_decl(ctx, is_struct, each, field_list, offset);
		if (is_struct) {
			size += delta;
		} else {
			size = max(size, delta);
		}
	DYNARR_FOREACH_END();

	if (size_ret) {
		*size_ret = size;
	}
	return field_list;
}

static struct type *create_noncomplete_struct_type(bool is_struct) {
	return create_struct_type(is_struct, -1, NULL);
}

/*
 * This method does not inc the reference count for type
 */
static struct type *parse_struct_type_by_decl_list(struct cgasm_context *ctx, bool is_struct, struct struct_declaration_list *decl_list) {
	int size;
	struct dynarr *field_list = parse_struct_field_list_by_decl_list(ctx, is_struct, decl_list, &size);
	return create_struct_type(is_struct, size, field_list);
}

static void complete_struct_definition(struct cgasm_context *ctx, struct type *struct_type, struct struct_declaration_list *decl_list) {
	CHECK_MAGIC(struct_type);
	assert((struct_type->tag == T_STRUCT || struct_type->tag == T_UNION) && struct_type->size < 0 && struct_type->field_list == NULL);
	int size;
	struct dynarr *field_list = parse_struct_field_list_by_decl_list(ctx, struct_type->tag == T_STRUCT, decl_list, &size);
	struct_type->size = size;
	struct_type->field_list = field_list;
}

/*
 * This method will retrieve the struct type and 
 * 1. create one if not exist yet 
 * 2. complete one if this is a definition and we have not defined the struct yet
 */
static struct type *cgasm_get_register_struct_type(struct cgasm_context *ctx, bool is_struct, const char *name, struct struct_declaration_list *decl_list) {
	if (name == NULL) {
		assert(decl_list != NULL);
		return parse_struct_type_by_decl_list(ctx, is_struct, decl_list);
	}
	if (decl_list == NULL) {
		// recursively retrieve the struct type
		struct type *struct_type = cgasm_get_struct_type_by_name(ctx, is_struct, name, true);
		if (struct_type == NULL) {
			struct_type = create_noncomplete_struct_type(is_struct);
			cgasm_add_struct_type(ctx, name, struct_type); // this method is independent on the value of is_struct
		}
		return struct_type;
	} else {
		// get struct type from current scope
		struct type *struct_type = cgasm_get_struct_type_by_name(ctx, is_struct, name, false);
		if (struct_type == NULL) {
			/* 
			 * We should add the struct type to symtab first and then parse the decl list.
			 * Otherwise, for the following case:
			 *   struct Node {
			 *     struct Node *next;
			 *   };
			 * At first we assume the symtab does not contains Node. After we parse the 
			 * decl list, a non-complete Node symbol is added to symtab. So the assumption
			 * is not true anymore.
			 */
			struct_type = create_noncomplete_struct_type(is_struct);
			cgasm_add_struct_type(ctx, name, struct_type);
			complete_struct_definition(ctx, struct_type, decl_list);
		} else {
			// We must reuse the same type object since the noncomplete type object may 
			// already being refered
			complete_struct_definition(ctx, struct_type, decl_list);
		}
		return struct_type;
	}
}

static int cgasm_register_enum(struct cgasm_context *ctx, struct enumerator *enumerator, int next_ord) {
	const char *name = enumerator->name;
	if (enumerator->expr != NULL) {
		next_ord = cgasm_interpret_const_expr(ctx, enumerator->expr);
	}

	cgasm_add_enumerator(ctx, name, next_ord);
	return next_ord + 1;
}

/*
 * XXX: current implementation just register all the enum and return int type.
 */
static struct type *cgasm_get_register_enum_type(struct cgasm_context *ctx, const char *name, struct enumerator_list *enum_list) {
	assert(name != NULL || enum_list != NULL);
	int next_ord = 0;

	if (enum_list != NULL) {
		DYNARR_FOREACH_BEGIN(enum_list->enum_list, enumerator, each);
			next_ord = cgasm_register_enum(ctx, each, next_ord);
		DYNARR_FOREACH_END();
	}

	return get_int_type();
}

/*
 * darr is a list of type_specifier, type_qualifier, storage_class_specifier (maybe missing) for specifier_qualifier_list case)
 */ 
static struct type *parse_type_from_raw_type_list(struct cgasm_context *ctx, struct dynarr *darr) {
	int has_unsigned = 0;
	int has_signed = 0;
	int num_long = 0;
	int basetype = T_NONE;
	struct type_specifier *type_specifier;
	struct type *type = NULL; // type name will set this directly

	DYNARR_FOREACH_BEGIN(darr, syntreebasenode, each);
		switch (each->nodeType) {
		case STORAGE_CLASS_SPECIFIER: 
			break; // XXX ignore storage class here
		case TYPE_QUALIFIER: {
			static int times = 0;
			if (++times <= 5) {
				fprintf(stderr, "\033[31mtype qualifier is ignored right now\033[0m\n");
			}
			break;
		}
		case TYPE_SPECIFIER:
			type_specifier = (struct type_specifier *) each;
			switch (type_specifier->tok_tag) {
			case TOK_LONG:
				num_long++;
				break;
			case TOK_UNSIGNED:
				if (has_unsigned) {
					panic("multiple 'unsigned' not allowed");
				}
				has_unsigned = true;
				break;
			case TOK_SIGNED:
				if (has_signed) {
					panic("multiple 'signed' not allowed");
				}
				has_signed = true;
				break;
			case TOK_INT:
				if (basetype == T_SHORT) {
					break; // do nothing
				} else if (basetype != T_NONE) {
					panic("multi type specified");
				}
				basetype = T_INT;
				break;
			case TOK_SHORT:
				if (basetype == T_INT) {
					// fall thru
				} else if (basetype != T_NONE) {
					panic("multi type specified");
				}
				basetype = T_SHORT;
				break;
			case TOK_FLOAT:
				if (basetype != T_NONE) {
					panic("multi type specified");
				}
				basetype = T_FLOAT;
				break;
			case TOK_DOUBLE:
				if (basetype != T_NONE) {
					panic("multi type specified");
				}
				basetype = T_DOUBLE;
				break;
			case TOK_CHAR:
				if (basetype != T_NONE) {
					panic("multi type specified");
				}
				basetype = T_CHAR;
				break;
			case TOK_VOID:
				if (basetype != T_NONE) {
					panic("multi type specified");
				}
				basetype = T_VOID;
				break;
			case TOK_TYPE_NAME:
				if (type != NULL) {
					panic("type already set");
				}
				type = cgasm_get_type_from_type_name(ctx, type_specifier->type_name);
				break;
			case TOK_STRUCT: case TOK_UNION:
				if (type != NULL) {
					panic("type already set");
				}
				type = cgasm_get_register_struct_type(ctx, type_specifier->tok_tag == TOK_STRUCT, type_specifier->type_name, type_specifier->struct_decl_list); 
				break;
			case TOK_ENUM:
				if (type != NULL) {
					panic("type already set");
				}
				type = cgasm_get_register_enum_type(ctx, type_specifier->type_name, type_specifier->enumerator_list);
				break;
			default:
				panic("not supported %s", token_tag_str(type_specifier->tok_tag));
			}
			break;
		default:
			panic("invalid");
			break;
		}
	DYNARR_FOREACH_END();

	if (type != NULL) {
		if (num_long > 0 || has_unsigned || basetype != T_NONE) {
			panic("other type mixed with type name/struct");
		}
		return type; // should inc the ref count
	}

	if (basetype == T_SHORT && num_long > 0) {
		panic("short and long can not be used at the same time");
	}

	// XXX ignore long if num_long == 1
	if (num_long >= 1) {
		if (num_long == 1) {
			if (basetype == T_NONE) {
				basetype = T_INT; // use int here
			}

			{
				static int times = 0;
				if (++times <= 5) {
					fprintf(stderr, "\033[31m'long' is ignored right now\033[0m\n");
				}
			}
		} else if (num_long == 2) {
			if (basetype == T_NONE || basetype == T_INT) {
				basetype = T_LONG_LONG;
			} else {
				panic("invalid usage of long long");
			}
		} else {
			panic("more that 3 longs");
		}
	}
	if (has_signed && has_unsigned) {
		panic("signed and unsigned can not be used at the same time");
	}

	if (has_signed) {
		static int times = 0;
		if (++times <= 5) {
			fprintf(stderr, "\033[31m'signed' is ignored right now\033[0m\n");
		}
	}

	if (has_unsigned) {
		static int times = 0;
		if (++times <= 5) {	
			fprintf(stderr, "\033[31m'unsigned' is ignored right now\033[0m\n");
		}
	}

	switch (basetype) {
	case T_INT: type = get_int_type(); break;
	case T_VOID: type = get_void_type(); break;
	case T_CHAR: type = get_char_type(); break;
	case T_SHORT: type = get_short_type(); break;
	case T_LONG_LONG: type = get_long_long_type(); break; 
	case T_DOUBLE: type = get_double_type(); break;
	case T_FLOAT: type = get_float_type(); break;
	case T_NONE:
		cgc_dump_and_quit(declaration_specifiers, declaration_specifiers_init(darr));
		panic("no type specified");
		break;
	default:
		panic("not supported %d", basetype);
	}
	return type;
}

struct type *parse_type_from_specifier_qualifier_list(struct cgasm_context *ctx, struct specifier_qualifier_list *list) {
	return parse_type_from_raw_type_list(ctx, list->darr);
}

/*
 * We need pass in context since type_name need refer to the symbol table
 */
struct type *parse_type_from_decl_specifiers(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers) {
	return parse_type_from_raw_type_list(ctx, decl_specifiers->darr);
}

int type_get_size(struct type *type) {
	assert(type != NULL);
	CHECK_MAGIC(type);
	return type->size;
}

struct type *type_get_elem_type(struct type *parent_type) {
	assert(parent_type != NULL);
	CHECK_MAGIC(parent_type);

	switch (parent_type->tag) {
	case T_ARRAY:
		return parent_type->subtype;
	case T_PTR:
		return parent_type->subtype;
	default:
		panic("invalid type %d", parent_type->tag);
	}
}

struct type *type_deref(struct type *type) {
	assert(type != NULL);
	CHECK_MAGIC(type);
	assert(type->tag == T_PTR);
	return type->subtype;
}

int type_get_tag(struct type *type) {
	assert(type != NULL);
	CHECK_MAGIC(type);
	return type->tag;
}


