#include <inc/lexer.h>
#include <inc/util.h>
#include <inc/htab.h>

static void *static_val = (void *) 1;

struct typedef_tab {
	struct typedef_tab *enclosing;
	struct hashtab *htab;
};

void lexer_register_typedef(struct lexer *lexer, char *id) {
	htab_insert(lexer->typedef_tab->htab, id, static_val);
}

void lexer_push_typedef_tab(struct lexer *lexer) {
	lexer->typedef_tab = typedef_tab_init(lexer->typedef_tab);
}

static void typedef_tab_destroy(struct typedef_tab *tab) {
	htab_destroy(tab->htab);
	free(tab);
}

void lexer_pop_typedef_tab(struct lexer *lexer) {
	// pop the top tab
	struct typedef_tab *old_tab = lexer->typedef_tab;
	assert(old_tab != NULL);
	lexer->typedef_tab = old_tab->enclosing;
	typedef_tab_destroy(old_tab);
}

struct typedef_tab *typedef_tab_init(struct typedef_tab *enclosing) {
	struct typedef_tab *tab = (struct typedef_tab *) mallocz(sizeof(*tab));
	tab->enclosing = enclosing;
	tab->htab = htab_init();
	tab->htab->val_free_fn = htab_nop_val_free;
	return tab;
}

int lexer_is_typedef(struct lexer *lexer, const char *s) {
	if (lexer->disable_typedef) {
		return false;
	}
	void *ret;
	struct typedef_tab *tab = lexer->typedef_tab;
	while (tab) {
		if ((ret = htab_query(tab->htab, s)) != NULL) {
			break;
		}
		tab = tab->enclosing;
	}
	return ret != NULL;
}


