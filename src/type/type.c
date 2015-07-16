#include <inc/type.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/dynarr.h>

void type_destroy(struct type *type) {
	panic("ni");
}

static struct type *alloc_base_type(int tag, int size) {
	struct type *type = mallocz(sizeof(*type));
	type->tag = tag;
	type->size = size;
	return type;
}

/*
 * XXX We may pre-created all the singletons for base type.
 * XXX think how to reclaim memory for types
 */
static struct type *get_int_type() {
	return alloc_base_type(T_INT, 4);
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
	panic("ni");
}
