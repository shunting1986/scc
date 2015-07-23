#ifndef _INC_FILE_READER_H
#define _INC_FILE_READER_H

#ifdef __cplusplus
extern "C" {
#endif

struct file_reader {
	char *path;
	int fd;

	int putback;

	char buf[256];
	int pos;
	int len;

	struct file_reader *prev;
};

struct file_reader *file_reader_init(const char *path);
int file_reader_next_char(struct file_reader *fr);
void file_reader_destroy(struct file_reader *fr);
void file_reader_put_back(struct file_reader *fr, char putback);
void file_reader_dump_remaining(struct file_reader *fr);

#ifdef __cplusplus
}
#endif

#endif
