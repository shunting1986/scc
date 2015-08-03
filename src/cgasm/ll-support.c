#include <inc/ll-support.h>
#include <inc/util.h>
#include <inc/symtab.h>
#include <inc/cgasm.h>

// TODO use string operation as a optimization
static void cgasm_push_bytes(struct cgasm_context *ctx, int from_base_reg, int from_start_off, int size) {
	int end = from_start_off + size;
	int off;
	int reg = REG_EAX;
	for (off = from_start_off; off < end; off += 4) {
		cgasm_println(ctx, "movl %d(%%%s), %%%s", off, get_reg_str_code(from_base_reg), get_reg_str_code(reg));
		cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg));
	}
}

static void cgasm_load_sym_ll_to_reg2(struct cgasm_context *ctx, struct symbol *sym, int reg1, int reg2) {
	int offset;
	switch (sym->type) {
	case SYMBOL_LOCAL_VAR:
		offset = cgasm_get_local_var_offset(ctx, (struct local_var_symbol *) sym);
		cgasm_push_bytes(ctx, REG_EBP, offset, 8);
		break;
	default:	
		panic("ni %d", sym->type);
	}
}

void cgasm_push_sym_ll(struct cgasm_context *ctx, struct symbol *sym) {
	struct type *type = sym->ctype;
	assert(type->tag == T_LONG_LONG);
	int reg1 = REG_EAX, reg2 = REG_EDX;
	cgasm_load_sym_ll_to_reg2(ctx, sym, reg1, reg2);

	cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg2));	
	cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg1));
}
