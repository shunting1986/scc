#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/symtab.h>
#include <inc/cgc.h>

static void cgasm_initialize_global_var(struct cgasm_context *ctx, struct type *type, struct initializer *initializer);
static void cgasm_initialize_local_var(struct cgasm_context *ctx, int base_reg, int offset, struct type *type, struct initializer *initializer);

static void cgasm_initialize_global_ll(struct cgasm_context *ctx, struct initializer *initializer) {
	struct assignment_expression *expr = initializer->expr;
	if (expr == NULL) {
		panic("inavlid initializer");
	}
	struct expr_val val = cgasm_assignment_expression(ctx, expr);
	long long const_val = cgasm_get_ll_const_from_expr(ctx, val);
	cgasm_println(ctx, ".quad %lld", const_val);
}

static void cgasm_initialize_global_vint(struct cgasm_context *ctx, struct initializer *initializer, const char *directive) {
	struct assignment_expression *expr = initializer->expr;
	if (expr == NULL) {
		panic("inavlid initializer");
	}
	struct expr_val val = cgasm_assignment_expression(ctx, expr);
	int const_val = cgasm_get_int_const_from_expr(ctx, val);
	cgasm_println(ctx, "%s %d", directive, const_val);
}

static void cgasm_initialize_global_ptr(struct cgasm_context *ctx, struct type *type, struct initializer *initializer) {
	char buf[256];
	struct expr_val val;
	int const_val;
	assert(type->tag == T_PTR);
	if (initializer->expr == NULL) {
		panic("invalid pointer initializer");
	}
	
	val = cgasm_assignment_expression(ctx, initializer->expr);

	// initialize char ptr
	if (type->subtype->tag == T_CHAR) { 
		if (val.type == EXPR_VAL_STR_LITERAL) { // use string literal to initialize char *
			cgasm_println(ctx, ".long %s", get_str_literal_label(val.ind, buf));
			return;
		} 
	}

	// initialize func ptr
	if (type->subtype->tag == T_FUNC) {
		if (val.type == EXPR_VAL_SYMBOL && val.ctype->tag == T_FUNC) {
			cgasm_println(ctx, ".long %s", val.sym->name);
			return;
		}
	}

	const_val = cgasm_get_int_const_from_expr(ctx, val);
	cgasm_println(ctx, ".long %d", const_val);
}

// TODO this function should make sure type's size is > 0 before it returns
static void cgasm_initialize_global_array(struct cgasm_context *ctx, struct type *type, struct initializer *initializer) {
	if (type->dim < 0 && initializer->initz_list != NULL) {
		complete_array_dim(type, dynarr_size(initializer->initz_list->list));
	}

	if (type->size < 0) {
		panic("The size of array is undefined");
	}

	int i;
	struct dynarr *list = initializer->initz_list->list;
	int size = dynarr_size(list);
	if (type->dim < size) {
		panic("The array dimension is too small");
	}

	for (i = 0; i < size; i++) {
		cgasm_initialize_global_var(ctx, type->subtype, dynarr_get(list, i));
	}

	if (size < type->dim) {
		cgasm_println(ctx, ".zero %d", type->subtype->size * (type->dim - size));
	}
}

static void cgasm_initialize_global_union(struct cgasm_context *ctx, struct type *type, struct initializer *initializer) {
	assert(type->tag == T_UNION);
	assert(type->size >= 0);
	if (initializer->initz_list == NULL) {
		panic("Invalid union initializer");
	}

	int specified_size = 0;
	struct dynarr *init_list = initializer->initz_list->list;
	struct dynarr *field_list = type->field_list;

	if (dynarr_size(init_list) > 1) {
		panic("Invalid union initializer"); // only allow single item
	} else if (dynarr_size(init_list) == 1) {
		if (dynarr_size(field_list) == 0) {
			panic("too many initializers");
		}
		struct initializer *field_init = dynarr_get(init_list, 0);
		struct struct_field *field = dynarr_get(field_list, 0);

		if (dynarr_size(field_init->namelist) > 0) {
			panic("union does not support named initializer yet");
		}
		cgasm_initialize_global_var(ctx, field->type, field_init);
		specified_size = field->type->size;
	}

	if (specified_size < type->size) {
		cgasm_println(ctx, ".zero %d", type->size - specified_size);
	}
}

