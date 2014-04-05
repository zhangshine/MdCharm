TEMPLATE = lib
CONFIG(debug, debug|release){ #debug
    TARGET = gbreakpad_d
    DESTDIR = ../../debug
} else { #release
    TARGET = gbreakpad
    DESTDIR = ../../release
}
## google breakpad svn 969

CONFIG += warn_off thread exceptions rtti stl
CONFIG += static
QT -= gui
##why
DEFINES += QT_NO_CAST_TO_ASCII
DEFINES += QT_NO_CAST_FROM_ASCII

unix:!mac {
    debug {
        QMAKE_CFLAGS_DEBUG += -gstabs
        QMAKE_CXXFLAGS_DEBUG += -gstabs
    }
}

mac {
    LIBS += -lcrypto
}

LIST = thread exceptions rtti stl
for(f, LIST) {
    !CONFIG($$f){
        warning("Add '$$f' to CONFIG, or you will find yourself in 'funny' problems.")
    }
}
INCLUDEPATH += $$PWD
HEADERS += $$PWD/BreakpadHandler.h
SOURCES += $$PWD/BreakpadHandler.cpp

include($$PWD/gbreakpad/gbreakpad.pri)
