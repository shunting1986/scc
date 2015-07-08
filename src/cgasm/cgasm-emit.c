#include <inc/util.h>
#include <inc/cgasm.h>

// #define DEBUG 1
#define DEBUG 0

static void cgasm_vprintln(struct cgasm_context *ctx, int ind, const char *fmt, va_list va);
static void cgasm_func_vprintln(struct cgasm_func_context *ctx, int ind, const char *fmt, va_list va);

void cgasm_println(struct cgasm_context *ctx, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);

	cgasm_vprintln(ctx, 1, fmt, va);	

	va_end(va);
}

void cgasm_println_noind(struct cgasm_context *ctx, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);

	cgasm_vprintln(ctx, 0, fmt, va);	

	va_end(va);
}

static void cgasm_vprintln(struct cgasm_context *ctx, int ind, const char *fmt, va_list va) {
	if (ctx->func_ctx == NULL) {
		if (ind) {
			fprintf(ctx->fp, "  ");
		}
		vfprintf(ctx->fp, fmt, va);
		fprintf(ctx->fp, "\n");
	} else {
		cgasm_func_vprintln(ctx->func_ctx, ind, fmt, va);
	}
}

char line[512];
static void cgasm_func_vprintln(struct cgasm_func_context *ctx, int ind, const char *fmt, va_list va) {
	vsnprintf(line, sizeof(line), fmt, va);
	if (DEBUG) {
		printf("[DEBUG] ");
		printf("%s", line);
		printf("\n");
	}
	if (ind) {
		cbuf_add_str(ctx->code_buf, "  "); // indent
	}
	cbuf_add_str(ctx->code_buf, line);
	cbuf_add(ctx->code_buf, '\n');
}


