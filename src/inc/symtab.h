#ifndef _INC_SYMTAB_H
#define _INC_SYMTAB_H

#include <inc/htab.h>
#include <inc/type.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYMBOL_MAX_LEN 128

struct symtab {
	struct symtab *enclosing;
	struct hashtab *htab;

	struct dynarr *type_ref_list; // this is a list of type ref that need to release when finish parsing each scope. We move it from cgasm_context since each scope should has it's own list
};

enum {
	SYMBOL_GLOBAL_VAR,
	SYMBOL_LOCAL_VAR,
	SYMBOL_PARAM,

	SYMBOL_STRUCT_UNION,
	SYMBOL_ENUMERATOR,
	SYMBOL_UNDEF,
};

// NOTE: for struct/union, the key is decorated from the struct name but the name
// field is not 
#define struct_union_symbol symbol

#define SYMBOL_FLAG_TYPEDEF 1
#define SYMBOL_FLAG_EXTERN 2
#define SYMBOL_FLAG_STATIC 4

// NOTE: we will apply storage_class_specifier to symbol
struct symbol {
	int type;
	int flags;
	char name[SYMBOL_MAX_LEN];
	struct type *ctype;
};

struct enumerator_symbol {
	int type;
	int flags;
	char name[SYMBOL_MAX_LEN];
	struct type *ctype;
	int val;
};

struct global_var_symbol {
	int type;
	int flags;
	char name[SYMBOL_MAX_LEN];
	struct type *ctype;
};

struct local_var_symbol {
	int type;
	int flags;
	char name[SYMBOL_MAX_LEN];
	struct type *ctype;
	int var_ind;
};

// Note: although the syntax allow parameter declaration contains storage class
// GCC denied that.
struct param_symbol {
	int type;
	int flags;
	char name[SYMBOL_MAX_LEN];
	struct type *ctype;
	int param_ind;
};

struct symtab *symtab_init(struct symtab *enclosing);
void symtab_destroy(struct symtab *stab);
struct symbol *symtab_lookup_norec(struct symtab *stab, const char *id);
struct symbol *symtab_new_param(char *name, int ind, struct type *ctype);
struct symbol *symtab_new_local_var(char *name, int ind, struct type *ctype);
struct symbol *symtab_new_global_var(char *name, struct type *ctype);
struct symbol *symtab_new_struct_type(const char *name, struct type *ctype);
struct symbol *symtab_new_enumerator(const char *name, int val);
struct symbol *symtab_new_undef(const char *name);
void symtab_add(struct symtab *stab, struct symbol *sym);
void symtab_add_with_key(struct symtab *stab, const char *key, struct symbol *sym);
struct symbol *symtab_lookup(struct symtab *stab, const char *id);
void symtab_iter(struct symtab *symtab, void *ctx, htab_iter_fn_type *func);

void symbol_destroy(void *_sym);

#ifdef __cplusplus
}
#endif

#endif
