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

#ifndef MAINWINDOWDOWNLOADER_H
#define MAINWINDOWDOWNLOADER_H

#include <QMainWindow>
#include <QString>
#include <QMessageBox>

namespace Ui {
class MainWindowDownloader;
}

class MainWindowDownloader : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowDownloader(QWidget *parent = 0);
    ~MainWindowDownloader();

public slots:
    void set_ProgressBarMinimum(int);
    void set_ProgressBarValue(int);
    void set_ProgressBarMaximum(int);
    void setDownloadedFileName(QString);
    void setDownloadedDirectory(QString);
    void set_labelTotal(QString);
    void set_labelDownloaded(QString);
    void set_labelDownloadSpeed(QString);
    void set_labelRemainingTime(QString);
    void on_error_happens(QString);

private slots:
    void on_pushButtonNew_clicked();

    void on_pushButtonOpenDir_clicked();

    void on_pushButtonPause_clicked();
    virtual void closeEvent(QCloseEvent *);

    void on_downloading_finished(void);
    void on_downloading_started(QString);

    void on_pushButtonAbout_clicked();

signals:
    void newTaskShow(void);
    void m_quit(void);
    void resumeTask(void);

private:
    Ui::MainWindowDownloader *ui;
    QString m_downloadedFileName;
    QString m_downloadedDirectory;
    bool m_is_downloading_finished;
    bool m_is_downloading_started;
    bool m_has_error_happend;
    QMessageBox msgBox;
    bool m_is_downloading_paused;
};

#endif // MAINWINDOWDOWNLOADER_H
