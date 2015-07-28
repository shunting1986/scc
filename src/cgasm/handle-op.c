#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/lexer.h>
#include <inc/symtab.h>
#include <inc/pp.h>

static void cgasm_load_const_val_to_reg(struct cgasm_context *ctx, union token const_val, int reg);

/**
 * handle unary op, binary op, push op, load op, store op etc.
 */

/**********************/
/* util               */
/**********************/
static int cgasm_get_param_offset(struct cgasm_context *ctx /* unused */, struct param_symbol *sym) {
	return sym->param_ind * 4 + 8;
}

static int cgasm_get_local_var_offset(struct cgasm_context *ctx, struct local_var_symbol *sym) {
	return -((sym->var_ind + ctx->nstate_reg) * 4 + 4);
}

static int cgasm_get_temp_var_offset(struct cgasm_context *ctx, struct temp_var temp) {
	return -((temp.ind + ctx->nstate_reg) * 4 + 4);
}

static void cgasm_get_lval_local_var_asm_code(struct cgasm_context *ctx, struct local_var_symbol *sym, char *buf) {
	assert(buf != NULL);
	sprintf(buf, "%d(%%ebp)", cgasm_get_local_var_offset(ctx, sym));
}

static void cgasm_get_lval_global_var_asm_code(struct cgasm_context *ctx, struct global_var_symbol *sym, char *buf) {
	assert(buf != NULL);
	sprintf(buf, "%s", sym->name);
}

static void cgasm_get_lval_sym_asm_code(struct cgasm_context *ctx, struct symbol *sym, char *buf) {
	assert(buf != NULL);
	switch (sym->type) {
	case SYMBOL_LOCAL_VAR:
		cgasm_get_lval_local_var_asm_code(ctx, (struct local_var_symbol *) sym, buf);
		break;
	case SYMBOL_GLOBAL_VAR:
		cgasm_get_lval_global_var_asm_code(ctx, (struct global_var_symbol *) sym, buf);
		break;
	default:
		panic("ni %d", sym->type);
		break;
	}
}

static void cgasm_get_lval_temp_asm_code(struct cgasm_context *ctx, struct temp_var temp, char *buf) {
	assert(buf != NULL);
	sprintf(buf, "%d(%%ebp)", cgasm_get_temp_var_offset(ctx, temp));
}

// this funtion will report error if val is not an lval.
// we also use this method to get the asm code for temp var
static char *cgasm_get_lval_asm_code(struct cgasm_context *ctx, struct expr_val val, char *buf) {
	if (buf == NULL) {
		buf = mallocz(128); // caller should free it
	}
	switch (val.type) {
	case EXPR_VAL_SYMBOL:
		cgasm_get_lval_sym_asm_code(ctx, val.sym, buf);
		break;
	case EXPR_VAL_TEMP:
		cgasm_get_lval_temp_asm_code(ctx, val.temp_var, buf);
		break;
	default:
		panic("ni 0x%x", val.type);
	}
	return buf;
}

/**********************/
/* load               */
/**********************/
static void cgasm_load_local_var_to_reg(struct cgasm_context *ctx, struct local_var_symbol *sym, int reg) {
	cgasm_println(ctx, "movl %d(%%ebp), %%%s", cgasm_get_local_var_offset(ctx, sym), get_reg_str_code(reg));
}

static void cgasm_load_param_to_reg(struct cgasm_context *ctx, struct param_symbol *sym, int reg) {
	cgasm_println(ctx, "movl %d(%%ebp), %%%s", cgasm_get_param_offset(ctx, sym), get_reg_str_code(reg));
}

static void cgasm_load_global_var_to_reg(struct cgasm_context *ctx, struct global_var_symbol *sym, int reg) {
	cgasm_println(ctx, "movl %s, %%%s", sym->name, get_reg_str_code(reg));
}

static void cgasm_load_global_var_addr_to_reg(struct cgasm_context *ctx, struct global_var_symbol *sym, int reg) {
	cgasm_println(ctx, "movl $%s, %%%s", sym->name, get_reg_str_code(reg));
}

static void cgasm_load_local_var_addr_to_reg(struct cgasm_context *ctx, struct local_var_symbol *sym, int reg) {
	cgasm_println(ctx, "leal %d(%%ebp), %%%s", cgasm_get_local_var_offset(ctx, sym), get_reg_str_code(reg));
}

