#include <inc/pp.h>
#include <inc/util.h>
#include <inc/lexer.h>

static void expand_obj_macro(struct lexer *lexer, const char *name, struct macro *macro) {
	// TODO we can optimize to avoid deep copy the token
	DYNARR_FOREACH_PLAIN_BEGIN(macro->toklist, union token *, each);
		if (each->tok_tag == TOK_IDENTIFIER && try_expand_macro(lexer, each->id.s)) {
		} else {
			dynarr_add(lexer->expanded_macro, token_deep_dup(each));
		}
	DYNARR_FOREACH_END();
}

static void expand_func_macro(struct lexer *lexer, const char *name, struct macro *macro) {
	// TODO need revise the interface, should passin the macro arguments
	panic("ni");
}

bool try_expand_macro(struct lexer *lexer, const char *name) {
	struct macro *macro = query_macro_tab(lexer, name);
	if (macro == NULL) {
		return false;
	}
	normalize_expanded_token_list(lexer);
	if (macro->type == MACRO_OBJ) {
		expand_obj_macro(lexer, name, macro);
	} else {
		panic("need revise func macro interface right now");
		expand_func_macro(lexer, name, macro);
	}
	return true;
}
