#include <inc/util.h>
#include <inc/htab.h>

/** 
 * No delete call since we do not need that.
 */

struct hashtab *htab_init() {
	panic("ni");
}

void htab_destroy(struct hashtab *tab) {
	panic("ni");
}

void *htab_query(const char *key) {
	panic("ni");
}

/* 
 * This method assumes that the key is not in the table yet
 */
void htab_insert(const char *key, void *val) {
	panic("ni");
}
