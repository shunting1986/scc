#include <inc/cgasm.h>

// TODO use string operation as a optimization
void cgasm_push_bytes(struct cgasm_context *ctx, int from_base_reg, int from_start_off, int size) {
	int end = from_start_off + (size + 3) / 4 * 4;
	int off;
	int reg = from_base_reg != REG_EAX ? REG_EAX : REG_ECX;

	for (off = end - 4; off >= from_start_off; off -= 4) {
		cgasm_println(ctx, "movl %d(%%%s), %%%s", off, get_reg_str_code(from_base_reg), get_reg_str_code(reg));
		cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg));
	}
}

// TODO use string op
void cgasm_copy_bytes(struct cgasm_context *ctx, int from_base_reg, int from_start_off, int to_base_reg, int to_start_off, int size) {
	int reg_mask = (1 << from_base_reg) | (1 << to_base_reg);
	int temp_reg = find_avail_reg(reg_mask);
	int off;

	for (off = 0; off + 3 < size; off += 4) {
		cgasm_println(ctx, "movl %d(%%%s), %%%s", from_start_off + off, get_reg_str_code(from_base_reg), get_reg_str_code(temp_reg));
		cgasm_println(ctx, "movl %%%s, %d(%%%s)", get_reg_str_code(temp_reg), to_start_off + off, get_reg_str_code(to_base_reg));
	}

	if (off < size) { // XXX handle the case that the left number if not power of 2
		int left = size - off;

		if ((left & (-left)) == left) {
			cgasm_println(ctx, "mov%s %d(%%%s), %%%s", size_to_suffix(left), from_start_off + off, get_reg_str_code(from_base_reg), get_reg_str_code_size(temp_reg, left));
			cgasm_println(ctx, "mov%s %%%s, %d(%%%s)", size_to_suffix(left), get_reg_str_code_size(temp_reg, left), to_start_off + off, get_reg_str_code(to_base_reg));
		} else {
			// copy byte by byte
			int i;
			for (i = 0; i < left; i++) {
				cgasm_println(ctx, "movb %d(%%%s), %%%s", from_start_off + off + i, get_reg_str_code(from_base_reg), get_reg_str_code_size(temp_reg, 1));
				cgasm_println(ctx, "movb %%%s, %d(%%%s)", get_reg_str_code_size(temp_reg, 1), to_start_off + off + i, get_reg_str_code(to_base_reg));
			}
		}
	}
}

void cgasm_copy_bytes_to_temp(struct cgasm_context *ctx, int from_base_reg, int from_start_off, struct expr_val temp) {
	assert(temp.type == EXPR_VAL_TEMP);
	struct type *type = expr_val_get_type(temp);
	cgasm_copy_bytes(ctx, from_base_reg, from_start_off, REG_EBP, cgasm_get_temp_var_offset(ctx, temp.temp_var), type_get_size(type));
}