static void cgasm_load_sym_to_reg(struct cgasm_context *ctx, struct symbol *sym, int reg) {
	switch (sym->type) {
	case SYMBOL_LOCAL_VAR:
		cgasm_load_local_var_to_reg(ctx, (struct local_var_symbol *) sym, reg);
		break;
	case SYMBOL_PARAM:
		cgasm_load_param_to_reg(ctx, (struct param_symbol *) sym, reg);
		break;
	case SYMBOL_GLOBAL_VAR:
		cgasm_load_global_var_to_reg(ctx, (struct global_var_symbol *) sym, reg);
		break;
	case SYMBOL_ENUMERATOR:
		cgasm_load_const_val_to_reg(ctx, wrap_int_const_to_token(((struct enumerator_symbol *) sym)->val), reg);
		break;
	default:
		panic("ni %d %s", sym->type, sym->name);
	}
}

static void cgasm_load_sym_addr_to_reg(struct cgasm_context *ctx, struct symbol *sym, int reg) {
	switch (sym->type) {
	case SYMBOL_GLOBAL_VAR:
		cgasm_load_global_var_addr_to_reg(ctx, (struct global_var_symbol *) sym, reg);
		break;
	case SYMBOL_LOCAL_VAR:
		cgasm_load_local_var_addr_to_reg(ctx, (struct local_var_symbol *) sym, reg);
		break;
	default:
		panic("ni %d %s", sym->type, sym->name);
	}
}

static void cgasm_load_temp_to_reg(struct cgasm_context *ctx, struct temp_var temp, int reg) {
	cgasm_println(ctx, "movl %d(%%ebp), %%%s", cgasm_get_temp_var_offset(ctx, temp), get_reg_str_code(reg));
}

static void cgasm_load_const_val_to_reg(struct cgasm_context *ctx, union token const_val, int reg) {
	int flags = const_val.const_val.flags;
	if (flags & CONST_VAL_TOK_INTEGER) {
		cgasm_println(ctx, "movl $%d, %%%s", const_val.const_val.ival, get_reg_str_code(reg));
	} else if (flags & CONST_VAL_TOK_LONG_LONG) {
		cgasm_emit_abort(ctx); // not support 64 bit yet
	} else {
		panic("ni");
	}
}

/* 
 * TODO current solution is translate the logic as
 *
 * if (expr) {
 *   reg = 1;
 * } else {
 *   reg = 0;
 * }
 *
 * Check conditional code directly by setcc instruction should be more efficient
 */
static void cgasm_load_cc_to_reg(struct cgasm_context *ctx, struct condcode *cc, int reg) {
	int set1_label = cgasm_new_label_no(ctx);
	int out_label = cgasm_new_label_no(ctx);
	char buf[128];

	if (cc->op == TOK_EXCLAMATION) {
		panic("'!' not supported yet");
	}

	cgasm_goto_ifcond_cc(ctx, cc, set1_label, 0);
	cgasm_println(ctx, "movl $0, %%%s", get_reg_str_code(reg));
	cgasm_println(ctx, "jmp %s", get_jump_label_str(out_label, buf));
	cgasm_emit_jump_label(ctx, set1_label);
	cgasm_println(ctx, "movl $1, %%%s", get_reg_str_code(reg));
	cgasm_emit_jump_label(ctx, out_label);
}

static void cgasm_deref_reg(struct cgasm_context *ctx, int reg) {
	cgasm_println(ctx, "movl (%%%s), %%%s", get_reg_str_code(reg), get_reg_str_code(reg));
}

void cgasm_load_val_to_reg(struct cgasm_context *ctx, struct expr_val val, int reg) {
	switch (val.type & ~EXPR_VAL_FLAG_MASK) {
	case EXPR_VAL_SYMBOL_ADDR:
		cgasm_load_sym_addr_to_reg(ctx, val.sym, reg);
		break;
	case EXPR_VAL_SYMBOL:
		cgasm_load_sym_to_reg(ctx, val.sym, reg);
		break;
	case EXPR_VAL_TEMP:
		cgasm_load_temp_to_reg(ctx, val.temp_var, reg);
		break;
	case EXPR_VAL_CONST_VAL:
		cgasm_load_const_val_to_reg(ctx, val.const_val, reg);
		break;
	case EXPR_VAL_CC:
		cgasm_load_cc_to_reg(ctx, val.cc, reg);
		break;
	default:
		panic("ni 0x%x", val.type);
		break;
	}

	if (val.type & EXPR_VAL_FLAG_DEREF) {
		cgasm_deref_reg(ctx, reg);
	}
}

