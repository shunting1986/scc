#include <inc/syntree.h>
#include <inc/util.h>

bool is_abstract_declarator(struct declarator *declarator) {
	struct direct_declarator *dd = declarator->direct_declarator;
	assert(dd->id == NULL || dd->declarator == NULL);
	if (dd->id == NULL && dd->declarator == NULL) {
		return true;
	}
	if (dd->id != NULL) {
		return false;
	}
	return is_abstract_declarator(dd->declarator);
}

static int is_return_stmt(struct syntreebasenode *stmt) {
	struct jump_statement *ret_stmt;
	return stmt->nodeType == JUMP_STATEMENT && (ret_stmt = (struct jump_statement *) stmt)->init_tok_tag == TOK_RETURN;
}

/*
 * return 1 if return statement is added
 */
int add_return_cond(struct compound_statement *compound_stmt) {
	struct dynarr *stmtList = compound_stmt->decl_or_stmt_list;
	int ret = 0;
	if (dynarr_size(stmtList) == 0 || !is_return_stmt(dynarr_last(stmtList))) {
		dynarr_add(stmtList, jump_statement_init(TOK_RETURN));
		ret = 1;
	}
	return ret;
}

static int is_func_decl_direct_declarator_suffix(struct direct_declarator_suffix *suff) {
	return suff->empty_paren || suff->param_type_list != NULL;
}

static int is_func_decl_declarator(struct declarator *declarator) {
	struct direct_declarator *direct_declarator = declarator->direct_declarator;
	if (direct_declarator->id == NULL) {
		return 0;
	}
	struct dynarr *darr = direct_declarator->suff_list;
	return dynarr_size(darr) == 1 && is_func_decl_direct_declarator_suffix(dynarr_get(darr, 0));
}

/*
 * check if the init_declarator_list is for a function declaration
 */
int is_func_decl_init_declarator_list(struct init_declarator_list *init_declarator_list) {
	if (init_declarator_list == NULL) { // struct declaration belongs to this case
		return 0;
	}
	struct dynarr *darr = init_declarator_list->darr;
	if (dynarr_size(darr) != 1) {
		return 0;
	}
	struct init_declarator *init_declarator = dynarr_get(darr, 0);
	return init_declarator->initializer == NULL && is_func_decl_declarator(init_declarator->declarator);
}

char *extract_id_from_declarator(struct declarator *declarator) {
	struct direct_declarator *dd = declarator->direct_declarator;
	if (dd->id != NULL) {
		return dd->id;
	}
	if (dd->declarator != NULL) {
		return extract_id_from_declarator(dd->declarator);
	}
	return NULL;
}

/*
 * Retrieve the identifier list from init_declarator_list
 */
struct dynarr *extract_id_list_from_init_declarator_list(struct init_declarator_list *init_declarator_list) {
	struct dynarr *darr = dynarr_init();
	DYNARR_FOREACH_BEGIN(init_declarator_list->darr, init_declarator, each);
		char * id = extract_id_from_declarator(each->declarator);
		if (id != NULL) {
			dynarr_add(darr, id);
		}
	DYNARR_FOREACH_END();
	return darr;
}

int has_typedef(struct declaration_specifiers *decl_specifiers) {
	DYNARR_FOREACH_BEGIN(decl_specifiers->darr, syntreebasenode, each);
		struct storage_class_specifier *scspec;
		if (each->nodeType == STORAGE_CLASS_SPECIFIER && (scspec = (void *) each)->tok_tag == TOK_TYPEDEF) {
			return true;
		}
	DYNARR_FOREACH_END();
	return false;
}

int has_extern(struct declaration_specifiers *decl_specifiers) {
	DYNARR_FOREACH_BEGIN(decl_specifiers->darr, syntreebasenode, each);
		struct storage_class_specifier *scspec;
		if (each->nodeType == STORAGE_CLASS_SPECIFIER && (scspec = (void *) each)->tok_tag == TOK_EXTERN) {
			return true;
		}
	DYNARR_FOREACH_END();
	return false;
}

int has_static(struct declaration_specifiers *decl_specifiers) {
	DYNARR_FOREACH_BEGIN(decl_specifiers->darr, syntreebasenode, each);
		struct storage_class_specifier *scspec;
		if (each->nodeType == STORAGE_CLASS_SPECIFIER && (scspec = (void *) each)->tok_tag == TOK_STATIC) {
			return true;
		}
	DYNARR_FOREACH_END();
	return false;
}
