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

#include "../utils.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

extern void
catch_ctrl_c(int signo);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    quitDialog(0), saveChanges(false)
{
    ui->setupUi(this);
    m_downloadedDirectory = QDir::homePath();
    m_is_downloading_started = false;
    m_is_downloading_finished = true;
    m_is_downloading_paused = true;
    m_has_error_happend = false;
    m_is_torrent_mode = false;
    ui->pushButtonPause->setDisabled(true);
    QMetaObject::invokeMethod(this, "loadSettings", Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonNew_clicked()
{
    emit newTaskShow();
}

void MainWindow::set_ProgressBarMaximum(int maximum)
{
    ui->progressBar->setMaximum(maximum);
}

void MainWindow::set_ProgressBarMinimum(int minimum)
{
    ui->progressBar->setMinimum(minimum);
}

void MainWindow::set_ProgressBarValue(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindow::setDownloadedFileName(QString FileName)
{
    ui->labelFileName->setText(FileName);
}

void MainWindow::setDownloadedDirectory(QString Directory)
{
    m_downloadedDirectory = Directory;
}

void MainWindow::set_labelTotal(QString total)
{
    ui->labelTotalValue->setText(total);
}

void MainWindow::set_labelDownloaded(QString downloaded)
{
    ui->labelDownloadedValue->setText(downloaded);
}

void MainWindow::set_labelDownloadSpeed(QString speed)
{
    ui->labelSpeedValue->setText(speed);
}

void MainWindow::set_labelRemainingTime(QString remainingTime)
{
    ui->labelReaminingTimeValue->setText(remainingTime);
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
    QDesktopServices::openUrl(QUrl("file:///" + m_downloadedDirectory));
}

void MainWindow::on_pushButtonPause_clicked()
{
    if (m_is_downloading_paused == false) { // into state PAUSED
        emit m_quit();
        m_is_downloading_paused = true;
        m_is_downloading_started = true;
        m_is_downloading_finished = true;
        ui->pushButtonPause->setText(tr("Resume"));
    }else{
        //Resume the task, into state DOWNLOADING
        ui->pushButtonPause->setText(tr("Pause"));
        emit resumeTask();
        m_is_downloading_paused = false;
        m_is_downloading_finished = false;
        m_is_downloading_started = true;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "closeEvent is called";

    if (m_is_downloading_started == false && m_is_downloading_finished == true && m_is_downloading_paused == true) {        // Not downloading
        event->accept();
    }else if (m_is_downloading_started == true && m_is_downloading_finished == true && m_is_downloading_paused == true) {   // Paused
        event->accept();
    }else if (m_is_downloading_started == true && m_is_downloading_finished == false && m_is_downloading_paused == false) { // Downloading
        msgBox.setText("Downloading is in progress. Please press Pause first.");
        msgBox.exec();
        event->ignore();
    }else {                                                                                      // Impossible case, Defending programming
        msgBox.setText("Downloading is in progress. Please press Pause first.");
        msgBox.exec();
        event->ignore();
    }
}

void MainWindow::on_downloading_finished(void)
{
    // into state NO DOWNLOADING
    if (m_has_error_happend == false) {
        m_is_downloading_started = false;
        m_is_downloading_finished = true;
        m_is_downloading_paused = true;
    }
}

void MainWindow::on_downloading_started(QString)
{
    // into state DOWNLOADING
    m_is_downloading_started = true;
    m_is_downloading_finished = false;
    m_is_downloading_paused = false;
    m_has_error_happend = false;
    ui->pushButtonPause->setDisabled(false);
}

void MainWindow::on_pushButtonAbout_clicked()
{
    QString about;
    about = tr("mDownloader: A GUI download accelerator.");
    about += QChar::LineSeparator;
    about += tr("Version: 1.0.1Build005.");
    about += QChar::LineSeparator;
    about += tr("Written by Chuan Qin. Email: qc2105@qq.com");
    about += QChar::LineSeparator;
    about += tr("It is based on Qt, and licensed under GPL.");
    msgBox.setWindowTitle(tr("mDownloader"));
    msgBox.setText(about);
    msgBox.exec();
}
