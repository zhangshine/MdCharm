win32-msvc*: {
    CONFIG += warn_off
    INCLUDEPATH += $$PWD/win
    SOURCES += $$PWD/win/getopt.c \
                $$PWD/win/GLibFacade.c

    HEADERS +=  $$PWD/win/getopt.h \
                $$PWD/win/libgen.h \
                $$PWD/win/stdbool.h \
                $$PWD/win/unistd.h \
                $$PWD/win/glib.h \
                $$PWD/win/GLibFacade.h
} else {
    QT_CONFIG -= no-pkg-config
    CONFIG += link_pkgconfig
    PKGCONFIG += glib-2.0
}

HEADERS +=  $$PWD/markdown_lib.h \
            $$PWD/markdown_peg.h \
            $$PWD/MarkdownMacPrefix.h \
            $$PWD/parsing_functions.h \
            $$PWD/utility_functions.h \
            $$PWD/odf.h

SOURCES +=  $$PWD/markdown_lib.c \
            $$PWD/markdown_output.c \
            $$PWD/markdown_parser.c \
            $$PWD/parsing_functions.c \
            $$PWD/utility_functions.c \
            $$PWD/odf.c 
