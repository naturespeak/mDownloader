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
#include <QFileDialog>

#include "ratecontroller.h"
#include "torrentclient.h"
#include "utils.h"
#include "addtorrentdialog.h"

#include <iostream>
using std::cerr;
using std::endl;

#include "mainwindowdownloader.h"
#include "ui_mainwindowdownloader.h"

extern void
catch_ctrl_c(int signo);

MainWindowDownloader::MainWindowDownloader(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindowDownloader),
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
    if (!errorMsg.contains(tr("Download successfully in"))) {
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

void MainWindowDownloader::closeEvent(QCloseEvent *event)
{
    qDebug() << "closeEvent is called";
    if (m_is_torrent_mode)
    {
        if (jobs.isEmpty())
        {
            qDebug() << "jobs is empty.";
            return;
        }else {
            qDebug() << "jobs is not empty";
        }

        // Save upload / download numbers.
        saveSettings();
        saveChanges = false;

        quitDialog = new QProgressDialog(tr("Disconnecting from trackers"), tr("Abort"), 0, jobsToStop, this);

        // Stop all clients, remove the rows from the view and wait for
        // them to signal that they have stopped.
        jobsToStop = 0;
        jobsStopped = 0;
        foreach (Job job, jobs) {
            ++jobsToStop;
            TorrentClient *client = job.client;
            client->disconnect();
            connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
            client->stop();
        }

        if (jobsToStop > jobsStopped)
            quitDialog->exec();
        quitDialog->deleteLater();
        quitDialog = 0;
    }
    else
    {
        if (m_is_downloading_started == false && m_is_downloading_finished == true && m_is_downloading_paused == true) {        // Not downloading
            event->accept();
        }else if (m_is_downloading_started == true && m_is_downloading_finished == true && m_is_downloading_paused == true) {   // Paused
            event->accept();
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
    about = tr("mDownloader: A GUI download accelerator.");
    about += QChar::LineSeparator;
    about += tr("Version: 1.0.1Build005.");
    about += QChar::LineSeparator;
    about += tr("Written by Chuan Qin. Email: qc2105@qq.com");
    about += QChar::LineSeparator;
    about += tr("It is based on Qt and Mytget, and licensed under GPL.");
    msgBox.setWindowTitle(tr("mDownloader"));
    msgBox.setText(about);
    msgBox.exec();
}

int MainWindowDownloader::rowOfClient(TorrentClient *client) const
{
    // Return the row that displays this client's status, or -1 if the
    // client is not known.
    int row = 0;
    foreach (Job job, jobs) {
        if (job.client == client)
            return row;
        ++row;
    }
    return -1;
}

void MainWindowDownloader::setUploadLimit(int value)
{
//    int rate = rateFromValue(value);
//    uploadLimitLabel->setText(tr("%1 KB/s").arg(QString().sprintf("%4d", rate)));
      RateController::instance()->setUploadLimit(value * 1024);
}

void MainWindowDownloader::setDownloadLimit(int value)
{
//    int rate = rateFromValue(value);
//    downloadLimitLabel->setText(tr("%1 KB/s").arg(QString().sprintf("%4d", rate)));
      RateController::instance()->setDownloadLimit(value * 1024);
}

bool MainWindowDownloader::addTorrent(const QString &fileName, const QString &destinationFolder,
                         const QByteArray &resumeState)
{
    // Check if the torrent is already being downloaded.
    foreach (Job job, jobs) {
        if (job.torrentFileName == fileName && job.destinationDirectory == destinationFolder) {
            QMessageBox::warning(this, tr("Already downloading"),
                                 tr("The torrent file %1 is "
                                    "already being downloaded.").arg(fileName));
            return false;
        }
    }

    // Create a new torrent client and attempt to parse the torrent data.
    TorrentClient *client = new TorrentClient(this);
    if (!client->setTorrent(fileName)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("The torrent file %1 cannot not be opened/resumed.").arg(fileName));
        delete client;
        return false;
    }
    client->setDestinationFolder(destinationFolder);
    client->setDumpedState(resumeState);

    // Setup the client connections.
    connect(client, SIGNAL(stateChanged(TorrentClient::State)), this, SLOT(updateState(TorrentClient::State)));
    connect(client, SIGNAL(peerInfoUpdated()), this, SLOT(updatePeerInfo()));
    connect(client, SIGNAL(progressUpdated(int)), this, SLOT(updateProgress(int)));
    connect(client, SIGNAL(downloadRateUpdated(int)), this, SLOT(updateDownloadRate(int)));
    connect(client, SIGNAL(downloadedUpdated(qint64)), this, SLOT(updateDownloadedBytes(qint64)));
    connect(client, SIGNAL(uploadRateUpdated(int)), this, SLOT(updateUploadRate(int)));
    connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
    connect(client, SIGNAL(error(TorrentClient::Error)), this, SLOT(torrentError(TorrentClient::Error)));

    // Add the client to the list of downloading jobs.
    Job job;
    job.client = client;
    job.torrentFileName = fileName;
    job.destinationDirectory = destinationFolder;
    jobs << job;


    QString baseFileName = QFileInfo(fileName).fileName();
    if (baseFileName.toLower().endsWith(".torrent"))
        baseFileName.remove(baseFileName.size() - 8, 8);

    setDownloadedFileName(baseFileName);
    setDownloadedDirectory(destinationFolder);
    set_ProgressBarMaximum(100);
    set_ProgressBarMinimum(0);
    char buf[20];
    convert_size(buf, client->totalSize());
    set_labelTotal(buf);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
    client->start();
    return true;
}

void MainWindowDownloader::updateState(TorrentClient::State)
{
    // Update the state string whenever the client's state changes.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    ui->labelTorrentStatusValue->setText(client->stateString());
}

void MainWindowDownloader::updatePeerInfo()
{
    // Update the number of connected, visited, seed and leecher peers.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);

    ui->labelPeersSeedsValue->setText(tr("%1/%2").arg(client->connectedPeerCount())
                                      .arg(client->seedCount()));
}

void MainWindowDownloader::updateProgress(int percent)
{
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);

    set_ProgressBarValue(percent);
}

void MainWindowDownloader::updateDownloadRate(int bytesPerSecond)
{
    // Update the download rate.
    speedBytesPerSecond = bytesPerSecond;
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    QString num;
    num.sprintf("%.1f KB/s", bytesPerSecond / 1024.0);
    set_labelDownloadSpeed(num);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
}

void MainWindowDownloader::updateDownloadedBytes(qint64 downloaded)
{
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    char time_string[20];
    if(speedBytesPerSecond >= 1){
        convert_time(time_string, (client->totalSize() - downloaded) / speedBytesPerSecond);
    }else{
        sprintf(time_string, "--:--");
    }
    char size_string[20];
    convert_size(size_string, downloaded);
    set_labelDownloaded(size_string);
    set_labelRemainingTime(time_string);
    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
}

void MainWindowDownloader::updateUploadRate(int bytesPerSecond)
{
    // Update the upload rate.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    QString num;
    num.sprintf("%.1f KB/s", bytesPerSecond / 1024.0);
    ui->labelUploadingSpeedValue->setText(num);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
}

void MainWindowDownloader::torrentStopped()
{
    // Schedule the client for deletion.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    client->deleteLater();

    // If the quit dialog is shown, update its progress.
    if (quitDialog) {
        if (++jobsStopped == jobsToStop)
            quitDialog->close();
    }
}

void MainWindowDownloader::torrentError(TorrentClient::Error)
{
    // Delete the client.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    QString fileName = jobs.at(row).torrentFileName;
    jobs.removeAt(row);

    // Display the warning.
    QMessageBox::warning(this, tr("Error"),
                         tr("An error occurred while downloading %0: %1")
                         .arg(fileName)
                         .arg(client->errorString()));

//    delete torrentView->takeTopLevelItem(row);
    client->deleteLater();
}

void MainWindowDownloader::saveSettings()
{
    if (!saveChanges)
      return;
    saveChanges = false;

    // Prepare and reset the settings
    QSettings settings("Chuan Qin", "mDownloader");
    settings.clear();

    settings.setValue("LastDirectory", lastDirectory);
//    settings.setValue("UploadLimit", uploadLimitSlider->value());
//    settings.setValue("DownloadLimit", downloadLimitSlider->value());

    // Store data on all known torrents
    settings.beginWriteArray("Torrents");
    for (int i = 0; i < jobs.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("sourceFileName", jobs.at(i).torrentFileName);
        settings.setValue("destinationFolder", jobs.at(i).destinationDirectory);
        settings.setValue("uploadedBytes", jobs.at(i).client->uploadedBytes());
        settings.setValue("downloadedBytes", jobs.at(i).client->downloadedBytes());
        settings.setValue("resumeState", jobs.at(i).client->dumpedState());
    }
    settings.endArray();
    settings.sync();
}

void MainWindowDownloader::loadSettings()
{
    // Load base settings (last working directory, upload/download limits).
    QSettings settings("Chuan Qin", "mDownloader");
    lastDirectory = settings.value("LastDirectory").toString();
    if (lastDirectory.isEmpty())
        lastDirectory = QDir::currentPath();

//    int up = settings.value("UploadLimit").toInt();
//    int down = settings.value("DownloadLimit").toInt();
//    uploadLimitSlider->setValue(up ? up : 170);
//    downloadLimitSlider->setValue(down ? down : 550);

    // Resume all previous downloads.
    int size = settings.beginReadArray("Torrents");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QByteArray resumeState = settings.value("resumeState").toByteArray();
        QString fileName = settings.value("sourceFileName").toString();
        QString dest = settings.value("destinationFolder").toString();

        if (addTorrent(fileName, dest, resumeState)) {
            TorrentClient *client = jobs.last().client;
            client->setDownloadedBytes(settings.value("downloadedBytes").toLongLong());
            client->setUploadedBytes(settings.value("uploadedBytes").toLongLong());
        }
    }
}

