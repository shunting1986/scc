#include <inc/file_reader.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>

struct file_reader {
	char *path;
	int fd;

	char buf[256];
	int pos;
	int len;
};

struct file_reader *file_reader_init(const char *path) {
	struct file_reader *fr = malloc(sizeof(*fr));
	fr->path = strdup(path);
	fr->fd = open(path, O_RDONLY);
	assert(fr->fd >= 0);
	fr->pos = fr->len = 0;
	return fr;
}

int file_reader_next_char(struct file_reader *fr) {
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
