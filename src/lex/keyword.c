#include <string.h>

#include <inc/keyword.h>
#include <inc/lexer.h>
#include <inc/util.h>
#include <inc/htab.h>

static const char *keyword_str_list[] = {
	[TOK_CONST] = "const",
	[TOK_VOID] = "void",
	[TOK_INT] = "int",
	[TOK_LONG] = "long",
	[TOK_FLOAT] = "float",
	[TOK_DOUBLE] = "double",

	[TOK_RETURN] = "return",
	[TOK_CHAR] = "char",
	[TOK_DO] = "do",
	[TOK_WHILE] = "while",
	[TOK_FOR] = "for",
	[TOK_BREAK] = "break",
	[TOK_CONTINUE] = "continue",

	[TOK_IF] = "if",
	[TOK_ELSE] = "else",
	[TOK_SWITCH] = "switch",
	[TOK_CASE] = "case",
	[TOK_DEFAULT] = "default",
	[TOK_TYPEDEF] = "typedef",
	[TOK_SIZEOF] = "sizeof",
	[TOK_SIGNED] = "signed",
	[TOK_UNSIGNED] = "unsigned",
	[TOK_SHORT] = "short",
	[TOK_STRUCT] = "struct",
	[TOK_UNION] = "union",
	[TOK_ENUM] = "enum",
	[TOK_GOTO] = "goto",

	[TOK_EXTERN] = "extern",
	[TOK_STATIC] = "static",
	[TOK_VOLATILE] = "volatile",
	[TOK_REGISTER] = "register",
	[TOK_INLINE] = "inline",

	[TOK_TOTAL_NUM] = NULL,
};

static struct hashtab *keyword_hashtab = NULL;

struct hashtab *setup_keyword_hashtab(const char *list[], int size) {
	int tok = 0;
	const char *s;
	struct hashtab *tab = htab_init();
	tab->val_free_fn = htab_nop_val_free;

	for (tok = 0; tok < size; tok++) {
		s = list[tok];
		if (s != NULL) {
			htab_insert(tab, s, (void *) (long) tok);
		}
	}
	return tab;
}

int check_keyword_token(char *s) {
	// TODO handle the table destroy
	if (keyword_hashtab == NULL) {
		keyword_hashtab = setup_keyword_hashtab(keyword_str_list, TOK_TOTAL_NUM);
	}
	void *ret = htab_query(keyword_hashtab, s);
	if (ret == NULL) {
		return TOK_UNDEF;
	} else {
		return (int) (long) ret;
	}
}

const char *keyword_str_noabort(int tok_tag) {
	if (tok_tag >= 0 && tok_tag < TOK_TOTAL_NUM) {
		const char *cstr = keyword_str_list[tok_tag];
		if (cstr != NULL) {
			return cstr;
		}
	}
	return NULL;
}

/*
 * Used to dump the syntree back to C
 */
const char *keyword_str(int tok_tag) {
	const char *s = keyword_str_noabort(tok_tag);
	if (s == NULL) {
		panic("not support %s", token_tag_str(tok_tag));
	}
	return s;
}