/***********************/
/* store               */
/***********************/
void cgasm_store_reg_to_mem(struct cgasm_context *ctx, int reg, struct expr_val mem) {
	char buf[128];

	cgasm_get_lval_asm_code(ctx, mem, buf);
	cgasm_println(ctx, "movl %%%s, %s", get_reg_str_code(reg), buf);
}

/***********************/
/* push                */
/***********************/
static void cgasm_push_str_literal(struct cgasm_context *ctx, int ind) {
	char buf[256];
	get_str_literal_label(ind, buf);
	cgasm_println(ctx, "pushl $%s", buf);
}

static void cgasm_push_local_var_addr(struct cgasm_context *ctx, struct local_var_symbol *sym) {
	cgasm_println(ctx, "leal %d(%%ebp), %%eax", cgasm_get_local_var_offset(ctx, sym));
	cgasm_println(ctx, "pushl %%eax");
}

static void cgasm_push_global_var_addr(struct cgasm_context *ctx, struct global_var_symbol *sym) {
	cgasm_println(ctx, "pushl $%s", sym->name);
}

static void cgasm_push_sym_addr(struct cgasm_context *ctx, struct symbol *sym) {
	switch (sym->type) {
	case SYMBOL_LOCAL_VAR:
		cgasm_push_local_var_addr(ctx, (struct local_var_symbol *) sym);
		break;
	case SYMBOL_GLOBAL_VAR:
		cgasm_push_global_var_addr(ctx, (struct global_var_symbol *) sym);
		break;
	default:
		panic("ni %d", sym->type);
	}
}

static void cgasm_push_sym(struct cgasm_context *ctx, struct symbol *sym) {
	// load the register first
	int reg = REG_EAX;
	cgasm_load_sym_to_reg(ctx, sym, reg);
	cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg));
}

static void cgasm_push_temp(struct cgasm_context *ctx, struct temp_var temp) {
	int reg = REG_EAX;
	cgasm_load_temp_to_reg(ctx, temp, reg);
	cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg));
}

static void cgasm_push_const_val(struct cgasm_context *ctx, union token const_tok) {
	if (!(const_tok.const_val.flags & CONST_VAL_TOK_INTEGER)) {
		panic("only support integer right now");
	}
	cgasm_println(ctx, "pushl $%d", const_tok.const_val.ival);
}

void cgasm_push_val(struct cgasm_context *ctx, struct expr_val val) {
	if (val.type & EXPR_VAL_FLAG_DEREF) {
		int reg = REG_EAX;
		// load to reg and then push reg
		cgasm_load_val_to_reg(ctx, val, reg);
		cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg));
		return;
	}
	switch (val.type) {
	case EXPR_VAL_SYMBOL_ADDR:
		cgasm_push_sym_addr(ctx, val.sym);
		break;
	case EXPR_VAL_STR_LITERAL:
		cgasm_push_str_literal(ctx, val.ind);
		break;
	case EXPR_VAL_SYMBOL:
		cgasm_push_sym(ctx, val.sym);
		break;
	case EXPR_VAL_TEMP:
		cgasm_push_temp(ctx, val.temp_var);
		break;
	case EXPR_VAL_CONST_VAL:
		cgasm_push_const_val(ctx, val.const_val);
		break;
	default:
		panic("ni 0x%x", val.type);
	}
}

/***********************/
/* unary op            */
/***********************/
struct expr_val cgasm_handle_ampersand(struct cgasm_context *ctx, struct expr_val operand) {
	if (operand.type & EXPR_VAL_FLAG_DEREF) {
		operand.type &= ~EXPR_VAL_FLAG_DEREF;
		return operand;
	}

	if (operand.type != EXPR_VAL_SYMBOL) {
		panic("'&' can only be applied to lval 0x%x", operand.type);
	}
	operand.type = EXPR_VAL_SYMBOL_ADDR;
	struct type *basetype = operand.ctype;
	operand.ctype = get_ptr_type(basetype);
	register_type_ref(ctx, operand.ctype);
	// type_put(basetype); // this is not needed
	return operand;
}

struct expr_val cgasm_handle_negate(struct cgasm_context *ctx, struct expr_val operand) {
	// special case for constant
	if (operand.type == EXPR_VAL_CONST_VAL) {
		assert(operand.const_val.const_val.flags & CONST_VAL_TOK_INTEGER);
		operand.const_val.const_val.ival = -operand.const_val.const_val.ival;
		return operand;
	} else {
		int reg = REG_EAX;
		struct expr_val temp = cgasm_alloc_temp_var(ctx);
		cgasm_load_val_to_reg(ctx, operand, reg);
		cgasm_println(ctx, "negl %%%s", get_reg_str_code(reg));
		cgasm_store_reg_to_mem(ctx, reg, temp);
		return temp;
	}
}

