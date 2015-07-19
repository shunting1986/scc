#include <inc/cgasm.h>
#include <inc/dynarr.h>
#include <inc/util.h>

static struct type *parse_type_from_declarator(struct type *base_type, struct declarator *declarator);

/*
 * need ctx for typedef
 */
struct type *parse_type_from_declaration(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct declarator *declarator) {
	struct type *base_type = parse_type_from_decl_specifiers(ctx, decl_specifiers);
	return parse_type_from_declarator(base_type, declarator);
}

static struct type *parse_type_from_declarator(struct type *base_type, struct declarator *declarator) {
	struct direct_declarator *dd = declarator->direct_declarator;
	struct type *final_type = base_type;

	// don't care about the id
	// XXX does not handle nested declarator yet

	// handle ptr
	DYNARR_FOREACH_BEGIN(declarator->ptr_list, type_qualifier_list, each);
		// XXX ignore the type qualifier right now
		(void) each;
		final_type = get_ptr_type(final_type);
	DYNARR_FOREACH_END();

	if (dynarr_size(dd->suff_list) > 0) {
		struct direct_declarator_suffix *suff = dynarr_get(dd->suff_list, 0);
		if (suff->empty_bracket || suff->const_expr) {
			final_type = parse_array_type(base_type, dd->suff_list);
		} else {
			panic("case not handled yet");
		}
	}
	return final_type;
}

void cgasm_declaration(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list) {
	// function declaration
	if (is_func_decl_init_declarator_list(init_declarator_list)) {
		return; // XXX ignore function declaration right now
	}

#ifdef NAIVE_IMPL
	struct dynarr *idlist = extract_id_list_from_init_declarator_list(init_declarator_list);
	DYNARR_FOREACH_PLAIN_BEGIN(idlist, char *, each);
		cgasm_add_decl_sym(ctx, each);
	DYNARR_FOREACH_END();
	// TODO free idlist
#else
	struct type *base_type = parse_type_from_decl_specifiers(ctx, decl_specifiers);
	struct type *final_type = NULL;
	DYNARR_FOREACH_BEGIN(init_declarator_list->darr, init_declarator, each);
		struct declarator *declarator = each->declarator;
		struct direct_declarator *dd = declarator->direct_declarator;
		char *id = dd->id;
		if (id == NULL) {
			panic("only support declarator with direct id right now");
		}

		final_type = parse_type_from_declarator(base_type, declarator);

		// register symbol id with type 'final_type'
		struct symbol *sym = cgasm_add_decl_sym(ctx, id, final_type);

		// handle initializer (XXX does not support struct initializer yet)
		// TODO: need handle global intializer correctly..
		struct initializer *initializer = each->initializer;
		if (initializer != NULL && initializer->expr != NULL) {
			(void) cgasm_handle_assign_op(ctx, symbol_expr_val(sym), cgasm_assignment_expression(ctx, initializer->expr), TOK_ASSIGN);
		}
	DYNARR_FOREACH_END();
#endif
}


