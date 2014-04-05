TEMPLATE = app

CONFIG += warn_off

QCMAKE_CXXFLAGS += -fPIC

# Generate markdown_parser.c
markdown_parser_c.target = markdown_parser.c
win32-msvc*: markdown_parser_c.commands = markdown_parser.bat
unix: markdown_parser_c.commands = $$PWD/markdown_parser.sh
markdown_parser_c.depends = markdown_parser_c_nonexist
markdown_parser_c.CONFIG += recursive

markdown_parser_c_nonexist.commands = @echo generating markdown_parser.c
QMAKE_EXTRA_TARGETS += markdown_parser_c markdown_parser_c_nonexist
PRE_TARGETDEPS += markdown_parser.c
# end generation

win32-msvc*: {
    CONFIG += warn_off
    INCLUDEPATH += win
    SOURCES += win/getopt.c
    HEADERS +=  win/getopt.h \
                win/libgen.h \
                win/stdbool.h \
                win/unistd.h
}

HEADERS +=  glib.h \
            GLibFacade.h \
            markdown_lib.h \
            markdown_peg.h \
            MarkdownMacPrefix.h \
            parsing_functions.h \
            utility_functions.h \
            odf.h

SOURCES +=  GLibFacade.c \
            markdown_lib.c \
            markdown_output.c \
            markdown_parser.c \
            parsing_functions.c \
            utility_functions.c \
            odf.c \
            main.c
