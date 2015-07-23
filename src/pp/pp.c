#include <stdio.h>
#include <inc/util.h>
#include <inc/pp.h>
#include <inc/lexer.h>
#include <inc/file_reader.h>

#define DEBUG 1

/*
 * XXX Right now, the implemenation does not check the case that #else can not
 * be followed by #elif or (other) #else. If we want to check, we need handle 
 * that in both skip mode and non-skip mode
 */

enum {
	SKIP_TO_ALTERNATIVE,
	SKIP_TO_END,
};

static void put_back_sharp_if(struct lexer *lexer) {
	lexer_put_back(lexer, wrap_to_simple_token(PP_TOK_IF));
	lexer_put_back(lexer, wrap_to_simple_token(TOK_SHARP));
}

/*
 * When calling this method, lexer->if_nest_level does not count for current
 * conditional compiling block yet.
 * XXX pp_skip actually is the key for conditional compiling
 */
static void pp_skip(struct lexer *lexer, int mode) {
	assert(!lexer->want_sharp); // want_sharp can not nesting
	lexer->want_sharp = 1;

	union token tok;
	bool follow_sharp = false;
	int nest_level = 0;
	(void) nest_level;

	// XXX take care of nest_level..
	while (true) {
		tok = lexer_next_token(lexer);
		if (follow_sharp) {
			switch (tok.tok_tag) {
			case PP_TOK_DEFINE: case PP_TOK_UNDEF: 
				break;
			case PP_TOK_INCLUDE: // include need special handling since
				// '.' in stdio.h can not be treat as token in the lexer
				pp_include(lexer, true);
				break;
			case PP_TOK_IF: case PP_TOK_IFDEF: case PP_TOK_IFNDEF:
				nest_level++;
				break;
			case PP_TOK_ELIF:
				if (nest_level > 0)  {
					break;
				}
				if (mode == SKIP_TO_END) {
					break;
				}
				put_back_sharp_if(lexer);
				goto out;
			case PP_TOK_ELSE:
				if (nest_level > 0) {
					break;
				}
				if (mode == SKIP_TO_END) {
					break;
				}
				// this is a if true case
				lexer->cstream->if_nest_level++;
				goto out;
			case PP_TOK_ENDIF:
				if (nest_level > 0) {
					nest_level--;
					break;
				}
				goto out;
			default:
				token_dump(tok);
				panic("not support in skip mode yet: %s", token_tag_str(tok.tok_tag));
			}
		}
		follow_sharp = tok.tok_tag == TOK_SHARP;
		token_destroy(tok);
	}
out:

	lexer->want_sharp = 0;
}

static void pp_ifxdef(struct lexer *lexer, bool need_def) {
	union token tok = expect(lexer, TOK_IDENTIFIER);
	union token nxtok = expect(lexer, TOK_NEWLINE);
	(void) tok;
	(void) nxtok;

	if ((need_def && macro_defined(lexer, tok.id.s))
			|| (!need_def && !macro_defined(lexer, tok.id.s))) {
		token_destroy(tok);	
		lexer->cstream->if_nest_level++;
		return;
	}

	// need do skipping
	pp_skip(lexer, SKIP_TO_ALTERNATIVE);
}

static void pp_ifdef(struct lexer *lexer) {
	pp_ifxdef(lexer, true);
}

static void pp_ifndef(struct lexer *lexer) {
	pp_ifxdef(lexer, false);
}

static void pp_if(struct lexer *lexer) {
	int result = pp_expr(lexer, true);
	if (result) {
		lexer->cstream->if_nest_level++;
		return;
	}
	pp_skip(lexer, SKIP_TO_ALTERNATIVE);
}

static void pp_else(struct lexer *lexer) {
	if (lexer->cstream->if_nest_level == 0) {
		panic("unmatched #else");
	}
	lexer->cstream->if_nest_level--;
	pp_skip(lexer, SKIP_TO_END);
}

static void pp_endif(struct lexer *lexer) {
	if (lexer->cstream->if_nest_level == 0) {
		panic("unmatched #endif");
	}
	lexer->cstream->if_nest_level--;
}

void pp_entry(struct lexer *lexer) {
	int old_in_pp_context = lexer_push_config(lexer, in_pp_context, 1);
	int old_want_newline = push_want_newline(lexer, 1);
	int old_no_expand_macro = lexer_push_config(lexer, no_expand_macro, 1);

	union token tok = lexer_next_token(lexer);

	switch (tok.tok_tag) {
	case PP_TOK_INCLUDE:
		pp_include(lexer, false);
		break;
	case PP_TOK_IFNDEF:
		pp_ifndef(lexer);
		break;
	case PP_TOK_IFDEF:
		pp_ifdef(lexer);
		break;
	case PP_TOK_IF: 
		pp_if(lexer);
		break;
	case PP_TOK_ELIF: // in non-skip mode, when we encounter a ELIF, the block
		// followed by ELIF must be skipped
	case PP_TOK_ELSE: 
		pp_else(lexer);
		break;
	case PP_TOK_DEFINE:
		pp_define(lexer);
		// lexer_discard_line(lexer);
		break;
	case PP_TOK_UNDEF:
		pp_undef(lexer);
		// lexer_discard_line(lexer);
		break;
	case PP_TOK_ENDIF:
		pp_endif(lexer);
		break;
	default:
#if DEBUG
		// lexer_dump_remaining(lexer);
#endif
		token_dump(tok);
		panic("ni %s", token_tag_str(tok.tok_tag));
	}

	lexer_pop_config(lexer, no_expand_macro, old_no_expand_macro);
	lexer_pop_config(lexer, in_pp_context, old_in_pp_context);
	pop_want_newline(lexer, old_want_newline);
}


