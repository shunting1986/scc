#include <inc/type.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/dynarr.h>
#include <inc/cgasm.h>

#define DEBUG 1

#if TYPE_DEBUG
#define INIT_MAGIC .magic = TYPE_MAGIC,
#define CHECK_MAGIC(type) assert(type->magic == TYPE_MAGIC)
#define SET_MAGIC(type) type->magic = TYPE_MAGIC
#else
#define INIT_MAGIC
#define CHECK_MAGIC(type)
#define SET_MAGIC(type)
#endif

static int type_allocated, type_freed;

static struct type int_type = {
	.tag = T_INT,
	.flags = TYPE_FLAG_STATIC, // statically allocated
	INIT_MAGIC
	.size = 4,
};

// TODO make expression parsing completely independent of cgasm module
static struct cgasm_context CONST_REQUIRED_CONTEXT = {
	.const_required = 1,
};

static void struct_field_destroy(struct struct_field *field);

void register_type_ref(struct cgasm_context *ctx, struct type *type) {
	dynarr_add(ctx->type_ref_list, type_get(type));
}

void free_type_ref_in_list(struct cgasm_context *ctx) {
	DYNARR_FOREACH_BEGIN(ctx->type_ref_list, type, each);
		type_put(each);
	DYNARR_FOREACH_END();
	dynarr_clear(ctx->type_ref_list);
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

	assert(type->field_list != NULL);
	DYNARR_FOREACH_BEGIN(type->field_list, struct_field, each);
		struct_field_destroy(each);
	DYNARR_FOREACH_END();

	dynarr_destroy(type->field_list);
}

