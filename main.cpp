/*  mDownloader - a multiple-threads downloading accelerator program that is based on Myget.
 *  Homepage: http://qinchuan.me/article.php?id=100
 *  2015 By Richard (qc2105@qq.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Myget - A download accelerator for GNU/Linux
 *  Homepage: http://myget.sf.net
 *  2005 by xiaosuo
 */

#include <iostream>
using namespace std;

#include "mainwindowdownloader.h"
#include "newtask.h"
#include "myget.h"
#include "errormessagebox.h"

#include <QObject>
#include <QApplication>
#include <QDesktopWidget>
#include <QTranslator>
#include <QLibraryInfo>


void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}

int main(int argc, char *argv[])
{   
    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);

    QTranslator qtTranslator;
       qtTranslator.load("qt_" + QLocale::system().name(),
               QLibraryInfo::location(QLibraryInfo::TranslationsPath));
       a.installTranslator(&qtTranslator);

       QTranslator myappTranslator;
       myappTranslator.load("mDownloader_" + QLocale::system().name());
       a.installTranslator(&myappTranslator);

    a.setQuitOnLastWindowClosed(true);

    QDesktopWidget qd;

    MainWindowDownloader w;
    w.set_ProgressBarValue(0);
    w.move((qd.availableGeometry(-1).width()-w.width())/2, (qd.availableGeometry(-1).height()-w.height())/2);
    w.show();

    NewTask newTask;
    QObject::connect(&w, SIGNAL(newTaskShow()), &newTask, SLOT(showMyself()));
    QObject::connect(&newTask, SIGNAL(setFileName(QString)), &w, SLOT(setDownloadedFileName(QString)));
    QObject::connect(&newTask, SIGNAL(setDownloadedDirectory(QString)), &w, SLOT(setDownloadedDirectory(QString)));    

    ErrorMessageBox errMsgBox;
    Downloader downloader;

    QObject::connect(&downloader, SIGNAL(set_GuiProgressBarMinimum(int)), &w, SLOT(set_ProgressBarMinimum(int)));
    QObject::connect(&downloader, SIGNAL(set_GuiProgressBarValue(int)), &w, SLOT(set_ProgressBarValue(int)));
    QObject::connect(&downloader, SIGNAL(set_GuiProgressBarMaximum(int)), &w, SLOT(set_ProgressBarMaximum(int)));
    QObject::connect(&downloader, SIGNAL(set_GuiLabelTotal(QString)), &w, SLOT(set_labelTotal(QString)));
    QObject::connect(&downloader, SIGNAL(set_GuiLabelDownloaded(QString)), &w, SLOT(set_labelDownloaded(QString)));
    QObject::connect(&downloader, SIGNAL(set_GuiLabelSpeed(QString)), &w, SLOT(set_labelDownloadSpeed(QString)));
    QObject::connect(&downloader, SIGNAL(set_GuiLabelRemainingTime(QString)), &w, SLOT(set_labelRemainingTime(QString)));

    QObject::connect(&downloader, SIGNAL(errorHappened(QString)), &errMsgBox, SLOT(DisplayError(QString)));
    QObject::connect(&downloader, SIGNAL(errorHappened(QString)), &w, SLOT(on_error_happens(QString)));

    QObject::connect(&newTask, SIGNAL(setFileName(QString)), &downloader, SLOT(setLocalFileName(QString)));
    QObject::connect(&newTask, SIGNAL(setDownloadedDirectory(QString)), &downloader, SLOT(setLocalDirectory(QString)));
    QObject::connect(&newTask, SIGNAL(setThreadNum(int)), &downloader, SLOT(setThreadNum(int)));
    QObject::connect(&w, SIGNAL(resumeTask()), &downloader, SLOT(resumeTask()));
    QObject::connect(&newTask, SIGNAL(runDownloader(QString)), &w, SLOT(on_downloading_started(QString)));
    QObject::connect(&newTask, SIGNAL(runDownloader(QString)), &downloader, SLOT(runMyself(QString)));



    QObject::connect(&w, SIGNAL(m_quit()), &downloader, SLOT(quit()));
    QObject::connect(&downloader, SIGNAL(done()), &w, SLOT(on_downloading_finished()));

    return a.exec();
}

