TEMPLATE = lib

CONFIG += warn_on #to replace with "warn_off" after stable

QT += core

#DEFINES += ZLIB_DLL
DEFINES += QT_MAKEDLL

CONFIG(debug, debug|release){
    TARGET = zlib_d
    DESTDIR = $$PWD/../../debug
} else {
    TARGET = zlib
    DESTDIR = ../../release
}

INCLUDEPATH += $$PWD/zlib

SOURCES+= \
    $$PWD/zlib/adler32.c \
    $$PWD/zlib/compress.c \
    $$PWD/zlib/crc32.c \
    $$PWD/zlib/deflate.c \
    $$PWD/zlib/gzclose.c \
    $$PWD/zlib/gzlib.c \
    $$PWD/zlib/gzread.c \
    $$PWD/zlib/gzwrite.c \
    $$PWD/zlib/infback.c \
    $$PWD/zlib/inffast.c \
    $$PWD/zlib/inflate.c \
    $$PWD/zlib/inftrees.c \
    $$PWD/zlib/trees.c \
    $$PWD/zlib/uncompr.c \
    $$PWD/zlib/zutil.c
