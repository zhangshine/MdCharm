#include <string>
#include <stdio.h>

#include "codesyntaxhighlighter.h"
#include "highlighter.h"
#include "buffer.h"

using namespace std;

void highlighter(struct buf *ob, const char *name, int len, const char *code, int codeLen)
{
    CodeSyntaxHighlighter highlighter;
    const string& result = highlighter.highlight(name, len, code, codeLen);
    bufput(ob, (const void*)result.c_str(), result.length());
}
