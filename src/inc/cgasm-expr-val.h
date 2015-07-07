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
};

struct temp_var {
	int ind; // use (anonymous) local variable as temporary variable right now
		// may optimize to use register in future
};

enum {
	EXPR_VAL_TEMP,
	EXPR_VAL_SYMBOL,
	EXPR_VAL_SYMBOL_ADDR,
	EXPR_VAL_VOID,
	EXPR_VAL_REGISTER, //
	EXPR_VAL_STR_LITERAL,
};

struct expr_val {
	int type;
	union {
		struct symbol *sym;
		struct temp_var temp_var;
		int ind; // for string literal
	};
};

struct expr_val str_literal_expr_val(int ind);
struct expr_val symbol_expr_val(struct symbol *sym);
struct expr_val cgasm_alloc_temp_var(struct cgasm_context *ctx);
void load_val_to_reg(struct expr_val val, int reg);
void store_reg_to_mem(int reg, struct expr_val mem);

#ifdef __cplusplus
}
#endif

#endif

