#include <string.h>

#include <inc/keyword.h>
#include <inc/lexer.h>
#include <inc/util.h>
#include <inc/htab.h>

static const char *keyword_str_list[] = {
	[TOK_CONST] = "const",
	[TOK_VOID] = "void",
	[TOK_INT] = "int",
	[TOK_RETURN] = "return",
	[TOK_CHAR] = "char",
	[TOK_WHILE] = "while",
	[TOK_FOR] = "for",
	[TOK_IF] = "if",
	[TOK_ELSE] = "else",
	[TOK_TOTAL_NUM] = NULL,
};

struct hashtab *keyword_hashtab = NULL;

// TODO handle the table destroy
static void setup_keyword_hashtab() {
	int tok = 0;
	const char *s;
	keyword_hashtab = htab_init();
	keyword_hashtab->nofreekey = keyword_hashtab->nofreeval = 1;

	for (tok = 0; tok < TOK_TOTAL_NUM; tok++) {
		s = keyword_str_list[tok];
		if (s != NULL) {
			htab_insert(keyword_hashtab, s, (void *) (long) tok);
		}
	}
}

int check_keyword_token(char *s) {
	if (keyword_hashtab == NULL) {
		setup_keyword_hashtab();
	}
	void *ret = htab_query(keyword_hashtab, s);
	if (ret == NULL) {
		return TOK_UNDEF;
	} else {
		return (int) (long) ret;
	}
}

const char *keyword_str(int tok_tag) {
	if (tok_tag >= 0 && tok_tag < TOK_TOTAL_NUM) {
		const char *cstr = keyword_str_list[tok_tag];
		if (cstr != NULL) {
			return cstr;
		}
	}
	panic("not support %s", token_tag_str(tok_tag));
}
