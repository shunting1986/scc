#include <inc/pp.h>
#include <inc/util.h>
#include <inc/lexer.h>

#undef DEBUG
#define DEBUG 1

static void expand_obj_macro(struct lexer *lexer, const char *name, struct macro *macro, struct dynarr *out_list) {
	// TODO we can optimize to avoid deep copy the token
	DYNARR_FOREACH_PLAIN_BEGIN(macro->toklist, union token *, each);
		dynarr_add(out_list, token_deep_dup(each));
	DYNARR_FOREACH_END();
}

/* 
 * Need consider nested parenthesis. Return true if end with TOK_RPAREN
 */
static bool add_token_until_comma_or_paren(struct lexer *lexer, struct dynarr *out_list) {
	int nleft = 0;
	union token tok;
	union token *ptok;
	bool ret;
	while (true) {
		tok = lexer_next_token(lexer);
		switch (tok.tok_tag) {
		case TOK_LPAREN:
			nleft++;
			ptok = token_shallow_dup(&tok);
			break;
		case TOK_RPAREN:
			if (nleft == 0) {
				ret = true;
				goto out;
			} else {
				ptok = token_shallow_dup(&tok);
				nleft--;
				break;
			}
		case TOK_COMMA:
			if (nleft == 0) {
				ret = false;
				goto out;
			} else {
				ptok = token_shallow_dup(&tok);
				break;
			}
		default:
			ptok = 	token_shallow_dup(&tok);
			break;
		}
		dynarr_add(out_list, ptok);
	}
out:
	return ret;
}

static void expand_func_macro_with_args(struct lexer *lexer, struct dynarr *toklist, struct dynarr *param_list, struct dynarr *arg_list, struct dynarr *out_list) {
	panic("ni");
}

static void release_arg_list(struct dynarr *arg_list) {
	panic("ni");
}

static void expand_func_macro(struct lexer *lexer, const char *name, struct macro *macro, struct dynarr *out_list) {
	struct dynarr *arg_list = dynarr_init();
	struct dynarr *param_list = macro->paramlist;
	struct dynarr *arg;
	(void) arg_list;
	expect(lexer, TOK_LPAREN);
	union token tok;
	int i;
	bool end_with_paren;

	// obtain arg list
	if (dynarr_size(param_list) > 0) {
		for (i = 0; ; i++) {
			tok = lexer_next_token(lexer);
			if (tok.tok_tag == TOK_COMMA || tok.tok_tag == TOK_RPAREN) {
				panic("invalid comma or parenthesis in macro argument list");
			}
	
			arg = dynarr_init();
			dynarr_add(arg, token_shallow_dup(&tok));
			end_with_paren = add_token_until_comma_or_paren(lexer, arg);
			dynarr_add(arg_list, arg);
			if (end_with_paren) {
				if (i < dynarr_size(param_list) - 1) {
					panic("too few macro arguments");
				}
				break;
			}

			if (i == dynarr_size(param_list) - 1) {
				panic("too many macro arguments");
			}
		} 
	} else {
		expect(lexer, TOK_RPAREN);
	}

#if DEBUG
	{
		int i = 0;
		DYNARR_FOREACH_BEGIN(arg_list, dynarr, each);
			printf("arg %d:\n", ++i);
			token_list_dump(each);
		DYNARR_FOREACH_END();
	}
#endif
	
	expand_func_macro_with_args(lexer, macro->toklist, param_list, arg_list, out_list);
	release_arg_list(arg_list);
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
