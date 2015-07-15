#include <inc/type.h>
#include <inc/util.h>

void type_destroy(struct type *type) {
	panic("ni");
}

struct type *parse_type_from_decl_specifiers(struct declaration_specifiers *decl_specifiers) {
	panic("ni");
}

/*
 * the sufflist is the list of direct_declarator_suffix. This method will assum
 * that each element in the list is one dimension of the array
 */
struct type *parse_array_type(struct type *base_type, struct dynarr *sufflist) {
	panic("ni");
}
