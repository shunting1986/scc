#ifndef _INC_SYMTAB_H
#define _INC_SYMTAB_H

#ifdef __cplusplus
extern "C" {
#endif

struct symtab {
	struct symtab *enclosing;
	struct hashtab *htab;
};

struct symtab *symtab_init(struct symtab *enclosing);
void symtab_destroy(struct symtab *stab);

#ifdef __cplusplus
}
#endif

#endif