static struct expr_val cgasm_handle_deref(struct cgasm_context *ctx, struct expr_val operand) {
	// TODO type memory management
	if (!expr_val_has_deref_flag(operand) && type_get_tag(operand.ctype) == T_PTR) {
		return expr_val_add_deref_flag(operand);
	}
	panic("only support ptr right now");
}

struct expr_val cgasm_handle_unary_op(struct cgasm_context *ctx, int tok_tag, struct expr_val operand) {
	switch (tok_tag) {
	case TOK_AMPERSAND:
		return cgasm_handle_ampersand(ctx, operand);
	case TOK_SUB:
		return cgasm_handle_negate(ctx, operand);
	case TOK_STAR:
		return cgasm_handle_deref(ctx, operand);
	case TOK_EXCLAMATION:	// optimized for some cases like 'if(!x) { }'
		return condcode_expr(TOK_EXCLAMATION, operand, void_expr_val(), NULL);
	default:
		panic("ni %s", token_tag_str(tok_tag));
	}
}

/***********************/
/* binary op           */
/***********************/
// for logic and/or
struct expr_val cgasm_handle_binary_op_lazy(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct syntreebasenode *rhs) {
	assert(tok_tag == TOK_LOGIC_AND || tok_tag == TOK_LOGIC_OR);
	return condcode_expr(tok_tag, lhs, void_expr_val(), rhs);
}

// XXX: not support floating point right now
static struct expr_val cgasm_handle_binary_op_const(int tok_op, struct expr_val lhs, struct expr_val rhs) {
	assert(is_int_const(lhs));
	assert(is_int_const(rhs));
	int val = perform_int_bin_op(lhs.const_val.const_val.ival, rhs.const_val.const_val.ival, tok_op);
	return int_const_expr_val(val);
}

struct expr_val cgasm_handle_binary_op(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct expr_val rhs) {
	int lhs_reg = REG_EAX; // REVISE MUL if we change this register
	int rhs_reg = REG_ECX;
	struct expr_val res;
#define LOAD_TO_REG() do { \
	cgasm_load_val_to_reg(ctx, lhs, lhs_reg); \
	cgasm_load_val_to_reg(ctx, rhs, rhs_reg); \
} while (0)

// note: the result is stored in the lhs register 
#define STORE_TO_TEMP() do { \
	res = cgasm_alloc_temp_var(ctx); \
	cgasm_store_reg_to_mem(ctx, lhs_reg, res); \
} while (0)

	if (lhs.type == EXPR_VAL_CONST_VAL && rhs.type == EXPR_VAL_CONST_VAL) {
		return cgasm_handle_binary_op_const(tok_tag, lhs, rhs);
	}

	switch (tok_tag) {
	case TOK_ADD: 
		LOAD_TO_REG();
		cgasm_println(ctx, "addl %%%s, %%%s", get_reg_str_code(rhs_reg), get_reg_str_code(lhs_reg));
		STORE_TO_TEMP();
		break;
	case TOK_AMPERSAND: 
		LOAD_TO_REG();
		cgasm_println(ctx, "andl %%%s, %%%s", get_reg_str_code(rhs_reg), get_reg_str_code(lhs_reg));
		STORE_TO_TEMP();
		break;
	case TOK_VERT_BAR:
		LOAD_TO_REG();
		cgasm_println(ctx, "orl %%%s, %%%s", get_reg_str_code(rhs_reg), get_reg_str_code(lhs_reg));
		STORE_TO_TEMP();
		break;
	case TOK_SUB:
		LOAD_TO_REG();
		cgasm_println(ctx, "subl %%%s, %%%s", get_reg_str_code(rhs_reg), get_reg_str_code(lhs_reg));
		STORE_TO_TEMP();
		break;
	case TOK_STAR:
		LOAD_TO_REG(); 
		cgasm_println(ctx, "mull %%%s", get_reg_str_code(rhs_reg));
		STORE_TO_TEMP();
		break;
	case TOK_LSHIFT:
		LOAD_TO_REG();
		assert(rhs_reg == REG_ECX);
		cgasm_println(ctx, "shll %%cl, %%%s", get_reg_str_code(lhs_reg));
		STORE_TO_TEMP();
		break;
	case TOK_RSHIFT:
		LOAD_TO_REG();
		assert(rhs_reg == REG_ECX);
		cgasm_println(ctx, "shrl %%cl, %%%s", get_reg_str_code(lhs_reg));
		STORE_TO_TEMP();
		break;
	case TOK_EQ: case TOK_NE: case TOK_LE: case TOK_GT: case TOK_LT: case TOK_GE:
		res = condcode_expr(tok_tag, lhs, rhs, NULL);
		break;
	default:
		panic("ni %s", token_tag_str(tok_tag));
	}

