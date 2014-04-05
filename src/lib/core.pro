TEMPLATE = lib

TARGET = core

win32: DEFINES == _EXPORTING

CONFIG += warn_off

unix: CONFIG += static

markdown_parser_c.target = $$PWD/multimarkdown/src/markdown_parser.c
win32-msvc*: markdown_parser_c.commands = cmd /c $$PWD/multimarkdown/src/markdown_parser.bat
unix: markdown_parser_c.commands = $$PWD/multimarkdown/src/markdown_parser.sh
markdown_parser_c.depends = markdown_parser_c_nonexist
markdown_parser_c.CONFIG += recursive

markdown_parser_c_nonexist.commands = @echo generating markdown_parser.c
QMAKE_EXTRA_TARGETS += markdown_parser_c markdown_parser_c_nonexist
PRE_TARGETDEPS += $$PWD/multimarkdown/src/markdown_parser.c

#For pcre
INCLUDEPATH += $$PWD/pcre
CONFIG(debug, debug|release){
    LIBS += -L../debug -lmdcharm_pcre
} else {
    LIBS += -L../release -lmdcharm_pcre
}

#end pcre

CONFIG(release, debug|release){
    DESTDIR = ../release
} else {
    DESTDIR = ../debug
}


include(multimarkdown/src/multimarkdown_lib.pri)
include(../lib/markdown/markdown.pro)

INCLUDEPATH += multimarkdown/src

HEADERS += \
    core/markdowntohtml.h \
    core/languagedefinationxmlparser.h \
    core/highlighter.h \
    core/codesyntaxhighlighter.h

SOURCES += \
    core/markdowntohtml.cpp \
    core/languagedefinationxmlparser.cpp \
    core/highlighter.cpp \
    core/codesyntaxhighlighter.cpp


