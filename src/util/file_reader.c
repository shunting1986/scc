#include <inc/file_reader.h>
#include <inc/util.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

struct file_reader *file_reader_init(const char *path) {
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		return NULL;
	}

	struct file_reader *fr = malloc(sizeof(*fr));
	fr->path = strdup(path);
	fr->fd = fd;
	fr->pos = fr->len = 0;
	fr->putback = -1;
	return fr;
}

void file_reader_put_back(struct file_reader *fr, char putback) {
	assert(fr->putback < 0);
	fr->putback = putback;
}

int file_reader_next_char(struct file_reader *fr) {
	if (fr->putback >= 0) {
		char ch = fr->putback;
		fr->putback = -1;
		return ch;
	}

	if (fr->pos >= fr->len) {
		int r = read(fr->fd, fr->buf, sizeof(fr->buf));
		assert(r >= 0);
		if (r > 0) {
			fr->pos = 0;
			fr->len = r;
		}
	}

	if (fr->pos >= fr->len) {
		return EOF;
	} else {
		return fr->buf[fr->pos++];
	}
}

void file_reader_destroy(struct file_reader *fr) {
	free(fr->path);
	close(fr->fd);
	free(fr);
}
