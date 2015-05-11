#include <inc/file_reader.h>
#include <gtest/gtest.h>

TEST(file_reader, file_reader) {
	struct file_reader *fr = file_reader_init("test/data/greeting");
	char buf[256];
	int n = 0;
	int r;
	while (n < sizeof(buf) - 1 && (r = file_reader_next_char(fr)) != EOF) {
		buf[n++] = (char) r;
	}
	buf[n] = '\0';
	const char *msg = "Hello, World!\n";
	// printf("msg [%s], buf [%s]\n", msg, buf);
	ASSERT_TRUE(strcmp(msg, buf) == 0);
}
