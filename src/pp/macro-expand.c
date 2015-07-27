#include <inc/pp.h>
#include <inc/util.h>
#include <inc/lexer.h>

#undef DEBUG
#define DEBUG 0

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

// XXX use linear scan right now since I assume the list is short
static int find_param_index(struct dynarr *param_list, const char *name) {
	int i;
	for (i = 0; i < dynarr_size(param_list); i++) {
		const char *item = dynarr_get(param_list, i);
		if (strcmp(item, name) == 0) {
			return i;
		}
	}
	return -1;
}

static void concat_tok_list_with_deep_dup(struct dynarr *out_list, struct dynarr *arg) {
	DYNARR_FOREACH_PLAIN_BEGIN(arg, union token *, each);
		dynarr_add(out_list, token_deep_dup(each));
	DYNARR_FOREACH_END();
}

static void expand_func_macro_with_args(struct lexer *lexer, struct dynarr *toklist, struct dynarr *param_list, struct dynarr *arg_list, struct dynarr *out_list) {
	int ind;
	DYNARR_FOREACH_PLAIN_BEGIN(toklist, union token *, each);
		if (each->tok_tag == TOK_IDENTIFIER	&& (ind = find_param_index(param_list, each->id.s)) >= 0) {
			// concatenate the arg 
			// XXX can have better batch support
			concat_tok_list_with_deep_dup(out_list, dynarr_get(arg_list, ind));
		} else {
			// append the token
			dynarr_add(out_list, token_deep_dup(each));
		}
	DYNARR_FOREACH_END();
}

static void release_arg(struct dynarr *arg) {
	DYNARR_FOREACH_PLAIN_BEGIN(arg, union token *, each);	
		token_destroy(*each);
		free(each);
	DYNARR_FOREACH_END();
	dynarr_destroy(arg);
}

static void release_arg_list(struct dynarr *arg_list) {
	DYNARR_FOREACH_BEGIN(arg_list, dynarr, each);	
		release_arg(each);
	DYNARR_FOREACH_END();
	dynarr_destroy(arg_list);
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

			// NOTE: in some special cases like __attribute__((...)),
			// tok itself is TOK_LPAREN, we should not consume it but must put it back
			lexer_put_back(lexer, tok);
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
		fprintf(stderr, "func macro %s arguments:\n", name);
		int i = 0;
		DYNARR_FOREACH_BEGIN(arg_list, dynarr, each);
			fprintf(stderr, "  arg %d:\n", ++i);
			token_list_dump(each);
		DYNARR_FOREACH_END();
	}
#endif
	
	expand_func_macro_with_args(lexer, macro->toklist, param_list, arg_list, out_list);
	release_arg_list(arg_list);
}

static void merge_to_expanded_list(struct lexer *lexer, struct dynarr *expanded_list) {
	// NOTE: we should copy the pointer directly
	int i;
	for (i = lexer->expanded_macro_pos; i < dynarr_size(lexer->expanded_macro); i++) {
		union token *each = dynarr_get(lexer->expanded_macro, i);
		dynarr_add(expanded_list, each);
	}
	dynarr_destroy(lexer->expanded_macro);
	lexer->expanded_macro = expanded_list;
	lexer->expanded_macro_pos = 0;
}

/*
 * We do not expand macro when we are in the middle of expanding
 */
bool try_expand_macro(struct lexer *lexer, const char *name) {
	if (lexer->no_expand_macro) {
		return false;
	}

	struct macro *macro = query_macro_tab(lexer, name);
	if (macro == NULL) {
		return false;
	}

	lexer->no_expand_macro = 1;

	struct dynarr *expanded_list = dynarr_init();
	if (macro->type == MACRO_OBJ) {
		expand_obj_macro(lexer, name, macro, expanded_list);
	} else {
		expand_func_macro(lexer, name, macro, expanded_list);
	}
	merge_to_expanded_list(lexer, expanded_list); // the caller will release expanded_list
	// dynarr_destroy(expanded_list);

	lexer->no_expand_macro = 0;
	return true;
}
