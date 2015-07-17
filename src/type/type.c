#include <inc/type.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/dynarr.h>
#include <inc/cgasm.h>

#if TYPE_DEBUG
#define INIT_MAGIC .magic = TYPE_MAGIC,
#define CHECK_MAGIC(type) assert(type->magic == TYPE_MAGIC)
#define SET_MAGIC(type) type->magic = TYPE_MAGIC
#else
#define INIT_MAGIC
#define CHECK_MAGIC(type)
#define SET_MAGIC(type)
#endif

static struct type int_type = {
	.tag = T_INT,
	INIT_MAGIC
	.size = 4,
};

// TODO make expression parsing completely independent of cgasm module
static struct cgasm_context CONST_REQUIRED_CONTEXT = {
	.const_required = 1,
};

void type_destroy(struct type *type) {
	CHECK_MAGIC(type);
	switch (type->tag) {
	case T_INT: // we should have a separate place to free the reused types
		break;
	case T_ARRAY:
		type_destroy(type->subtype);
		free(type);
		break;
	case T_PTR:
		type_destroy(type->subtype);
		free(type);
		break;
	default:
		panic("ni %d", type->tag);
	}
}

static struct type *alloc_type(int tag, int size) {
	struct type *type = mallocz(sizeof(*type));
	type->tag = tag;
	type->size = size;
	SET_MAGIC(type);
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
	ret_type->subtype = elem_type;
	return ret_type;
}

static struct type *get_array_type(struct type *elem_type, int dim) {
	struct type *ret_type = alloc_type(T_ARRAY, dim * elem_type->size);
	ret_type->subtype = elem_type;
	ret_type->dim = dim;
	return ret_type;
}

struct type *parse_type_from_decl_specifiers(struct declaration_specifiers *decl_specifiers) {
	int has_unsigned = 0;
	int num_long = 0;
	int basetype = T_NONE;
	struct type_specifier *type_specifier;
	struct type *type = NULL;

	DYNARR_FOREACH_BEGIN(decl_specifiers->darr, syntreebasenode, each);
		switch (each->nodeType) {
		case STORAGE_CLASS_SPECIFIER: case TYPE_QUALIFIER:
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
				basetype = T_INT;
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


