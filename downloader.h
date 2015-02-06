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

#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

#include <iostream>
#include <QThread>
#include <QString>
#include <QWidget>

#include "plugin.h"
#include "task.h"
#include "block.h"
#include "url.h"
#include "progressbar.h"

using namespace std;


class Downloader: public QThread
{
    Q_OBJECT

	public:
        Downloader(QWidget *parent=0);
		~Downloader(void);

        void run(void);


	public:
		Task task;
        URL url;
        static int download_thread(Downloader *downloader, QThread *ptr_thread);
        int self(QThread *);

    public slots:
        void runMyself(QString);
        void resumeTask(void);
        void setLocalDirectory(QString);
        void setLocalFileName(QString);
        void quit(void);
        void setThreadNum(int);

    signals:
        void begin(Downloader *downloader, QThread *ptr_thread);
        void set_GuiProgressBarMinimum(int);
        void set_GuiProgressBarValue(int);
        void set_GuiProgressBarMaximum(int);
        void set_GuiLabelTotal(QString);
        void set_GuiLabelDownloaded(QString);
        void set_GuiLabelSpeed(QString);
        void set_GuiLabelRemainingTime(QString);
        void errorHappened(QString);
        void done();

	private:
		int init_plugin(void);
		int init_task(void);
		int init_local_file_name(void);
		int init_threads_from_mg(void);
		int init_threads_from_info(void);

		int thread_create(void);

		int schedule(void);
		int save_temp_file_exit(void);

		int directory_download(void);
		int file_download(void);
        void catch_ctrl_c(int signo);

	private:
		Plugin *plugin;
		char *localPath;
		char *localMg;
		int threadNum;
		Block *blocks;
		ProgressBar *pb;
        bool is_dirSetted;
        bool sigint_received;
        bool is_downloading;
};

class DownloadWorker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(Downloader *downloader, QThread *ptr_thread) {
        cerr << endl << "doWork going in thread:" << downloader->self(ptr_thread);
        QString result;
        Downloader::download_thread(downloader, ptr_thread);
        emit resultReady(result);
    }

signals:
    void resultReady(const QString &result);
};


#endif // _DOWNLOADER_H
