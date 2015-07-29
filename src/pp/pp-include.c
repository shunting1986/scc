#include <inc/pp.h>
#include <inc/util.h>

static const char *sys_inc_paths[] = {
	"/media/shunting/disk/compiler/std-headers", // put this first so we can override the system provided header file, or provider builtin macros
	"/usr/include",
	// TODO remove this dependency
	// stddef.h included (maybe indirectly) by stdio.h need this path
	"/usr/lib/gcc/x86_64-linux-gnu/4.8/include", 
	NULL,
};

// if this method returns true, the new file is already opened and buffered
static bool try_incl_dir(struct lexer *lexer, const char *dir, const char *file) {
	char buf[1024];
	struct file_reader *fr = NULL;
	snprintf(buf, sizeof(buf), "%s/%s", dir, file);
#ifdef DEBUG
	fprintf(stderr, "[%s] try incl path %s\n", lexer->cstream->path, buf);
#endif

	if ((fr = file_reader_init(buf)) != NULL) {
		fr->prev = lexer->cstream;
		lexer->cstream = fr;
		return true;
	}
	return false;
}

void open_header_file(struct lexer *lexer, const char *incl_path, int incl_tok) {
	int i;
	const char *dir;

	// try relative path first before the system dirs
	if (incl_tok == '"') {
		assert(lexer->cstream != NULL);
		assert(lexer->cstream->path != NULL);
		char *dir = getdir(lexer->cstream->path);
		bool ret = try_incl_dir(lexer, dir, incl_path);
		free(dir);

		if (ret) {
			return;
		}
	}

	for (i = 0; (dir = sys_inc_paths[i]) != NULL; i++) {
		if (try_incl_dir(lexer, dir, incl_path)) {
			return;
		}
	}
	panic("header file not found %s", incl_path);
}

void pp_include(struct lexer *lexer, bool skip) {
	int old_want_quotation = push_want_quotation(lexer, 1);
	union token tok = lexer_next_token(lexer);
	pop_want_quotation(lexer, old_want_quotation);
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

#if 1
	if (!skip) {
		open_header_file(lexer, incl_path, incl_tok);
	}
#else
	fprintf(stderr, "\033[31m#include is disabled temporarily\033[0m\n");
#endif
}


