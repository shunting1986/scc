#include <inc/pp.h>
#include <inc/util.h>

/*
 * XXX: For func macro, the string in paramlist should be freed as well
 */
void macro_destroy(void *_macro) {
	panic("ni");
}

struct macro *func_macro_init(struct dynarr *paramlist, struct dynarr *toklist) {
	struct macro *macro = mallocz(sizeof(*macro));
	macro->type = MACRO_FUNC;
	macro->toklist = toklist;
	macro->paramlist = paramlist;
	return macro;
}

struct macro *obj_macro_init(struct dynarr *toklist) {
	struct macro *macro = mallocz(sizeof(*macro));
	macro->type = MACRO_OBJ;
	macro->toklist = toklist;
	return macro;
}

void macro_dump(const char *name, struct macro *macro) {
	printf("macro %d %s\n", macro->type, name);
	if (macro->type == MACRO_FUNC) {
		printf("-> param list\n");
		DYNARR_FOREACH_PLAIN_BEGIN(macro->paramlist, const char *, each);
			printf("  %s\n", each);
		DYNARR_FOREACH_END();
	}
	printf("-> tok list\n");
	token_list_dump(macro->toklist);
}
