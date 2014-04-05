win32-msvc*: {
    CONFIG += warn_off
    INCLUDEPATH += $$PWD/win
    SOURCES += $$PWD/win/getopt.c
    HEADERS +=  $$PWD/win/getopt.h \
                $$PWD/win/libgen.h \
                $$PWD/win/stdbool.h \
                $$PWD/win/unistd.h
}
win32{
HEADERS +=  $$PWD/glib.h \
            $$PWD/GLibFacade.h \
}
HEADERS +=  $$PWD/markdown_lib.h \
            $$PWD/markdown_peg.h \
            $$PWD/MarkdownMacPrefix.h \
            $$PWD/parsing_functions.h \
            $$PWD/utility_functions.h \
            $$PWD/odf.h
win32: SOURCES += $$PWD/GLibFacade.c

SOURCES +=  $$PWD/markdown_lib.c \
            $$PWD/markdown_output.c \
            $$PWD/markdown_parser.c \
            $$PWD/parsing_functions.c \
            $$PWD/utility_functions.c \
            $$PWD/odf.c \
            $$PWD/main.c
