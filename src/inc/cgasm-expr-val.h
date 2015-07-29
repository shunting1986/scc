#ifndef _INC_CGASM_EXPR_VAL_H
#define _INC_CGASM_EXPR_VAL_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
	REG_EAX,
	REG_ECX,
	REG_EDX,
	REG_EBX,
	REG_ESI,
	REG_EDI,
	REG_ESP,
	REG_EDP,
	REG_NUM,
};

struct temp_var {
	int ind; // use (anonymous) local variable as temporary variable right now
		// may optimize to use register in future
};

// add a flag is better than add a extra field in expr_val
#define EXPR_VAL_FLAG_DEREF (1 << 30)
#define EXPR_VAL_FLAG_MASK (0xff << 24)

enum {
	EXPR_VAL_TEMP,
	EXPR_VAL_SYMBOL,
	EXPR_VAL_SYMBOL_ADDR,
	EXPR_VAL_VOID,
	EXPR_VAL_REGISTER, //
	EXPR_VAL_STR_LITERAL,
	EXPR_VAL_CONST_VAL,
	EXPR_VAL_CC, // conditional code, should delay the processing
};

struct condcode;

struct expr_val {
	int type;

	// We store the type without deref being handled here
	struct type *ctype;
	union {
		struct symbol *sym;
		struct temp_var temp_var;
		int ind; // for string literal
		union token const_val;
		struct condcode *cc;
	};
};

struct condcode {
	int op; // TOK_EQ, TOK_NE, TOK_LE, TOK_GE, TOK_GT, TOK_LT, TOK_LOGIC_AND, TOK_EXCLAMATION ...
	struct expr_val lhs, rhs;
	struct syntreebasenode *rhs_lazy;
};

struct expr_val str_literal_expr_val(int ind);
struct expr_val symbol_expr_val(struct symbol *sym);
struct expr_val cgasm_alloc_temp_var(struct cgasm_context *ctx, struct type *ctype);
struct expr_val const_expr_val(union token tok);
struct expr_val int_const_expr_val(int val);
bool is_int_const(struct expr_val val);
struct expr_val void_expr_val();
struct expr_val condcode_expr(int op, struct expr_val lhs, struct expr_val rhs, struct syntreebasenode *rhs_lazy);
const char *get_reg_str_code(unsigned int reg);
struct expr_val expr_val_add_deref_flag(struct expr_val val);
struct type *expr_val_get_type(struct expr_val val);
int expr_val_get_elem_size(struct expr_val val);
int expr_val_has_deref_flag(struct expr_val val);
struct expr_val expr_val_remove_deref_flag(struct expr_val val);

#ifdef __cplusplus
}
#endif

#endif

