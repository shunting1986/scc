#include <stdlib.h>
#include <assert.h>

#include <inc/cbuf.h>

struct cbuf {
	char *buf;
	int size; // not including the '\0'
	int capa;
};

struct cbuf *cbuf_init() {
	struct cbuf *buf = calloc(1, sizeof(*buf));
	buf->buf = malloc(8);
	buf->capa = 8;
	return buf;
}

// assume internal buffer is not null
void cbuf_add(struct cbuf *buf, char ch) {
	assert(buf->buf != NULL);
	assert(buf->capa > 0);
	assert(buf->size < buf->capa);

	if (buf->size + 1 == buf->capa) {
		buf->buf = realloc(buf->buf, buf->capa * 2);
		buf->capa <<= 1;
	}
	buf->buf[buf->size++] = ch;
}

// after transferring, the internal reference to the char array is given up
char *cbuf_transfer(struct cbuf *buf) {
	char *ret = buf->buf;
	buf->buf = NULL;
	if (ret != NULL) {
		ret[buf->size] = '\0';
	}
	return ret;
}

char *cbuf_gets(struct cbuf *buf) {
	char *ret = buf->buf;
	if (ret != NULL) {
		ret[buf->size] = '\0';
	}
	return ret;
}

void cbuf_destroy(struct cbuf *buf) {
	if (buf->buf) {
		free(buf->buf);
	}
	free(buf);
}