static void cgasm_initialize_global_struct(struct cgasm_context *ctx, struct type *type, struct initializer *initializer) {
	assert(type->tag == T_STRUCT);
	assert(type->size >= 0);
	if (initializer->initz_list == NULL) {
		panic("Invalid structure initializer");
	}
	struct dynarr *init_list = initializer->initz_list->list;
	struct dynarr *field_list = type->field_list;
	if (dynarr_size(field_list) < dynarr_size(init_list)) {
		panic("too many fields specified in struct initializer");
	}

	int i;
	int tot_size_inited = 0;
	int accum = 0;
	int start_offset = 0; // this may not be zero for anonymouse struct/union member

	if (dynarr_size(field_list) > 0) {
		start_offset = ((struct struct_field *) dynarr_get(field_list, 0))->offset;
	}

	// XXX alignment is not handled yet
	for (i = 0; i < dynarr_size(init_list); i++) {
		struct initializer *field_init = dynarr_get(init_list, i);

		if (dynarr_size(field_init->namelist) > 0) {
			panic("not support named initializer for struct yet");
		}

		struct struct_field *field = dynarr_get(field_list, i);
		if (field->width != 0) {
			panic("struct field width is not supported yet");
		}
		struct type *field_type = field->type;
		tot_size_inited += field_type->size;
		if (field->offset - start_offset != accum) {
			red("field offset %d name %s, accum %d", field->offset, field->name, accum);
			panic("not support initializer padding in struct yet");
		}
		cgasm_initialize_global_var(ctx, field_type, field_init);
		accum += field_type->size;
	}

	if (tot_size_inited < type->size) {
		cgasm_println(ctx, ".zero %d", type->size - tot_size_inited);
	}
}

static void cgasm_initialize_global_var(struct cgasm_context *ctx, struct type *type, struct initializer *initializer) {
	if (initializer == NULL) {
		if (type->size < 0) {
			panic("type not complete");
		}
		cgasm_println(ctx, ".zero %d", type->size);
		return;
	}

	switch (type->tag) {
	case T_LONG_LONG:
		cgasm_initialize_global_ll(ctx, initializer);
		break;
	case T_INT:
		cgasm_initialize_global_vint(ctx, initializer, ".long");
		break;
	case T_SHORT:
		cgasm_initialize_global_vint(ctx, initializer, ".word");
		break;
	case T_CHAR:
		cgasm_initialize_global_vint(ctx, initializer, ".byte");
		break;
	case T_PTR:
		cgasm_initialize_global_ptr(ctx, type, initializer);
		break;
	case T_ARRAY:
		cgasm_initialize_global_array(ctx, type, initializer);
		break;
	case T_STRUCT:
		cgasm_initialize_global_struct(ctx, type, initializer);
		break;
	case T_UNION:
		cgasm_initialize_global_union(ctx, type, initializer);
		break;
	default:
		cgc_dump(initializer, initializer);
		panic("ni %d", type->tag);
	}
}

void cgasm_allocate_global_var(struct cgasm_context *ctx, struct global_var_symbol *sym, struct initializer *initializer) {
	// no need to allocate space for typedef/extern symbol
	if (sym->flags & (SYMBOL_FLAG_TYPEDEF | SYMBOL_FLAG_EXTERN)) {
		if (initializer != NULL) {
			panic("typedef or extern variable can not have initializer");
		}
		return;
	}

	// no need to allocate space for func declaration
	if (sym->ctype->tag == T_FUNC) {
		return; 
	}

	cgasm_println(ctx, ".data");

	if (!(sym->flags & SYMBOL_FLAG_STATIC)) {
		cgasm_println(ctx, ".global %s", sym->name);
	}

	if (initializer == NULL) {
		cgasm_println(ctx, ".comm %s, %d, %d", sym->name, sym->ctype->size, 4); // XXX hardcode to 4 byte alignment right now
		return;
	}

	// global variable with initializer
	cgasm_println_noind(ctx, "%s:", sym->name);
	cgasm_initialize_global_var(ctx, sym->ctype, initializer);
}

static void cgasm_initialize_local_array(struct cgasm_context *ctx, int base_reg, int offset, struct type *type, struct initializer *initializer) {
	assert(type->tag == T_ARRAY);
	struct dynarr *initz_list = initializer->initz_list->list;
	struct type *subtype = type->subtype;
	int compsize = type_get_size(subtype);

	DYNARR_FOREACH_BEGIN(initz_list, initializer, each);
		cgasm_initialize_local_var(ctx, base_reg, offset, subtype, each);
		offset += compsize;
	DYNARR_FOREACH_END();
}

// handle scalar types like ptr, int, short, byte.
// long long and floating type are nog handled here
static void cgasm_initialize_local_scalar(struct cgasm_context *ctx, int base_reg, int offset, struct type *type, struct initializer *initializer) {
	assert(initializer->expr != NULL);
	struct expr_val val = cgasm_assignment_expression(ctx, initializer->expr);
	val = type_convert(ctx, val, type);
	int reg = find_avail_reg(1 << base_reg);
	cgasm_load_val_to_reg(ctx, val, reg);

	int size = type_get_size(type);
	assert(size <= 4);
	cgasm_println(ctx, "mov%s %%%s, %d(%%%s)", size_to_suffix(size), get_reg_str_code_size(reg, size), offset, get_reg_str_code(base_reg));
}

