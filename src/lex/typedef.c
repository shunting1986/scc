#include <inc/lexer.h>
#include <inc/util.h>
#include <inc/htab.h>

struct typedef_tab {
	struct typedef_tab *enclosing;
	struct hashtab *htab;
};

// NOTE: besides register typedef id, we should register non-typedef ids as well.
// Because non-typedef id may override typedef id
void lexer_register_typedef(struct lexer *lexer, char *id, int is_typedef) {
	htab_insert(lexer->typedef_tab->htab, id, (void *) (long) is_typedef);
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
	struct hashtab_item *item = NULL;
	struct typedef_tab *tab = lexer->typedef_tab;
	while (tab) {
		if ((item = htab_query_item(tab->htab, s)) != NULL) { // either 0 or NULL will be treated as non-typedef
			break;
		}
		tab = tab->enclosing;
	}

	if (item == NULL) {
		return false; // not found 
	} else {
		return (int) (long) item->val;
	}
}


