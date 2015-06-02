#include <inc/cbuf.h>
#include <gtest/gtest.h>

TEST(cbuf, cbuf) {
	struct cbuf *buf = cbuf_init();
	cbuf_add(buf, 'h');
	cbuf_add(buf, 'e');
	cbuf_add(buf, 'l');
	cbuf_add(buf, 'l');
	cbuf_add(buf, 'o');
	char *s = cbuf_gets(buf);
	ASSERT_TRUE(s != NULL);
	ASSERT_TRUE(strcmp(s, "hello") == 0);
	
	s = cbuf_transfer(buf);
	ASSERT_TRUE(s != NULL);
	ASSERT_TRUE(strcmp(s, "hello") == 0);

	s = cbuf_gets(buf);
	ASSERT_TRUE(s == NULL);
	cbuf_destroy(buf);
}
