# use sundown
CONFIG += warn_off #stable

#QT += core

#for dynamic library
win32: DEFINES += MARKDOWN_LIB
#revision b6b58da

#CONFIG += static

#CONFIG(debug, debug|release){ #debug
#    TARGET = markdown_d
#    DESTDIR = ../../debug
#    LIBS += -L ../../debug -lrapidxml
#} else { #release
#    TARGET = markdown
#    DESTDIR = ../../release
#    LIBS += -L ../../release -lrapidxml
#}

include(../rapidxml/rapidxml.pro)

#TEMPLATE = lib

INCLUDEPATH += $$PWD/html \
            $$PWD/src \
            $$PWD/../core \
            $$PWD/../pcre

SOURCES +=  $$PWD/src/autolink.c \
            $$PWD/src/buffer.c \
            $$PWD/src/markdown.c \
            $$PWD/src/stack.c \
            $$PWD/html/houdini_href_e.c \
            $$PWD/html/houdini_html_e.c \
            $$PWD/html/html.c \
            $$PWD/html/html_smartypants.c \

HEADERS +=  $$PWD/src/autolink.h \
            $$PWD/src/buffer.h \
            $$PWD/src/html_blocks.h \
            $$PWD/src/markdown.h \
            $$PWD/src/stack.h \
            $$PWD/html/houdini.h \
            $$PWD/html/html.h \
