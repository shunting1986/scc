#include <inc/pp.h>
#include <inc/util.h>

void macro_destroy(void *_macro) {
	panic("ni");
}

struct macro *obj_macro_init(struct dynarr *toklist) {
	struct macro *macro = malloc(sizeof(*macro));
	macro->type = MACRO_OBJ;
	macro->toklist = toklist;
	return macro;
}
