#ifndef __INC_KEYWORD_H
#define __INC_KEYWORD_H

#ifdef __cplusplus
extern "C" {
#endif

struct hashtab;

int check_keyword_token(char *s);
const char *keyword_str(int tok_tag);
const char *keyword_str_noabort(int tok_tag);
struct hashtab *setup_keyword_hashtab(const char *list[], int size);

#ifdef __cplusplus
}
#endif

#endif
