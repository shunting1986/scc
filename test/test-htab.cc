#include <inc/htab.h>
#include <inc/util.h>
#include <gtest/gtest.h>

TEST(htab, htab) {
	struct hashtab *htab = htab_init();

	void *ans = htab_query(htab, "abc");
	ASSERT_TRUE(ans == NULL);

	htab_insert(htab, "abc", strdup("def"));
	ans = htab_query(htab, "abc");
	ASSERT_TRUE(ans != NULL);
	ASSERT_TRUE(strcmp("def", (const char *) ans) == 0);

	int r = htab_delete(htab, "abc");
	ASSERT_TRUE(r > 0);
	ans = htab_query(htab, "abc");
	ASSERT_TRUE(ans == NULL);

	htab_destroy(htab);
}
