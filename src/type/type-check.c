#include <inc/type-check.h>
#include <inc/type.h>

bool type_assignable(struct type *lhs, struct type *rhs) {
	if (type_eq(lhs, rhs)) {
		return true;
	}

	if (lhs->tag == T_PTR && rhs->tag == T_PTR) {
		if (lhs->subtype->tag == T_VOID || rhs->subtype->tag == T_VOID) {
			return true;
		}
	}

	return false;
}

bool type_eq(struct type *lhs, struct type *rhs) {
	if (lhs->tag != rhs->tag) {
		return false;
	}
	if (lhs->tag == T_ARRAY) {
		panic("check array type");
	}
	if (lhs->tag == T_PTR) {
		return type_eq(lhs->subtype, rhs->subtype);
	}
	if (lhs->tag == T_STRUCT || lhs->tag == T_UNION) {
		return lhs == rhs; // for struct/union, there is a single reference
	}
	if (lhs->tag == T_FUNC) {
		panic("check func type");
	}
	return true;
}
