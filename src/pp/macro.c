#include <inc/pp.h>
#include <inc/util.h>

void macro_destroy(void *_macro) {
	panic("ni");
}

struct macro *obj_macro_init(struct dynarr *toklist) {
	struct macro *macro = mallocz(sizeof(*macro));
	macro->type = MACRO_OBJ;
	macro->toklist = toklist;
	return macro;
}

void macro_dump(const char *name, struct macro *macro) {
	printf("macro %d %s\n", macro->type, name);
	DYNARR_FOREACH_PLAIN_BEGIN(macro->toklist, union token *, each);
		token_dump(*each);
	DYNARR_FOREACH_END();
}
