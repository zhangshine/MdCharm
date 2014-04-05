#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

struct buf;

#ifdef __cplusplus
extern "C" {
#endif

void highlighter(struct buf *ob, const char *name, int len, const char *code, int codeLen);

#ifdef __cplusplus
}
#endif

#endif // HIGHLIGHTER_H