void MainWindowDownloader::on_pushButtonTorrent_clicked()
{
    m_is_torrent_mode = true;
    qDebug() << "m_is_torrent_mode: " << m_is_torrent_mode;
    // Show the file dialog, let the user select what torrent to start downloading.
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a torrent file"),
                                                    lastDirectory,
                                                    tr("Torrents (*.torrent);;"
                                                       " All files (*.*)"));
    if (fileName.isEmpty())
        return;
    lastDirectory = QFileInfo(fileName).absolutePath();

    // Show the "Add Torrent" dialog.
    AddTorrentDialog *addTorrentDialog = new AddTorrentDialog(this);
    addTorrentDialog->setTorrent(fileName);
    addTorrentDialog->deleteLater();
    if (!addTorrentDialog->exec())
        return;

    // Add the torrent to our list of downloads
    addTorrent(fileName, addTorrentDialog->destinationFolder());
    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(1000, this, SLOT(saveSettings()));
    }
}

void MainWindowDownloader::on_pushButtonDelTorrent_clicked()
{
    // Find the row of the current item, and find the torrent client
    // for that row.
//    int row = torrentView->indexOfTopLevelItem(torrentView->currentItem());
    TorrentClient *client = jobs.at(0).client;

    // Stop the client.
    client->disconnect();
    connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
    client->stop();

    // Remove the row from the view.
//    delete torrentView->takeTopLevelItem(row);
    jobs.removeFirst();

    setDownloadedFileName("FileName");
    setDownloadedDirectory(lastDirectory);
    set_ProgressBarValue(0);
    set_labelRemainingTime("--:--");
    set_labelTotal("--");
    ui->labelTorrentStatusValue->setText("--");
    ui->labelPeersSeedsValue->setText("--/--");

    updateDownloadRate(0);
    updateDownloadedBytes(0);
    updateUploadRate(0);
    saveChanges = true;
    saveSettings();
}
