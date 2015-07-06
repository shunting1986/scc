#include <inc/syntree.h>
#include <inc/util.h>

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
	struct dynarr *darr = init_declarator_list->darr;
	if (dynarr_size(darr) != 1) {
		return 0;
	}
	struct init_declarator *init_declarator = dynarr_get(darr, 0);
	return init_declarator->initializer == NULL && is_func_decl_declarator(init_declarator->declarator);
}

/*
 * Retrieve the identifier list from init_declarator_list
 */
struct dynarr *extract_id_list_from_init_declarator_list(struct init_declarator_list *init_declarator_list) {
	struct dynarr *darr = dynarr_init();
	DYNARR_FOREACH_BEGIN(init_declarator_list->darr, init_declarator, each);
		// XXX does care about initializer here
		struct declarator *declarator = each->declarator;
		char *id = declarator->direct_declarator->id;
		if (id != NULL) {
			dynarr_add(darr, id);
		}
	DYNARR_FOREACH_END();
	return darr;
}
