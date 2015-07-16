#-------------------------------------------------
#
# Project created by QtCreator 2014-01-26T14:49:20
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mDownloader
TEMPLATE = app


SOURCES += main.cpp\
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
    url.cpp \
    utils.cpp \
    errormessagebox.cpp \
    status.cpp \
    ui/mainwindowdownloader.cpp \
    ui/newtask.cpp

HEADERS  +=   advio.h \
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
    url.h \
    utils.h \
    errormessagebox.h \
    status.h \
    ui/mainwindowdownloader.h \
    ui/newtask.h

FORMS    += \
    ui/mainwindowdownloader.ui \
    ui/newtask.ui


RESOURCES += \
    mDownloader.qrc

TRANSLATIONS = mDownloader_zh_CN.ts
