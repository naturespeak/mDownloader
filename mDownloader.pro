#-------------------------------------------------
#
# Project created by QtCreator 2014-01-26T14:49:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mDownloader
TEMPLATE = app


SOURCES += main.cpp\
        mainwindowdownloader.cpp \
    advio.cpp \
    block.cpp \
    debug.cpp \
    downloader.cpp \
    ftp.cpp \
    ftpparser.cpp \
    ftpplugin.cpp \
    header.cpp \
    http.cpp \
    httpplugin.cpp \
    plugin.cpp \
    progressbar.cpp \
    proxy.cpp \
    task.cpp \
    tcp.cpp \
    url.cpp \
    utils.cpp \
    newtask.cpp \
    errormessagebox.cpp

HEADERS  += mainwindowdownloader.h \
    advio.h \
    block.h \
    debug.h \
    downloader.h \
    ftp.h \
    ftpparser.h \
    ftpplugin.h \
    header.h \
    http.h \
    httpplugin.h \
    macro.h \
    myget.h \
    plugin.h \
    progressbar.h \
    proxy.h \
    task.h \
    tcp.h \
    url.h \
    utils.h \
    newtask.h \
    errormessagebox.h

FORMS    += mainwindowdownloader.ui \
    newtask.ui


RESOURCES += \
    mDownloader.qrc

LIBS += -LC:/OpenSSL-Win32/lib -lssleay32
INCLUDEPATH += C:/OpenSSL-Win32/include

TRANSLATIONS = mDownloader_zh_CN.ts
