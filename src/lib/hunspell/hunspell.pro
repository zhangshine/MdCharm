TEMPLATE = lib

CONFIG += warn_off plugin
unix:VERSION = 1.3.2

win32: {
    RC_FILE = $$PWD/res/hunspell.rc
}
win32:DEFINES *= WIN32
win32:INCLUDEPATH += .
win32:DEPENDPATH += .

DEFINES -= HUNSPELL_WARNING_ON
DEFINES -= HUNSPELL_EXPERIMENTAL
win32-msvc*:DEFINES += BUILDING_LIBHUNSPELL

CONFIG(debug, debug|release){
    TARGET = hunspell_d
    DESTDIR = ../../debug
} else {
    TARGET = hunspell
    DESTDIR = ../../release
}

HEADERS +=  src/affentry.hxx \
            src/affixmgr.hxx \
            src/atypes.hxx \
            src/baseaffix.hxx \
            src/csutil.hxx \
            src/dictmgr.hxx \
            src/filemgr.hxx \
            src/hashmgr.hxx \
            src/htypes.hxx \
            src/hunspell.hxx \
            src/hunzip.hxx \
            src/langnum.hxx \
            src/phonet.hxx \
            src/replist.hxx \
            src/suggestmgr.hxx \
            src/w_char.hxx \
            src/hunspell.h \
            src/hunvisapi.h \
            src/config.h

SOURCES +=  src/affentry.cxx \
            src/affixmgr.cxx \
            src/csutil.cxx \
            src/dictmgr.cxx \
            src/filemgr.cxx \
            src/hashmgr.cxx \
            src/hunspell.cxx \
            src/hunzip.cxx \
            src/phonet.cxx \
            src/replist.cxx \
            src/suggestmgr.cxx \
            src/utf_info.cxx
