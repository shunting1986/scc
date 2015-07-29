#include <inc/util.h>
#include <inc/htab.h>

/** 
 * No delete call.
 */

#define INIT_TABLE_SIZE 1024

// When destroying the table, the hashtab_item and the internal key, val should be
// freed
struct hashtab_item {
	const char *key;
	void *val;
	struct hashtab_item *next;
};

static struct hashtab_item *alloc_item(const char *key, void *val) {
	struct hashtab_item *item = mallocz(sizeof(*item));
	item->key = strdup(key);
	item->val = val;
	return item;
}

static void destroy_item(struct hashtab *tab, struct hashtab_item *item) {
	free((void *) item->key);

	if (tab->val_free_fn == NULL) {
		free(item->val);
	} else {
		tab->val_free_fn(item->val);
	}
	free(item);
}

static unsigned long elf_hash(const unsigned char *name) {
	unsigned long h = 0, g;
	while (*name) {
		h = (h << 4) + *name++;
		g = h & 0xf0000000;
		if (g) 
			h ^= g >> 24;
		h &= ~g;
	}
	return h;
}

static unsigned long (*hash_fn)(const unsigned char *name) = elf_hash;

struct hashtab *htab_init() {
	struct hashtab *htab = mallocz(sizeof(*htab));	
	htab->buckets = mallocz(INIT_TABLE_SIZE * sizeof(*htab->buckets));
	htab->nbucket = INIT_TABLE_SIZE;
	return htab;
}

// XXX this can take use of the htab_iter function
void htab_destroy(struct hashtab *tab) {
	int i;
	for (i = 0; i < tab->nbucket; i++) {
		struct hashtab_item *cur, *next;
		for (cur = tab->buckets[i]; cur != NULL; cur = next) {
			next = cur->next;
			destroy_item(tab, cur);
		}
	}
	free(tab->buckets);
	free(tab);
}

void htab_iter(struct hashtab *tab, void *ctx, htab_iter_fn_type *func) {
	int i;
	int num = 0;
	for (i = 0; i < tab->nbucket; i++) {
		struct hashtab_item *cur, *next;
		for (cur = tab->buckets[i]; cur != NULL; cur = next) {
			next = cur->next;
			func(ctx, cur->key, cur->val);
			num++;
		}
	}

	// do a validation
	assert(num == tab->nitem);
}

void *htab_query(struct hashtab *htab, const char *key) {
	struct hashtab_item *head = htab->buckets[hash_fn((unsigned char *) key) % htab->nbucket];
	for (; head != NULL; head = head->next) {
		if (strcmp(key, head->key) == 0) {
			return head->val;
		}
	}
	return NULL;
}

/* 
 * This method assumes that the key is not in the table yet. The key will be
 * duplciated and val will be used directly
 */
void htab_insert(struct hashtab *htab, const char *key, void *val) {
	if (htab->nitem >= htab->nbucket) {	
		red("htab too many item %d", htab->nitem); // TODO
	}

	int bind = hash_fn((unsigned char *) key) % htab->nbucket;
	struct hashtab_item *item = alloc_item(key, val);
	item->next = htab->buckets[bind];
	htab->buckets[bind] = item;
	htab->nitem++;
}

int htab_delete(struct hashtab *htab, const char *key) {
	struct hashtab_item **pptr = &htab->buckets[hash_fn((unsigned char *) key) % htab->nbucket];
	for (; *pptr != NULL; pptr = &(*pptr)->next) {
		if (strcmp(key, (*pptr)->key) == 0) {
			// found
			struct hashtab_item *todel = *pptr;
			*pptr = todel->next;
			destroy_item(htab, todel);

			htab->nitem--;
			return 1;
		}
	}
	return 0;
}

void htab_nop_val_free(void *val) {
	// do nothing
}
