#-------------------------------------------------
#
# Project created by QtCreator 2015-06-05T13:54:03
#
#-------------------------------------------------

QT       += widgets network testlib

QT       -= gui

TARGET = tst_httpplugintest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_httpplugintest.cc \
    ../../httpplugin.cpp \
    ../../plugin.cpp \
    ../../task.cpp \
    ../../debug.cpp \
    ../../url.cpp \
    ../../utils.cpp \
    ../../block.cpp \
    ../../advio.cpp \
    ../../proxy.cpp \
    ../../header.cpp \
    ../../http.cpp \
    ../../tcp.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../httpplugin.h \
    ../../plugin.h \
    ../../task.h \
    ../../debug.h \
    ../../macro.h \
    ../../url.h \
    ../../utils.h \
    ../../block.h \
    ../../advio.h \
    ../../proxy.h \
    ../../header.h \
    ../../http.h \
    ../../tcp.h

LIBS += -LC:/OpenSSL-Win32/lib -lssleay32
INCLUDEPATH += C:/OpenSSL-Win32/include
