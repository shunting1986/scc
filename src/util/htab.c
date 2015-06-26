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

struct hashtab {
	struct hashtab_item **buckets; // we use a ptr rather than a static array so that
		// in future we can do hash table expansion
	int nbucket;
};

static struct hashtab_item *alloc_item(const char *key, void *val) {
	struct hashtab_item *item = mallocz(sizeof(*item));
	item->key = strdup(key);
	item->val = val;
	return item;
}

static void destroy_item(struct hashtab_item *item) {
	free((void *) item->key);
	free(item->val);
	free(item);
}

static unsigned long elf_hash(const unsigned char *name) {
	unsigned long h = 0, g;
	while (*name) {
		h = (h << 4) + *name++;
		if (g = h & 0xf0000000) 
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

void htab_destroy(struct hashtab *tab) {
	int i;
	for (i = 0; i < tab->nbucket; i++) {
		struct hashtab_item *cur, *next;
		for (cur = tab->buckets[i]; cur != NULL; cur = next) {
			next = cur->next;
			destroy_item(cur);
		}
	}
	free(tab->buckets);
	free(tab);
}

void *htab_query(struct hashtab *htab, const char *key) {
	struct hashtab_item *head = htab->buckets[hash_fn(key) % htab->nbucket];
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
	int bind = hash_fn(key) % htab->nbucket;
	struct hashtab_item *item = alloc_item(key, val);
	item->next = htab->buckets[bind];
	htab->buckets[bind] = item;
}


