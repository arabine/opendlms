# This is the Shaddam project file
#

DESTDIR = $${PWD}

Release:OBJECTS_DIR = $${PWD}/release/obj
Release:MOC_DIR = $${PWD}/release/moc
Release:RCC_DIR = $${PWD}/release/rcc
Release:UI_DIR = $${PWD}/release/ui
Release:UI_HEADERS_DIR  = $${PWD}/release/include
Release:UI_SOURCES_DIR  = $${PWD}/release/src

Debug:OBJECTS_DIR = $${PWD}/debug/obj
Debug:MOC_DIR = $${PWD}/debug/moc
Debug:RCC_DIR = $${PWD}/debug/rcc
Debug:UI_DIR = $${PWD}/debug/ui
Debug:UI_HEADERS_DIR  = $${PWD}/debug/include
Debug:UI_SOURCES_DIR  = $${PWD}/debug/src

TARGET = shaddam

# Specific OS stuff
win32 {
    RC_FILE = icon.rc
    # Let's make everything's static so that we don't need any DLL
    QMAKE_LFLAGS += -static-libgcc -static-libstdc++ -static -lpthread
}

QMAKE_CXXFLAGS += -std=c++11

VPATH += lua/src
VPATH += cosem

INCLUDEPATH += lua/src
INCLUDEPATH += cosem

QT += widgets

HEADERS += src/mainwindow.h \
        src/i_script.h \
        src/ThreadQueue.h \
        src/Settings.h \
        src/LuaWrapper.h \
        src/Cosem.h


SOURCES +=  src/mainwindow.cpp \
            src/main.cpp \
            src/LuaWrapper.cpp \
            src/Settings.cpp \
            src/Cosem.cpp

# ----------------------
# LUA
# ----------------------
SOURCES +=  lapi.c \
            lauxlib.c \
            lbaselib.c \
            lcode.c \
            lcorolib.c \
            lctype.c \
            ldblib.c \
            ldebug.c \
            ldo.c \
            ldump.c \
            lfunc.c \
            lgc.c \
            linit.c \
            liolib.c \
            llex.c \
            lmathlib.c \
            lmem.c \
            loadlib.c \
            lobject.c \
            lopcodes.c \
            loslib.c \
            lparser.c \
            lstate.c \
            lstring.c \
            lstrlib.c \
            ltable.c \
            ltablib.c \
            ltm.c \
            lundump.c \
            lutf8lib.c \
            lvm.c \
            lzio.c

RESOURCES += shaddam.qrc