static void cgasm_initialize_local_with_named_initializer(struct cgasm_context *ctx, int base_reg, int offset, struct type *type, struct initializer *initializer, int nameind) {
	struct dynarr *namelist = initializer->namelist;
	assert(dynarr_size(namelist) > 0);
	if (nameind == dynarr_size(namelist)) {
		// not copy namelist
		struct initializer *copy_initializer = initializer_init();
		copy_initializer->expr = initializer->expr;
		copy_initializer->initz_list = initializer->initz_list;
		cgasm_initialize_local_var(ctx, base_reg, offset, type, initializer);

		dynarr_destroy(copy_initializer->namelist);
		free(copy_initializer);
		return;
	}
	char *name = dynarr_get(namelist, nameind);
	struct struct_field *field = get_struct_field(type, name);
	if (field == NULL) {
		panic("invalid field name %s", name);
	}
	cgasm_initialize_local_with_named_initializer(ctx, base_reg, offset + field->offset, field->type, initializer, nameind + 1);
}

static void cgasm_initialize_local_struct(struct cgasm_context *ctx, int base_reg, int offset, struct type *type, struct initializer *initializer) {
	assert(type->tag == T_STRUCT);
	assert(type->size >= 0);
	if (initializer->initz_list == NULL) {
		panic("Invalid structure initializer");
	}
	struct dynarr *init_list = initializer->initz_list->list;
	struct dynarr *field_list = type->field_list;
	if (dynarr_size(field_list) < dynarr_size(init_list)) {
		panic("too many fields specified in struct initializer");
	}

	int i;
	for (i = 0; i < dynarr_size(init_list); i++) {
		struct initializer *subinit = dynarr_get(init_list, i);
		struct struct_field *field = dynarr_get(field_list, i);
		if (dynarr_size(subinit->namelist) > 0) {
			panic("not support named initializer for struct yet");
		}
		cgasm_initialize_local_var(ctx, base_reg, offset + field->offset, field->type, subinit);
	}
}

static void cgasm_initialize_local_union(struct cgasm_context *ctx, int base_reg, int offset, struct type *type, struct initializer *initializer) {
	assert(type->tag == T_UNION);
	assert(initializer->initz_list != NULL);
	struct dynarr *initz_list = initializer->initz_list->list;
	struct initializer *subini;
	if (dynarr_size(initz_list) == 0) {
		return;
	}

	if (dynarr_size(initz_list) == 1 && dynarr_size((subini = dynarr_get(initz_list, 0))->namelist) == 0) {
		// not named initializer
		struct dynarr *field_list = type->field_list;
		if (dynarr_size(field_list) == 0) {
			panic("too many fields specified in initializer");
		}
		struct struct_field *field = dynarr_get(field_list, 0);
		cgasm_initialize_local_var(ctx, base_reg, offset, field->type, subini);
		return;
	}

	// all named initializer
	DYNARR_FOREACH_BEGIN(initz_list, initializer, each);
		struct dynarr *namelist = each->namelist;
		if (dynarr_size(namelist) == 0) {
			panic("require named initializer");
		}
		cgasm_initialize_local_with_named_initializer(ctx, base_reg, offset, type, each, 0);
	DYNARR_FOREACH_END();
}

static void cgasm_initialize_local_var(struct cgasm_context *ctx, int base_reg, int offset, struct type *type, struct initializer *initializer) {
	switch (type->tag) {
	case T_ARRAY:
		cgasm_initialize_local_array(ctx, base_reg, offset, type, initializer);
		break;
	case T_PTR: case T_INT: case T_SHORT: case T_CHAR:
		cgasm_initialize_local_scalar(ctx, base_reg, offset, type, initializer);
		break;
	case T_STRUCT:
		cgasm_initialize_local_struct(ctx, base_reg, offset, type, initializer);
		break;
	case T_UNION:
		cgasm_initialize_local_union(ctx, base_reg, offset, type, initializer);
		break;
	default:
		panic("local initializer, type %d", type->tag);
	}
}

void cgasm_initialize_local_symbol(struct cgasm_context *ctx, struct local_var_symbol *sym, struct initializer *initializer) {
	assert(initializer->expr == NULL);
	struct type *type = sym->ctype;
	int offset = cgasm_get_local_var_offset(ctx, sym);
	cgasm_clear_range(ctx, REG_EBP, offset, type_get_size(type));

	assert(type->size >= 0);
	int base_reg = REG_EBP;

	cgasm_initialize_local_var(ctx, base_reg, offset, type, initializer);
}

