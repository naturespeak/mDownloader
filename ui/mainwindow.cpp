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

#include <QProcess>
#include <QStringList>
#include <stdlib.h>
#include <QChar>
#include <QDir>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <QDebug>
#include <QFileDialog>
#include <QTreeWidget>
#include <QHeaderView>
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>

#include "../status.h"
#include "../utils.h"

#include "mainwindow.h"

extern void
catch_ctrl_c(int signo);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    quitDialog(0), saveChanges(false)
{
    m_has_error_happend = false;
    QMetaObject::invokeMethod(this, "loadSettings", Qt::QueuedConnection);

    // Initiallize headers
    QStringList headers;
    headers << tr("Name") << tr("Downloaded/Total") << tr("Progress") << tr("Speed")
            << tr("Status");

    // Main job list
    jobView = new JobView(this);
    jobView->setHeaderLabels(headers);
    jobView->setSelectionBehavior(QAbstractItemView::SelectRows);
    jobView->setAlternatingRowColors(true);
    jobView->setRootIsDecorated(false);
    setCentralWidget(jobView);

    // Set header resize modes and initial section sizes
    QFontMetrics fm = fontMetrics();
    QHeaderView *header = jobView->header();
    header->resizeSection(0, fm.width("typical-name-length-for-a-download-task"));
    header->resizeSection(1, fm.width(headers.at(1) + "100MB/999MB"));
    header->resizeSection(2, fm.width(headers.at(2) + "100%"));
    header->resizeSection(3, qMax(fm.width(headers.at(3) + "   "), fm.width(" 1023.0 KB/s")));
    header->resizeSection(4, qMax(fm.width(headers.at(4) + "   "), fm.width(tr("Downloading") + "   ")));

    // Create common actions
    QAction *newJobAction = new QAction(QIcon(":/ui/icons/bottom.png"), tr("Add &new job"), this);
    pauseJobAction = new QAction(QIcon(":/ui/icons/player_pause.png"), tr("&Pause job"), this);
    removeJobAction = new QAction(QIcon(":/ui/icons/player_stop.png"), tr("&Remove job"), this);

    // File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newJobAction);
    fileMenu->addAction(pauseJobAction);
    fileMenu->addAction(removeJobAction);
    fileMenu->addSeparator();
    fileMenu->addAction(QIcon(":/ui/icons/exit.png"), tr("E&xit"), this, SLOT(close()));

    // Help Menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, SLOT(about()));

    // Top toolbar
    QToolBar *topBar = new QToolBar(tr("Tools"));
    addToolBar(Qt::TopToolBarArea, topBar);
    topBar->setMovable(false);
    topBar->addAction(newJobAction);
    topBar->addAction(pauseJobAction);
    topBar->addAction(removeJobAction);

    // Set up connections
    connect(jobView, SIGNAL(itemSelectionChanged()),
            this, SLOT(setActionsEnabled()));
}

MainWindow::~MainWindow()
{

}

void MainWindow::setActionsEnabled()
{
    // Find the view item and downloader for the current row, and update
    // the states of the actions.
    QTreeWidgetItem *item = 0;
    if (!jobView->selectedItems().isEmpty())
    {
        item = jobView->selectedItems().first();
    }
    Downloader *downloader = item ? jobs.at(jobView->indexOfTopLevelItem(item)).downloader : 0;
    bool pauseEnabled = downloader && ((downloader->m_status->downloadStatus() == Status::Paused)
                                       || (downloader->m_status->downloadStatus() == Status::Idle));

    removeJobAction->setEnabled(item != 0);
    pauseJobAction->setEnabled(item != 0 && pauseEnabled);

    if (downloader && downloader->m_status->downloadStatus() == Status::Paused)
    {
        pauseJobAction->setIcon(QIcon(":/ui/icons/player_play.png"));
        pauseJobAction->setText(tr("Resume job"));
    }
    else
    {
        pauseJobAction->setIcon(QIcon(":/ui/icons/player_pause.png"));
        pauseJobAction->setText(tr("Pause job"));
    }
}

QSize MainWindow::sizeHint() const
{
    const QHeaderView *header = jobView->header();

    // Add up the sizes of all header sections. The last section is
    // stretched, so its size is relative to the size of the width;
    // instead of counting it, we count the size of its largest value.
    int width = fontMetrics().width(tr("Downloading") + "  ");
    for (int i = 0; i < header->count() - 1; ++i)
        width += header->sectionSize(i);

    return QSize(width, QMainWindow::sizeHint().height())
        .expandedTo(QApplication::globalStrut());
}

bool MainWindow::addJob()
{
    return true;
}

void MainWindow::removeJob()
{

}

void MainWindow::pauseJob()
{

}

void MainWindow::moveJobUp()
{

}

void MainWindow::moveJobDown()
{

}

void MainWindow::on_pushButtonNew_clicked()
{
    emit newTaskShow();
}

const Downloader *MainWindow::downloaderForRow(int row) const
{
    return jobs.at(row).downloader;
}

void MainWindow::set_ProgressBarMaximum(int maximum)
{
//    ui->progressBar->setMaximum(maximum);
}

void MainWindow::set_ProgressBarMinimum(int minimum)
{
//    ui->progressBar->setMinimum(minimum);
}

void MainWindow::set_ProgressBarValue(int value)
{
//    ui->progressBar->setValue(value);
}

void MainWindow::set_labelTotal(QString total)
{
//    ui->labelTotalValue->setText(total);
}

void MainWindow::set_labelDownloaded(QString downloaded)
{
//    ui->labelDownloadedValue->setText(downloaded);
}

void MainWindow::set_labelDownloadSpeed(QString speed)
{
//    ui->labelSpeedValue->setText(speed);
}

void MainWindow::set_labelRemainingTime(QString remainingTime)
{
//    ui->labelReaminingTimeValue->setText(remainingTime);
}

void MainWindow::on_error_happens(QString errorMsg)
{
    // When the downloading succeeds m_has_error_happend should keep to be false.
    if (!errorMsg.contains(tr("Download successfully in"))) {
        m_has_error_happend = true;
    } else {
        m_has_error_happend = false;
    }
}

void MainWindow::on_pushButtonOpenDir_clicked()
{
//    QDesktopServices::openUrl(QUrl("file:///" + m_downloadedDirectory));
}

void MainWindow::on_pushButtonPause_clicked()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "closeEvent is called";


}

void MainWindow::on_downloading_finished(void)
{

}

void MainWindow::on_downloading_started(QString)
{

}

void MainWindow::about()
{
    QString about;
    about = tr("mDownloader: A GUI download accelerator.");
    about += QChar::LineSeparator;
    about += tr("Version: 1.1.0Build001.");
    about += QChar::LineSeparator;
    about += tr("Written by Chuan Qin. Email: qc2105@qq.com");
    about += QChar::LineSeparator;
    about += tr("It is based on Qt, and licensed under GPL.");
    msgBox.setWindowTitle(tr("mDownloader"));
    msgBox.setText(about);
    msgBox.exec();
}
