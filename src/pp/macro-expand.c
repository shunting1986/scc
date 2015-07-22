#include <inc/pp.h>
#include <inc/util.h>
#include <inc/lexer.h>

static void expand_obj_macro(struct lexer *lexer, const char *name, struct macro *macro, struct dynarr *out_list) {
	// TODO we can optimize to avoid deep copy the token
	DYNARR_FOREACH_PLAIN_BEGIN(macro->toklist, union token *, each);
		dynarr_add(out_list, token_deep_dup(each));
	DYNARR_FOREACH_END();
}

static void expand_func_macro(struct lexer *lexer, const char *name, struct macro *macro, struct dynarr *out_list) {
	struct dynarr *arg_list = dynarr_init();
	(void) arg_list;
	// TODO need revise the interface, should passin the macro arguments
	panic("ni");
}

static void merge_to_expanded_list(struct dynarr *expanded_list) {
	panic("ni");
}

/*
 * We do not expand macro when we are in the middle of expanding
 */
bool try_expand_macro(struct lexer *lexer, const char *name) {
	if (lexer->in_expanding_macro) {
		return false;
	}

	struct macro *macro = query_macro_tab(lexer, name);
	if (macro == NULL) {
		return false;
	}

	lexer->in_expanding_macro = 1;

	struct dynarr *expanded_list = dynarr_init();
	if (macro->type == MACRO_OBJ) {
		expand_obj_macro(lexer, name, macro, expanded_list);
	} else {
		expand_func_macro(lexer, name, macro, expanded_list);
	}
	merge_to_expanded_list(expanded_list);
	dynarr_destroy(expanded_list);

	lexer->in_expanding_macro = 0;
	return true;
}
