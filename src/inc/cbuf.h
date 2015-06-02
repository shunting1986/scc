#ifndef _INC_CBUF_H
#define _INC_CBUF_H

#ifdef __cplusplus
extern "C" {
#endif

struct cbuf;

struct cbuf *cbuf_init();
void cbuf_add(struct cbuf *buf, char ch);
char *cbuf_transfer(struct cbuf *buf);
char *cbuf_gets(struct cbuf *buf);
void cbuf_destroy(struct cbuf *buf);

#ifdef __cplusplus
}
#endif

#endif
