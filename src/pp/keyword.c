#include <inc/pp.h>
#include <inc/util.h>
#include <inc/keyword.h>
#include <inc/htab.h>

static const char *keyword_str_list[] = {
	[PP_TOK_INCLUDE] = "include",
	[TOK_TOTAL_NUM] = NULL,
}; 

static struct hashtab *keyword_hashtab = NULL;

int check_pp_keyword(char *s) {
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
