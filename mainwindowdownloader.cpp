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

#include <WINSOCK2.H>
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
using std::cerr;
using std::endl;

#include "mainwindowdownloader.h"
#include "ui_mainwindowdownloader.h"

extern void
catch_ctrl_c(int signo);

MainWindowDownloader::MainWindowDownloader(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowDownloader)
{
    ui->setupUi(this);
    m_downloadedDirectory = QDir::homePath();
    m_is_downloading_started = false;
    m_is_downloading_finished = true;
    m_is_downloading_paused = true;
    m_has_error_happend = false;
    ui->pushButtonPause->setDisabled(true);
}

MainWindowDownloader::~MainWindowDownloader()
{
    delete ui;
}

void MainWindowDownloader::on_pushButtonNew_clicked()
{
    emit newTaskShow();
}

void MainWindowDownloader::set_ProgressBarMaximum(int maximum)
{
    ui->progressBar->setMaximum(maximum);
}

void MainWindowDownloader::set_ProgressBarMinimum(int minimum)
{
    ui->progressBar->setMinimum(minimum);
}

void MainWindowDownloader::set_ProgressBarValue(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindowDownloader::setDownloadedFileName(QString FileName)
{
    ui->labelFileName->setText(FileName);
}

void MainWindowDownloader::setDownloadedDirectory(QString Directory)
{
    m_downloadedDirectory = Directory;
}

void MainWindowDownloader::set_labelTotal(QString total)
{
    ui->labelTotalValue->setText(total);
}

void MainWindowDownloader::set_labelDownloaded(QString downloaded)
{
    ui->labelDownloadedValue->setText(downloaded);
}

void MainWindowDownloader::set_labelDownloadSpeed(QString speed)
{
    ui->labelSpeedValue->setText(speed);
}

void MainWindowDownloader::set_labelRemainingTime(QString remainingTime)
{
    ui->labelReaminingTimeValue->setText(remainingTime);
}

void MainWindowDownloader::on_error_happens(QString errorMsg)
{
    // When the downloading succeeds m_has_error_happend should keep to be false.
    if (!errorMsg.contains("Download successfully in")) {
        m_has_error_happend = true;
    } else {
        m_has_error_happend = false;
    }
}

void MainWindowDownloader::on_pushButtonOpenDir_clicked()
{
    QDesktopServices::openUrl(QUrl("file:///" + m_downloadedDirectory));
}

void MainWindowDownloader::on_pushButtonPause_clicked()
{
    if (m_is_downloading_paused == false) { // into state PAUSED
        emit m_quit();
        m_is_downloading_paused = true;
        m_is_downloading_started = true;
        m_is_downloading_finished = true;
        ui->pushButtonPause->setText("Resume");
    }else{
        //Resume the task, into state DOWNLOADING
        ui->pushButtonPause->setText("Pause");
        emit resumeTask();
        m_is_downloading_paused = false;
        m_is_downloading_finished = false;
        m_is_downloading_started = true;
    }
}

void MainWindowDownloader::closeEvent(QCloseEvent *event)
{
    if (m_is_downloading_started == false && m_is_downloading_finished == true && m_is_downloading_paused == true) {        // Not downloading
        event->accept();
        WSACleanup();
    }else if (m_is_downloading_started == true && m_is_downloading_finished == true && m_is_downloading_paused == true) {   // Paused
        event->accept();
        WSACleanup();
    }else if (m_is_downloading_started == true && m_is_downloading_finished == false && m_is_downloading_paused == false) { // Downloading
        msgBox.setText("Downloading is in progress. Please press Pause first.");
        msgBox.exec();
        event->ignore();
    }else {                                                                                                                 // Impossible case, Defending programming
        msgBox.setText("Downloading is in progress. Please press Pause first.");
        msgBox.exec();
        event->ignore();
    }
}

void MainWindowDownloader::on_downloading_finished(void)
{
    // into state NO DOWNLOADING
    if (m_has_error_happend == false) {
        m_is_downloading_started = false;
        m_is_downloading_finished = true;
        m_is_downloading_paused = true;
    }
}

void MainWindowDownloader::on_downloading_started(QString)
{
    // into state DOWNLOADING
    m_is_downloading_started = true;
    m_is_downloading_finished = false;
    m_is_downloading_paused = false;
    m_has_error_happend = false;
    ui->pushButtonPause->setDisabled(false);
}

void MainWindowDownloader::on_pushButtonAbout_clicked()
{
    QString about;
    about = "mDownloader: A GUI download accelerator.";
    about += QChar::LineSeparator;
    about += "Version: 1.0.1Build005.";
    about += QChar::LineSeparator;
    about += "Written by Chuan Qin. Email: qc2105@qq.com";
    about += QChar::LineSeparator;
    about += "It is based on Qt and Mytget, and licensed under GPL.";
    msgBox.setText(about);
    msgBox.exec();
}
