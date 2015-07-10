#ifndef _INC_HTAB_H
#define _INC_HTAB_H

#ifdef __cplusplus
extern "C" {
#endif

struct hashtab {
	struct hashtab_item **buckets; // we use a ptr rather than a static array so that
		// in future we can do hash table expansion
	int nbucket;
	int nofreekey;
	int nofreeval;
};

struct hashtab *htab_init();
void htab_destroy(struct hashtab *tab);
void *htab_query(struct hashtab *htab, const char *key);
void htab_insert(struct hashtab *htab, const char *key, void *val);

#ifdef __cplusplus
}
#endif

#endif
