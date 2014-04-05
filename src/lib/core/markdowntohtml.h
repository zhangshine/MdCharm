#ifndef MARKDOWNTOHTML_H
#define MARKDOWNTOHTML_H

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifdef MARKDOWN_LIB
    #define DECLSPEC __declspec(dllexport)
#else
    #define DECLSPEC
#endif

class DECLSPEC MarkdownToHtml
{
public:
    const static int OUTPUT_UNIT = 512;
    enum MarkdownToHtmlResult {
        ERROR = -1,
        NOTHING,
        SUCCESS
    };

    enum MarkdownType {
        Markdown,
        PHPMarkdownExtra,
        MultiMarkdown
    };

public:
    MarkdownToHtml();
    static MarkdownToHtmlResult translateMarkdownToHtml(MarkdownType type,
                                        const std::string &inMarkdown,
                                        std::string &outHtml);
    static MarkdownToHtmlResult translateMarkdownToHtml(MarkdownType type,
                                        const char* data,
                                        const int length, std::string &outHtml);
    static MarkdownToHtmlResult translateMarkdownExtraToHtml(MarkdownType type,
                                        const char* data,
                                        const int length, std::string &outHtml);
    static MarkdownToHtmlResult translateMultiMarkdownToHtml(MarkdownType type,
                                        const char* data,
                                        const int length, std::string &outHtml);
};

#endif // MARKDOWNTOHTML_H
