#include <inc/cgasm.h>
#include <inc/dynarr.h>
#include <inc/util.h>
#include <inc/cgc.h>
#include <inc/symtab.h>

static struct type *parse_type_from_parameter_declaration(struct cgasm_context *ctx, struct parameter_declaration *param_decl) {
	struct type *base_type = parse_type_from_decl_specifiers(ctx, param_decl->decl_specifiers);
	return param_decl->declarator == NULL ?
		base_type :
		parse_type_from_declarator(ctx, base_type, param_decl->declarator, NULL);
}

static struct type *parse_func_type_from_parameter_type_list(struct cgasm_context *ctx, struct type *base_type, struct parameter_type_list *param_type_list) {
	assert(dynarr_size(param_type_list->param_decl_list) > 0);
	struct dynarr *tlist = dynarr_init();

	DYNARR_FOREACH_BEGIN(param_type_list->param_decl_list, parameter_declaration, each);
		dynarr_add(tlist, parse_type_from_parameter_declaration(ctx, each));
	DYNARR_FOREACH_END();

	return get_func_type(base_type, tlist, param_type_list->has_ellipsis);
}

static struct type *parse_type_from_direct_declarator_suffixes(struct cgasm_context *ctx, struct type *base_type, struct dynarr *suff_list, int start_ind) {
	if (start_ind == dynarr_size(suff_list)) {
		return base_type;
	}

	struct direct_declarator_suffix *suff = dynarr_get(suff_list, start_ind);

	// array case
	if (suff->empty_bracket || suff->const_expr) {
		int dim = suff->empty_bracket ? -1 : cgasm_interpret_const_expr(ctx, suff->const_expr);
		base_type = parse_type_from_direct_declarator_suffixes(ctx, base_type, suff_list, start_ind + 1);
		return get_array_type(base_type, dim);
	}

	if (suff->empty_paren) {
		base_type = get_noparam_func_type(base_type);
	} else {
		// function declaration case
		base_type = parse_func_type_from_parameter_type_list(ctx, base_type, suff->param_type_list);
	}

	return parse_type_from_direct_declarator_suffixes(ctx, base_type, suff_list, start_ind + 1);
}

/*
 * Need pass in ctx, since array dimension may refer to variables in sizeof
 */
struct type *parse_type_from_declarator(struct cgasm_context *ctx, struct type *base_type, struct declarator *declarator, char **idret) {
	assert(declarator != NULL);
	struct direct_declarator *dd = declarator->direct_declarator;
	assert(dd != NULL);
	struct type *final_type = base_type;

	// handle ptr
	DYNARR_FOREACH_BEGIN(declarator->ptr_list, type_qualifier_list, each);
		// XXX ignore the type qualifier right now
		(void) each;
		final_type = get_ptr_type(final_type);
	DYNARR_FOREACH_END();

	final_type = parse_type_from_direct_declarator_suffixes(ctx, final_type, dd->suff_list, 0);

	if (dd->declarator != NULL) {	// nested case
		return parse_type_from_declarator(ctx, final_type, dd->declarator, idret);
	} 

	if (idret) {
		*idret = dd->id; // may be NULL;
	}
	return final_type;
}

void cgasm_declaration(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list) {
	#if 0
	// function declaration
	if (is_func_decl_init_declarator_list(init_declarator_list)) {
		return; // XXX ignore function declaration right now
	} 
	#endif

#ifdef NAIVE_IMPL
	struct dynarr *idlist = extract_id_list_from_init_declarator_list(init_declarator_list);
	DYNARR_FOREACH_PLAIN_BEGIN(idlist, char *, each);
		cgasm_add_decl_sym(ctx, each);
	DYNARR_FOREACH_END();
	// TODO free idlist
#else
	struct type *base_type = parse_type_from_decl_specifiers(ctx, decl_specifiers);
	struct type *final_type = NULL;

	if (init_declarator_list == NULL) {
		return; // the sideeffect is parse_type_from_decl_specifiers may
			// register struct type
	}
	DYNARR_FOREACH_BEGIN(init_declarator_list->darr, init_declarator, each);
		#if 0
		struct direct_declarator *dd = declarator->direct_declarator;
		char *id = dd->id;
		if (id == NULL) {
			panic("only support declarator with direct id right now");
		} 
		#endif
		struct declarator *declarator = each->declarator;
		char *id = NULL;

		final_type = parse_type_from_declarator(ctx, base_type, declarator, &id);

		bool symbol_flags = 0;
		if (has_typedef(decl_specifiers)) {
			symbol_flags |= SYMBOL_FLAG_TYPEDEF;
		}
		if (has_extern(decl_specifiers)) {
			symbol_flags |= SYMBOL_FLAG_EXTERN;
		}

		if (id == NULL) {
			panic("declarator requires id");
		}

		//   typedef struct _IO_FILE FILE;  or
		//   extern struct _IO_FILE FILE;  
		// is allowed even if we do not have the 
		// definition of struct _IO_FILE
		if (final_type->size < 0 && !(symbol_flags & (SYMBOL_FLAG_TYPEDEF | SYMBOL_FLAG_EXTERN)) && final_type->tag != T_FUNC) {
			panic("The size of symbol is undefined: %s", id);
		}

		// register symbol id with type 'final_type'
		struct symbol *sym = cgasm_add_decl_sym(ctx, id, final_type);
		sym->flags = symbol_flags; // TODO verify the flags for function redeclare

		if (ctx->func_ctx != NULL) { 
			// handle initializer (XXX does not support struct initializer yet)
			struct initializer *initializer = each->initializer;
			if (initializer != NULL && initializer->expr != NULL) {
				(void) cgasm_handle_assign_op(ctx, symbol_expr_val(sym), cgasm_assignment_expression(ctx, initializer->expr), TOK_ASSIGN);
			} 
		} else {
			// global variable
			cgasm_allocate_global_var(ctx, (struct global_var_symbol *) sym, each->initializer);
		}
	DYNARR_FOREACH_END();
#endif
}


