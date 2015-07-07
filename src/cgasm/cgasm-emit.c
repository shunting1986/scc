#include <inc/util.h>
#include <inc/cgasm.h>

#define DEBUG 1

static void cgasm_vprintln(struct cgasm_context *ctx, const char *fmt, va_list va);
static void cgasm_func_vprintln(struct cgasm_func_context *ctx, const char *fmt, va_list va);

void cgasm_println(struct cgasm_context *ctx, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);

	cgasm_vprintln(ctx, fmt, va);	

	va_end(va);
}

static void cgasm_vprintln(struct cgasm_context *ctx, const char *fmt, va_list va) {
	if (ctx->func_ctx == NULL) {
		vfprintf(ctx->fp, fmt, va);
		fprintf(ctx->fp, "\n");
	} else {
		cgasm_func_vprintln(ctx->func_ctx, fmt, va);
	}
}

char line[512];
static void cgasm_func_vprintln(struct cgasm_func_context *ctx, const char *fmt, va_list va) {
	vsnprintf(line, sizeof(line), fmt, va);
	if (DEBUG) {
		printf("[DEBUG] ");
		printf("%s", line);
		printf("\n");
	}
	cbuf_add_str(ctx->code_buf, line);
	cbuf_add(ctx->code_buf, '\n');
}