void type_destroy(struct type *type) {
	CHECK_MAGIC(type);
	assert(type->ref_cnt == 0);
	struct type *tofree = NULL;

	switch (type->tag) {
	case T_INT: // no need to destroy
		break;
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
	default: 
		panic("ni %p, %d", type, type->tag);
	}

	if (tofree) {
		type_freed++;
#if DEBUG
	fprintf(stderr, "\033[31mtype destroy %p, %d\033[0m\n", tofree, tofree->tag); 
#endif
		free(tofree);
	}
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

void type_dump(struct type *type) {
	fprintf(stderr, "\033[31m");
	fprintf(stderr, "Type dump:\n");
	fprintf(stderr, "  tag %d, ref_cnt %d, size %d\n", type->tag, type->ref_cnt, type->size);
	fprintf(stderr, "\033[0m");
}

void type_put(struct type *type) {
	CHECK_MAGIC(type);
	// no reference count handling for static allocated type object
	if (type->flags & TYPE_FLAG_STATIC) {
		return;
	}

#if TYPE_DEBUG
	if (type->ref_cnt <= 0) {
		type_dump(type);
	}
#endif

	assert(type->ref_cnt > 0);
	if (--type->ref_cnt == 0) {
		type_destroy(type);
	} 
}

static struct type *alloc_type(int tag, int size) {
	struct type *type = mallocz(sizeof(*type));
#if DEBUG
	fprintf(stderr, "\033[31mallocated type %p, %d\033[0m\n", type, tag); 
#endif
	type->tag = tag;
	type->size = size;
	SET_MAGIC(type);
	type_allocated++;
	return type;
}

/*
 * XXX We may pre-created all the singletons for base type.
 * XXX think how to reclaim memory for types
 */
static struct type *get_int_type() {
	return &int_type;
}

struct type *get_ptr_type(struct type *elem_type) {
	struct type *ret_type = alloc_type(T_PTR, 4);
	ret_type->subtype = type_get(elem_type);
	return ret_type;
}

static struct type *get_array_type(struct type *elem_type, int dim) {
	struct type *ret_type = alloc_type(T_ARRAY, dim * elem_type->size);
	ret_type->subtype = type_get(elem_type);
	ret_type->dim = dim;
	return ret_type;
}

static struct type *create_struct_type(int size, struct dynarr *field_list) {
	struct type *ret = alloc_type(T_STRUCT, size);
	ret->field_list = field_list;
	return ret;
}

/* 
 * XXX: Use linear scan since the list suppose to be relatively short
 * TODO handle union
 */
struct struct_field *get_struct_field(struct type *type, const char *name) {
	assert(type != NULL && type->tag == T_STRUCT && type->field_list != NULL);
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
	type_put(field->type);
	free(field);
}

struct struct_field *struct_field_init(const char *name, struct type *type, int offset) {
	struct struct_field *field = mallocz(sizeof(*field));
	field->name = name;
	field->type = type_get(type);
	field->offset = offset;
	return field;
}

static int parse_struct_field_list_by_decl(struct cgasm_context *ctx, struct struct_declaration *decl, struct dynarr *field_list, int offset) {
	struct type *type = parse_type_from_specifier_qualifier_list(ctx, decl->sqlist);
	int size = 0;
	const char *id;
	(void) type;
	DYNARR_FOREACH_BEGIN(decl->declarator_list, struct_declarator, each);
		if (each->declarator == NULL) {
			panic("does not support struct declartion without declarator right now");
		}

		if ((id = each->declarator->direct_declarator->id) == NULL) {
			panic("only support struct declaration with id right now");
		}

		if (each->const_expr != NULL) {
			panic("does not support struct declartion with field width right now");
		}

		struct type *final_type = parse_type_from_declarator(type, each->declarator);
		if (final_type->size < 0) {
			panic("The size of symbol is undefined: %s", id);
		}
	
		dynarr_add(field_list, struct_field_init(id, final_type, offset));
		offset += final_type->size;
		size += final_type->size;
	DYNARR_FOREACH_END();
	return size;
}

static struct dynarr *parse_struct_field_list_by_decl_list(struct cgasm_context *ctx, struct struct_declaration_list *decl_list, int *size_ret) {
	int size = 0;
	struct dynarr *field_list = dynarr_init();
	int offset;

	DYNARR_FOREACH_BEGIN(decl_list->decl_list, struct_declaration, each);
		offset = size;
		size += parse_struct_field_list_by_decl(ctx, each, field_list, offset);
	DYNARR_FOREACH_END();

	if (size_ret) {
		*size_ret = size;
	}
	return field_list;
}

static struct type *create_noncomplete_struct_type() {
	return create_struct_type(-1, NULL);
}

/*
 * This method does not inc the reference count for type
 */
static struct type *parse_struct_type_by_decl_list(struct cgasm_context *ctx, struct struct_declaration_list *decl_list) {
	int size;
	struct dynarr *field_list = parse_struct_field_list_by_decl_list(ctx, decl_list, &size);
	return create_struct_type(size, field_list);
}

static void complete_struct_definition(struct cgasm_context *ctx, struct type *struct_type, struct struct_declaration_list *decl_list) {
	CHECK_MAGIC(struct_type);
	assert(struct_type->tag == T_STRUCT && struct_type->size < 0 && struct_type->field_list == NULL);
	int size;
	struct dynarr *field_list = parse_struct_field_list_by_decl_list(ctx, decl_list, &size);
	struct_type->size = size;
	struct_type->field_list = field_list;
}

/*
 * This method will retrieve the struct type and 
 * 1. create one if not exist yet 
 * 2. complete one if this is a definition and we have not defined the struct yet
 */
static struct type *cgasm_get_register_struct_type(struct cgasm_context *ctx, const char *name, struct struct_declaration_list *decl_list) {
	if (name == NULL) {
		assert(decl_list != NULL);
		return parse_struct_type_by_decl_list(ctx, decl_list);
	}
	if (decl_list == NULL) {
		// recursively retrieve the struct type
		struct type *struct_type = cgasm_get_struct_type_by_name(ctx, name, true); // XXX this method only cover the case for struct (not union)
		if (struct_type == NULL) {
			struct_type = create_noncomplete_struct_type();
			cgasm_add_struct_type(ctx, name, struct_type);
		}
		return struct_type;
	} else {
		// get struct type from current scope
		struct type *struct_type = cgasm_get_struct_type_by_name(ctx, name, false);
		if (struct_type == NULL) {
			struct_type = parse_struct_type_by_decl_list(ctx, decl_list);
			cgasm_add_struct_type(ctx, name, struct_type);
		} else {
			// We must reuse the same type object since the noncomplete type object may 
			// already being refered
			complete_struct_definition(ctx, struct_type, decl_list);
		}
		return struct_type;
	}
}

/*
 * darr is a list of type_specifier, type_qualifier, storage_class_specifier (maybe missing) for specifier_qualifier_list case)
 */ 
static struct type *parse_type_from_raw_type_list(struct cgasm_context *ctx, struct dynarr *darr) {
	int has_unsigned = 0;
	int num_long = 0;
	int basetype = T_NONE;
	struct type_specifier *type_specifier;
	struct type *type = NULL; // type name will set this directly

	DYNARR_FOREACH_BEGIN(darr, syntreebasenode, each);
		switch (each->nodeType) {
		case STORAGE_CLASS_SPECIFIER: 
			break; // XXX ignore storage class here
		case TYPE_QUALIFIER:
			panic("not supported yet");
			break;
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
			case TOK_INT:
				if (basetype != T_NONE) {
					panic("multi type specified");
				}
				basetype = T_INT;
				break;
			case TOK_TYPE_NAME:
				if (type != NULL) {
					panic("type already set");
				}
				type = cgasm_get_type_from_type_name(ctx, type_specifier->type_name);
				break;
			case TOK_STRUCT: 
				if (type != NULL) {
					panic("type already set");
				}
				type = cgasm_get_register_struct_type(ctx, type_specifier->type_name, type_specifier->struct_decl_list); // TODO support union in this same function
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

	if (num_long > 0) {
		panic("'long' not support yet");
	}
	if (has_unsigned) {
		panic("'unsigned' not support yet");
	}

	switch (basetype) {
	case T_INT:
		type = get_int_type();
		break;
	default:
		panic("not supported");
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


/*
 * the sufflist is the list of direct_declarator_suffix. This method will assum
 * that each element in the list is one dimension of the array
 */
struct type *parse_array_type(struct type *base_type, struct dynarr *sufflist) {
	assert(dynarr_size(sufflist) > 0);
	struct type *final_type = base_type;
	int i;

	for (i = dynarr_size(sufflist) - 1; i >= 0; i--) {
		struct direct_declarator_suffix *suff = dynarr_get(sufflist, i);
		int dim;
		if (suff->empty_bracket) {
			dim = -1;
		} else if (suff->const_expr) {
			dim = cgasm_interpret_const_expr(&CONST_REQUIRED_CONTEXT, suff->const_expr);
		} else {
			panic("require array dimension");
		}
		final_type = get_array_type(final_type, dim);
	}

	return final_type;
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