#undef LOAD_TO_REG
#undef STORE_TO_TEMP
	return res;
}

/*******************************/
/* assignment                  */
/*******************************/
struct expr_val cgasm_handle_assign_op(struct cgasm_context *ctx, struct expr_val lhs, struct expr_val rhs, int op) {
	int rhs_reg = REG_EAX;
	char buf[128];
	cgasm_load_val_to_reg(ctx, rhs, rhs_reg);

	if (lhs.type & EXPR_VAL_FLAG_DEREF) {
		int lhs_reg = REG_ECX;

		lhs.type &= ~EXPR_VAL_FLAG_DEREF; // clear the flag so that we can get the addr
		cgasm_load_val_to_reg(ctx, lhs, lhs_reg);
		lhs.type |= EXPR_VAL_FLAG_DEREF; 
		cgasm_println(ctx, "movl %%%s, (%%%s)", get_reg_str_code(rhs_reg), get_reg_str_code(lhs_reg));
		return lhs;
	}

	switch (op) {
	case TOK_ASSIGN:
		cgasm_println(ctx, "movl %%%s, %s", get_reg_str_code(rhs_reg), cgasm_get_lval_asm_code(ctx, lhs, buf));
		break;
	case TOK_ADD_ASSIGN:
		cgasm_println(ctx, "addl %%%s, %s", get_reg_str_code(rhs_reg), cgasm_get_lval_asm_code(ctx, lhs, buf));
		break;
	default:
		panic("ni %s", token_tag_str(op));
	}
	return lhs;
}

/********************************/
/* return op                    */
void cgasm_handle_ret(struct cgasm_context *ctx) {
	int i;
	cgasm_println(ctx, "leal -%d(%%ebp), %%esp", ctx->nstate_reg * 4);
	for (i = ctx->nstate_reg - 1; i >= 0; i--) {
		cgasm_println(ctx, "pop %%%s", get_reg_str_code(ctx->state_reg[i]));
	}
	cgasm_println(ctx, "pop %%ebp");
	cgasm_println(ctx, "ret");
}

/*******************************/
/* check truth                 */
/*******************************/
void cgasm_test_expr(struct cgasm_context *ctx, struct expr_val val) {
	// XXX don't need this yet
	panic("ni");
}

/*******************************/
/* inc/dec                     */
/*******************************/
static struct expr_val cgasm_handle_post_incdec(struct cgasm_context *ctx, struct expr_val val, int is_inc) {
	int reg = REG_EAX;
	struct expr_val temp_var = cgasm_alloc_temp_var(ctx);

	cgasm_load_val_to_reg(ctx, val, reg);
	cgasm_store_reg_to_mem(ctx, reg, temp_var);
	cgasm_println(ctx, "%s %%%s", is_inc ? "incl" : "decl", get_reg_str_code(reg));
	cgasm_store_reg_to_mem(ctx, reg, val);
	return temp_var;
}

static struct expr_val cgasm_handle_pre_incdec(struct cgasm_context *ctx, struct expr_val val, int is_inc) {
	int reg = REG_EAX;

	cgasm_load_val_to_reg(ctx, val, reg);
	cgasm_println(ctx, "%s %%%s", is_inc ? "incl" : "decl", get_reg_str_code(reg));
	cgasm_store_reg_to_mem(ctx, reg, val);
	return val;

}

struct expr_val cgasm_handle_pre_inc(struct cgasm_context *ctx, struct expr_val val) {
	return cgasm_handle_pre_incdec(ctx, val, true);
}

struct expr_val cgasm_handle_pre_dec(struct cgasm_context *ctx, struct expr_val val) {
	return cgasm_handle_pre_incdec(ctx, val, false);
}

struct expr_val cgasm_handle_post_inc(struct cgasm_context *ctx, struct expr_val val) {
	return cgasm_handle_post_incdec(ctx, val, 1);
}

