#ifndef _INC_HTAB_H
#define _INC_HTAB_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void htab_val_free_fn_type(void *val);
typedef void htab_iter_fn_type(void *ctx, const char *key, void *val);

struct hashtab {
	struct hashtab_item **buckets; // we use a ptr rather than a static array so that
		// in future we can do hash table expansion
	int nbucket;

	htab_val_free_fn_type *val_free_fn;
};

struct hashtab *htab_init();
void htab_destroy(struct hashtab *tab);
void *htab_query(struct hashtab *htab, const char *key);
void htab_insert(struct hashtab *htab, const char *key, void *val);
void htab_iter(struct hashtab *htab, void *ctx, htab_iter_fn_type *func);
void htab_nop_val_free(void *val);

#ifdef __cplusplus
}
#endif

#endif
