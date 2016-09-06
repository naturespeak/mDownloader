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

#include <stdlib.h>
#include <stdio.h>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QUuid>
#include <QUrl>
#include <QTimer>


#include "newtask.h"
#include "ui_newtask.h"


NewTask::NewTask(QWidget *parent) :
    QDialog(parent), isChangesSaved(false),
    ui(new Ui::NewTask)
{
    ui->setupUi(this);

    loadSettings();
    ui->lineEditUrl->setText(tr("Paste or input the URL of the file you want to download here."));
    ui->lineEditSaveLocation->setText(lastDirectory + QDir::separator() + ui->lineEditFileName->text());
    ui->spinBoxThreadNum->setMinimum(1);
    ui->spinBoxThreadNum->setValue(1);
    connect(ui->lineEditUrl, SIGNAL(textChanged(QString)), this, SLOT(setFileNameSlot(QString)));
}

NewTask::~NewTask()
{
    delete ui;
}

void NewTask::showMyself(void)
{
    show();
}

void NewTask::on_buttonBoxWhetherOk_accepted()
{
    if (QUrl(ui->lineEditUrl->text()).isValid() && QUrl(ui->lineEditFileName->text()).isValid())
    {
        emit setDownloadedDirectory(lastDirectory);
        emit setFileName(ui->lineEditFileName->text());
        emit setSaveLocation(ui->lineEditSaveLocation->text());
        emit setThreadNum(ui->spinBoxThreadNum->value());
        emit runDownloader(ui->lineEditUrl->text());
        emit newJob(ui->lineEditFileName->text(), lastDirectory, ui->lineEditUrl->text(), ui->spinBoxThreadNum->value());
    }
}

void NewTask::setFileNameSlot(QString Url)
{
    localFileName = Url.section("/", -1);
    if (localFileName.isEmpty()) {
        localFileName = "index.html";
    }
    ui->lineEditFileName->setText(/*"file-" + QUuid::createUuid().toString()+ */localFileName);
    ui->lineEditSaveLocation->setText(QDir::toNativeSeparators(lastDirectory + QDir::separator() +  /*"file-" + QUuid::createUuid().toString()+*/ localFileName));
}

void NewTask::on_pushButtonSetSaveLocation_clicked()
{
    lastDirectory = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QDir::homePath(), QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    ui->lineEditSaveLocation->setText(QDir::toNativeSeparators(lastDirectory + QDir::separator() + ui->lineEditFileName->text()));
    if (!isChangesSaved) {
        isChangesSaved = true;
        QTimer::singleShot(1000, this, SLOT(saveSettings()));
    }
}

void NewTask::loadSettings()
{
    QSettings settings("linuxqc", "mDownloader");
    lastDirectory = settings.value("lastDirectory").toString();
    if (lastDirectory.isEmpty())
        lastDirectory = QDir::toNativeSeparators(QDir::homePath());
}

void NewTask::saveSettings()
{
    if (!isChangesSaved)
      return;
    isChangesSaved = false;

    // Prepare and reset the settings
    QSettings settings("linuxqc", "mDownloader");
    settings.clear();

    settings.setValue("lastDirectory", lastDirectory);

    settings.sync();
}

void NewTask::closeEvent(QCloseEvent */*event*/)
{
    saveSettings();
    isChangesSaved = false;
}