struct expr_val cgasm_handle_post_dec(struct cgasm_context *ctx, struct expr_val val) {
	return cgasm_handle_post_incdec(ctx, val, 0);
}

/**********************************/
/* index op                       */
/**********************************/
struct expr_val cgasm_handle_index_op(struct cgasm_context *ctx, struct expr_val base_val, struct expr_val ind_val) {
	struct type *parent_type = expr_val_get_type(base_val);
	struct type *elemtype = type_get_elem_type(parent_type);
	int elemsize = type_get_size(elemtype);

	// this is special for array symbol
	if (base_val.type == EXPR_VAL_SYMBOL && base_val.sym->ctype->tag == T_ARRAY) {
		base_val.type = EXPR_VAL_SYMBOL_ADDR;
	} else if (parent_type->tag == T_ARRAY && expr_val_has_deref_flag(base_val)) {
		base_val = expr_val_remove_deref_flag(base_val);
	}

	// TODO can optimize for power of 2
	// struct expr_val offset_val = cgasm_handle_binary_op(ctx, TOK_LSHIFT, ind_val, const_expr_val(wrap_int_const_to_token(2))); 
	struct expr_val offset_val = cgasm_handle_binary_op(ctx, TOK_STAR, ind_val, const_expr_val(wrap_int_const_to_token(elemsize))); 

	struct expr_val result_val = cgasm_handle_binary_op(ctx, TOK_ADD, base_val, offset_val);

	// handle expression type 
	// TODO reclaim type memory
	result_val.ctype = get_ptr_type(elemtype);
	register_type_ref(ctx, result_val.ctype); // register the type reference so that we can later release it
	return expr_val_add_deref_flag(result_val);
}

/**********************************/
/* conditional op                 */
/**********************************/
struct expr_val cgasm_handle_conditional(struct cgasm_context *ctx, struct expr_val cond, struct dynarr *inner_expr_list, int inner_expr_ind, struct dynarr *or_expr_list, int or_expr_ind, struct expr_val temp_var) {
	if (inner_expr_ind == dynarr_size(inner_expr_list)) {
		assert(or_expr_ind == dynarr_size(or_expr_list));
		
		return cgasm_handle_assign_op(ctx, temp_var, cond, TOK_ASSIGN);
	}

	// similar to if-else
	int else_label = cgasm_new_label_no(ctx);
	int exit_label = cgasm_new_label_no(ctx);
	char buf[128];

	cgasm_goto_ifcond(ctx, cond, else_label, true);
	struct expr_val inner_val = cgasm_expression(ctx, dynarr_get(inner_expr_list, inner_expr_ind));
	cgasm_handle_assign_op(ctx, temp_var, inner_val, TOK_ASSIGN);
	cgasm_println(ctx, "jmp %s", get_jump_label_str(exit_label, buf));
	cgasm_emit_jump_label(ctx, else_label);

	struct expr_val sub_cond = cgasm_logical_or_expression(ctx, dynarr_get(or_expr_list, or_expr_ind));
	cgasm_handle_conditional(ctx, sub_cond, inner_expr_list, inner_expr_ind + 1, or_expr_list, or_expr_ind + 1, temp_var);

	cgasm_emit_jump_label(ctx, exit_label);

	return temp_var;
}

/**************************/
/* handle ptr op          */
/**************************/
struct expr_val cgasm_handle_ptr_op(struct cgasm_context *ctx, struct expr_val stptr, const char *name) {
	struct type *stptr_type = expr_val_get_type(stptr);
	if (stptr_type->tag != T_PTR) {
		panic("struct pointer required");
	}
	struct type *st_type = stptr_type->subtype;
	assert(st_type != NULL && (st_type->tag == T_STRUCT || st_type->tag == T_UNION)); 
	struct struct_field *field = get_struct_field(st_type, name);
	if (field == NULL) {
		panic("invalid struct field %s", name);
	}
	assert(field->offset >= 0);

	// TODO can be optimized for union
	int ptr_reg = REG_EAX;
	cgasm_load_val_to_reg(ctx, stptr, ptr_reg);
	cgasm_println(ctx, "addl $%d, %%%s", field->offset, get_reg_str_code(ptr_reg));
	struct expr_val temp_var = cgasm_alloc_temp_var(ctx);
	cgasm_store_reg_to_mem(ctx, ptr_reg, temp_var);

	struct type *field_ptr_type = get_ptr_type(field->type);
	register_type_ref(ctx, field_ptr_type);

	temp_var.ctype = field_ptr_type;
	return expr_val_add_deref_flag(temp_var);
}




