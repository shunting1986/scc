#include <stdio.h>
#include <inc/util.h>
#include <inc/pp.h>
#include <inc/lexer.h>
#include <inc/file_reader.h>

#define DEBUG 1

static const char *sys_inc_paths[] = {
	"/usr/include",
	NULL,
};

void open_header_file(struct lexer *lexer, const char *incl_path, int incl_tok) {
	int i;
	const char *dir;
	char buf[1024];
	struct file_reader *fr = NULL;

	if (incl_tok == '"') {
		panic("#include \"...\" not supported yet");
	}

	for (i = 0; (dir = sys_inc_paths[i]) != NULL; i++) {
		snprintf(buf, sizeof(buf), "%s/%s", dir, incl_path);
#ifdef DEBUG
		fprintf(stderr, "try incl path %s\n", buf);
#endif

		if ((fr = file_reader_init(buf)) != NULL) {
			fr->prev = lexer->cstream;
			lexer->cstream = fr;
			return;
		}
	}
	panic("header file not found %s", incl_path);
}

static void pp_include(struct lexer *lexer) {
	union token tok = lexer_next_token(lexer);
	int incl_tok = tok.tok_tag;
	int term_tok;
	(void) term_tok;

	if (incl_tok == TOK_LT) {
		term_tok = TOK_GT;
	} else if (incl_tok == TOK_QUOTATION) {
		term_tok = TOK_QUOTATION;
	} else {
		panic("invalid %s", token_tag_str(incl_tok));
	}

	const char *incl_path = parse_string_literal(lexer, term_tok);
	(void) incl_path;

	open_header_file(lexer, incl_path, incl_tok);
}

static void pp_ifndef(struct lexer *lexer) {
	int old_want_newline = push_want_newline(lexer, 1);
	
	union token tok = expect(lexer, TOK_IDENTIFIER);
	union token nxtok = expect(lexer, TOK_NEWLINE);
	(void) nxtok;
	panic("ni %s %d", tok.id.s, macro_defined(lexer, tok.id.s));

	pop_want_newline(lexer, old_want_newline);
}

void pp_entry(struct lexer *lexer) {
	int old_in_pp_context = lexer->in_pp_context;
	lexer->in_pp_context = 1;

	union token tok = lexer_next_token(lexer);
	lexer->in_pp_context = old_in_pp_context;

	switch (tok.tok_tag) {
	case PP_TOK_INCLUDE:
		pp_include(lexer);
		break;
	case PP_TOK_IFNDEF:
		pp_ifndef(lexer);
		break;
	default:
#if DEBUG
		// lexer_dump_remaining(lexer);
#endif
		panic("ni %s", token_tag_str(tok.tok_tag));
	}
}
