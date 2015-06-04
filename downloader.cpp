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

#include <sys/stat.h>
#include <errno.h>
#include <direct.h>

#include "downloader.h"
#include "macro.h"
#include "utils.h"
#include "ftpplugin.h"
#include "httpplugin.h"
#include "progressbar.h"
#include "debug.h"
#include "header.h"

#include <QDir>
#include <QByteArray>

#if WIN32
#define snprintf sprintf_s
#endif

#ifdef HAVE_SSL
#	include <openssl/ssl.h>
#endif

typedef void* (*threadFunction) (void*);

void
Downloader::catch_ctrl_c(int signo)
{
    if(is_downloading){
        sigint_received = true;
        cerr << endl << "sigint_received: " << sigint_received << endl;
    }else{
        cerr << "Will exit." << endl;
        exit(0);
    }
}


Downloader::Downloader(QWidget *parent) :
    QThread(parent)
{
    sigint_received = false;
    is_downloading = false;
    plugin = NULL;
    blocks = NULL;
    localPath = NULL;
    localMg = NULL;
    pb = new ProgressBar;
    is_dirSetted = false;
    emit set_GuiProgressBarMinimum(0);
}

Downloader::~Downloader(void)
{
    delete[] blocks;
    delete[] localPath;
    delete[] localMg;
    delete pb;
    delete plugin;
}

int
Downloader::init_plugin(void)
{
    if(task.get_proxy_type() == HTTP_PROXY){
        delete plugin;
        plugin = new HttpPlugin;
    }else{
        switch(task.get_protocol()){
        case HTTP:
#ifdef HAVE_SSL
        case HTTPS:
#endif
            delete plugin;
            plugin = new HttpPlugin;
            break;
        case FTP:
            delete plugin;
            plugin = new FtpPlugin;
            break;
        default:
            return -1;
        }
    }

    return 0;
};

/* init_plugin, plugin->get_info */
int
Downloader::init_task(void)
{
    int i;
    int ret;

_reinit_plugin:
    if(init_plugin() < 0){
        cerr<<"Unknown protocol"<<endl;
        return -1;
    }

    for(i = 0; task.get_tryCount() <= 0 || i < task.get_tryCount(); i ++){
        ret = plugin->get_info(&task);
        if(ret == -1){
            cerr << "Plugin->get_info return -1" << endl;
            return -1;
        }else if(ret == S_REDIRECT){
            cerr<<"Redirect to: "<<task.get_url()<<endl;
            goto _reinit_plugin;
        }else if(ret == 0){
            return 0;
        }else{
            continue;
        }
    }
    cerr << "return e_max_count" << endl;
    cerr << "ret = " << ret << endl;
    return E_MAX_COUNT;
}

int
Downloader::init_local_file_name(void)
{
    int length;
    char *tmpStr;

    length = task.get_local_dir() ? strlen(task.get_local_dir()) : 1;
    length += task.get_local_file() ? strlen(task.get_local_file()) :
                                      strlen(task.get_file());
    length += 6;

    tmpStr = new char[length];

    snprintf( tmpStr, length, "%s/%s.mg!",
              task.get_local_dir() ? task.get_local_dir() : ".",
              task.get_local_file() ? task.get_local_file() :
                                      task.get_file() );
    delete[] localPath;
    delete[] localMg;
    tmpStr[length - 5] = '\0';
    localPath = StrDup(tmpStr);
    tmpStr[length - 5] = '.';
    localMg = tmpStr;

    return 0;
}

int
Downloader::init_threads_from_mg(void)
{
    QFile *tempFile = NULL;
    QByteArray readData;
    int i;
    int blockSize = 0;
    QString error;

    readData.clear();

    readData.setNum(task.get_file_size());
    blockSize = readData.size();

    readData.clear();

    tempFile = new QFile(localMg);

    if (!tempFile->open(QIODevice::ReadOnly)) {
        cerr<<"Can not access the temp file: "<<localMg<<endl;
        error = QString("Can not access the temp file: ");
        error += QString(localMg);
        emit errorHappened(error);
        delete tempFile;
        return -1;
    }

    cerr << "File size immediately after open: " << tempFile->size() << endl;

    tempFile->seek(task.get_file_size());
    readData = tempFile->read(blockSize);
    threadNum = readData.toInt();
    readData.clear();

    cerr << "File size: " << tempFile->size()<< endl;
    cerr << "threadNum: " << threadNum << endl;

    // TODO: We'd better use a hash function to check.
    if(tempFile->size() < task.get_file_size() + 3 * blockSize * threadNum) {
        cerr<<"the temp file: \""<<localMg<<"\" is not correct, type I."<<endl;
        error = QString("The temp file: ");
        error += QString(localMg);
        error += QString(" is not correct, type I.");
        tempFile->close();
        delete tempFile;
        emit errorHappened(error);
        return -1;
    }


    delete[] blocks;
    blocks = new Block[threadNum];
    for(i = 0; i < threadNum; i ++){
        tempFile->seek(task.get_file_size() + (threadNum*i+1) * blockSize);
        readData = tempFile->read(blockSize);
        blocks[i].startPoint = readData.toLong();
        readData.clear();
        cerr << "startPoint for thread " << i << ": " << blocks[i].startPoint << endl;
        tempFile->seek(task.get_file_size() + (threadNum*i+2) * blockSize);
        readData = tempFile->read(blockSize);
        blocks[i].downloaded = readData.toLong();
        readData.clear();
        cerr << "downloaded for thread " << i << ": " << blocks[i].downloaded << endl;
        tempFile->seek(task.get_file_size() + (threadNum*i+3) * blockSize);
        readData = tempFile->read(blockSize);
        blocks[i].size = readData.toLong();
        readData.clear();
        cerr << "size for thread " << i << ": " << blocks[i].size << endl;
        if(blocks[i].bufferFile.open(localMg) < 0){
            tempFile->close();
            delete tempFile;
            perror("Can not open the temp file to write");
            emit errorHappened(QString("Can not open the temp file to write"));
            return -1;
        }
    }

    tempFile->close();
    delete tempFile;
    return 0;
}

int
Downloader::init_threads_from_info(void)
{
    qint64 block_size;
    int i;

    threadNum = task.get_threadNum()> 0 ? task.get_threadNum() : 1;
    block_size = task.get_file_size() / threadNum;
    if(block_size <= 20480){ // too small file
        threadNum = 1;
        block_size = task.get_file_size();
    }

    delete[] blocks;
    blocks = new Block[threadNum];
    for(i = 0; i < threadNum; i ++){
        blocks[i].startPoint = i * block_size;
        blocks[i].size = block_size;
        if(blocks[i].bufferFile.open(localMg) < 0){
            perror("Can not open the temp file to write");
            emit errorHappened(QString("Can not open the temp file to write"));
            return -1;
        }
    }

    blocks[threadNum - 1].size = task.get_file_size() - block_size * ( threadNum - 1);

    return 0;
}

int
Downloader::thread_create(void)
{
    int i;

    QThread *workerThread = new QThread;
    DownloadWorker *worker = new DownloadWorker;
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    workerThread->start();
    for(i = 0; i < threadNum; i ++){
        if(blocks[i].ptr_thread == 0){ // found an empty slot
            blocks[i].ptr_thread = workerThread;
            break;
        }
    }

    if(i == threadNum) return -1;

    connect(this, SIGNAL(begin(Downloader*,QThread*)), worker, SLOT(doWork(Downloader*,QThread*)));
    emit begin(this, workerThread);
    disconnect(this,SIGNAL(begin(Downloader*,QThread*)), 0,0);

    return 0;
}

int
Downloader::self(QThread *ptr_thread)
{
    QThread *self;
    self = ptr_thread;
    int i;
    while(1){
        for(i = 0; i < threadNum; i ++){
            if(blocks[i].ptr_thread == self) return i;
        }
        // the parent thread maybe slower than me
    }
}

int
Downloader::download_thread(Downloader *downloader, QThread *ptr_thread)
{
    int self, ret;
    self = downloader->self(ptr_thread);
    cerr << endl << "download_thread start in thread: " << self;

    while(1){
        ret = downloader->plugin->download(downloader->task, downloader->blocks + self);
        if (ret == E_SYS) {
            cerr << endl << "thread " << self << " plugin download returen E_SYS" << endl;
        }
        else
        {
            cerr << endl << "thread " << self << " plugin download returned: " << ret << endl;
        }
        if(ret == E_SYS){ // system error
            downloader->blocks[self].state = EXIT;
            return -1;
        }else if(ret == 0){
            downloader->blocks[self].state = EXIT;
            return 0;
        }else{
            continue;
        }
    }

    downloader->blocks[self].state = EXIT;
    return E_MAX_COUNT;
}

int
Downloader::schedule(void)
{
    int i, j;
    int joined;

    joined = 0;
    for(i = 0; i < threadNum; i ++){
        if(blocks[i].state == WAIT){
            for(j = i + 1; j < threadNum; j ++){
                if(blocks[i].startPoint + blocks[i].size == blocks[j].startPoint){
                    break;
                }
            }
            if(j < threadNum && blocks[j].downloaded == 0){
                if(blocks[j].state == STOP || blocks[j].state == EXIT){
                    blocks[j].ptr_thread->quit();
                    blocks[j].state = JOINED;
                    blocks[j].bufferFile.close();
                }else if(blocks[j].state != JOINED){
                    continue;
                }
                blocks[i].size += blocks[j].size;
                blocks[i].state = WAKEUP;
                blocks[j].startPoint = -1;
                blocks[j].downloaded = 0;
                blocks[j].size = 0;
                i = j;
                joined += j - i;
                qint64 *data = new qint64[threadNum];
                for(j = 0; j < threadNum; j ++){
                    data[j] = blocks[j].startPoint;
                }
                pb->set_start_point(data);

                delete[] data;
            }
        }else if(blocks[i].state == EXIT){
            blocks[i].ptr_thread->quit();
            blocks[i].state = JOINED;
            blocks[i].bufferFile.close();
            joined ++;
        }else if(blocks[i].state == JOINED){
            joined ++;
        }else{
            continue;
        }
    }

    return threadNum - joined;
}

int
Downloader::save_temp_file_exit(void)
{
    int i;
    QFile *tempFile;
    QByteArray writeData;
    int blockSize = 0;
    QString error;

    writeData.clear();

    writeData.setNum(task.get_file_size());

    blockSize = writeData.size();

    writeData.clear();

    for(i = 0; i < threadNum; i ++){
        if(blocks[i].state != JOINED){
            blocks[i].ptr_thread->quit();
            blocks[i].ptr_thread->terminate();
            blocks[i].ptr_thread->wait();
            blocks[i].bufferFile.close();
            blocks[i].state = STOP;
        }
    };

    if(task.get_file_size() < 0){
        cerr<<"!!!You can not continue in further"<<endl;
        is_downloading = false;
        exit(-1); // Is this right?
    }

    tempFile = new QFile(localMg);
    if (!tempFile->open(QIODevice::ReadWrite)) {
        cerr<<"Can not create the temp file: "<<localMg<<endl;
        error = QString("Can not create the temp file: ");
        error += QString(localMg);
        emit errorHappened(error);
        delete tempFile;
        return -1;
    }

    tempFile->seek(task.get_file_size());
    writeData.setNum(threadNum);
    tempFile->write(writeData);
    writeData.clear();
    for(i = 0; i < threadNum; i++){
        tempFile->seek(task.get_file_size() + (threadNum*i+1) * blockSize);
        writeData.setNum(blocks[i].startPoint);
        tempFile->write(writeData);
        writeData.clear();
        cerr << "Write startPoint for thread " << i << ": " << blocks[i].startPoint << endl;
        tempFile->seek(task.get_file_size() + (threadNum*i+2) * blockSize);
        writeData.setNum(blocks[i].downloaded);
        tempFile->write(writeData);
        writeData.clear();
        cerr << "Write downloaded for thread " << i << ": " << blocks[i].downloaded << endl;
        tempFile->seek(task.get_file_size() + (threadNum*i+3) * blockSize);
        writeData.setNum(blocks[i].size);
        tempFile->write(writeData);
        writeData.clear();
        cerr << "Write size for thread " << i << ": " << blocks[i].size << endl;
    }
    tempFile->close();
    delete tempFile;

    is_downloading = false;
    QString inforMsg = tr("Downloading paused.");
    emit errorHappened(inforMsg);
    emit done();
    return 0;
}

/* mkstemp extracted from libc/sysdeps/posix/tempname.c.  Copyright
   (C) 1991-1999, 2000, 2001, 2006 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.  */

static const char letters[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/* Generate a temporary file name based on TMPL.  TMPL must match the
   rules for mk[s]temp (i.e. end in "XXXXXX").  The name constructed
   does not exist at the time of the call to mkstemp.  TMPL is
   overwritten with the result.  */
int
mkstemp (char *tmpl)
{
    int len;
    char *XXXXXX;
    static unsigned long long value;
    unsigned long long random_time_bits;
    unsigned int count;
    int fd = -1;
    int save_errno = errno;

    /* A lower bound on the number of temporary files to attempt to
     generate.  The maximum total number of temporary file names that
     can exist for a given template is 62**6.  It should never be
     necessary to try all these combinations.  Instead if a reasonable
     number of names is tried (we define reasonable as 62**3) fail to
     give the system administrator the chance to remove the problems.  */
#define ATTEMPTS_MIN (62 * 62 * 62)

    /* The number of times to attempt to generate a temporary file.  To
     conform to POSIX, this must be no smaller than TMP_MAX.  */
#if ATTEMPTS_MIN < TMP_MAX
    unsigned int attempts = TMP_MAX;
#else
    unsigned int attempts = ATTEMPTS_MIN;
#endif

    len = strlen (tmpl);
    if (len < 6 || strcmp (&tmpl[len - 6], "XXXXXX"))
    {
        errno = EINVAL;
        return -1;
    }

    /* This is where the Xs start.  */
    XXXXXX = &tmpl[len - 6];

    /* Get some more or less random data.  */
    {
        SYSTEMTIME      stNow;
        FILETIME ftNow;

        // get system time
        GetSystemTime(&stNow);
        stNow.wMilliseconds = 500;
        if (!SystemTimeToFileTime(&stNow, &ftNow))
        {
            errno = -1;
            return -1;
        }

        random_time_bits = (((unsigned long long)ftNow.dwHighDateTime << 32)
                            | (unsigned long long)ftNow.dwLowDateTime);
    }
    value += random_time_bits ^ (unsigned long long)GetCurrentThreadId ();

    for (count = 0; count < attempts; value += 7777, ++count)
    {
        unsigned long long v = value;

        /* Fill in the random bits.  */
        XXXXXX[0] = letters[v % 62];
        v /= 62;
        XXXXXX[1] = letters[v % 62];
        v /= 62;
        XXXXXX[2] = letters[v % 62];
        v /= 62;
        XXXXXX[3] = letters[v % 62];
        v /= 62;
        XXXXXX[4] = letters[v % 62];
        v /= 62;
        XXXXXX[5] = letters[v % 62];

        if (_sopen_s(&fd, tmpl, O_RDWR | O_CREAT | O_EXCL |_O_BINARY , _SH_DENYNO,  _S_IREAD| _S_IWRITE) == 0)
        {
            errno = save_errno;
            return fd;
        }
        else if (errno != EEXIST)
            return -1;
    }

    /* We got out of the loop because we ran out of combinations to try.  */
    errno = EEXIST;
    return -1;
}

int
Downloader::directory_download(void)
{
    char tempfile[17];
    int ret;

    strcpy(tempfile, "/tmp/list.XXXXXX");
    ret = mkstemp(tempfile);
    if(ret < 0){
        return -1;
    }
    close(ret);
    debug_log("Tempfile: %s", tempfile);

    if(plugin->get_dir_list(task, tempfile) < 0){
        unlink(tempfile);
        return -1;
    }

    FILE *fd;
    char buf[1024];
    char buf2[1024];
    char *local_dir = NULL;
    int orig_dir_len = 0;
    char *ptr;

    ptr = (char*)task.get_dir();
    if(ptr != NULL){
        ptr = strrchr(ptr, '/') ? (strrchr(ptr, '/') + 1) : ptr;
    }
    ptr = task.get_local_file() ? (char*)task.get_local_file() : ptr;
    snprintf(buf, 1024, "%s%s%s",
             task.get_local_dir() ? task.get_local_dir() : ".",
             ptr ? "/" : "",
             ptr ? ptr : "");
    if(_mkdir(buf) < 0 && errno != EEXIST){
        cerr<<"Can not create directory : "<<buf<<endl;
        QString temp;
        temp = QString("Can not create directory : ");
        temp += QString(buf);
        emit errorHappened(temp);
        goto _dd_error;
    }
    local_dir = StrDup(buf);
    if(task.get_dir()){
        orig_dir_len = strlen(task.get_dir());
    }
    task.set_local_file(NULL);

    fd = fopen(tempfile, "r");
    task.set_isDirectory(false);
    while(1){
        qint64 size = 0;
        if(fread(&size, sizeof(qint64), 1, fd) != 1) {
            break;
        } else {
            task.set_file_size(size);
        }
        if(fgets(buf, 1024, fd) == NULL) break;
        buf[strlen(buf) - 1] = '\0';
        if(buf[0] == '/'){ // a directory
            snprintf(buf2, 1024, "%s%s",
                     local_dir ? local_dir : ".",
                     orig_dir_len ? buf + orig_dir_len + 1 : buf);
            if(_mkdir(buf2) < 0 && errno != EEXIST){
                cerr<<"Can not create directory : "<<buf2<<endl;
                QString temp;
                temp = QString("Can not create directory : ");
                temp += QString(buf);
                emit errorHappened(temp);
                goto _dd_error;
            }
        }else{ // a file
            cout<<"Download file : "<<buf<<endl;
            snprintf(buf2, 1024, "/%s", buf);
            task.reset_url(buf2);
            ptr = strrchr(buf, '/');
            if(ptr) *ptr = '\0';
            snprintf(buf2, 1024, "%s%s%s",
                     local_dir ? local_dir : ".",
                     orig_dir_len ? "" : "/",
                     orig_dir_len ? buf + orig_dir_len : buf);
            task.set_local_dir(buf2);
            file_download();
        }
    }

    delete[] local_dir;
    unlink(tempfile);
    return 0;

_dd_error:
    delete[] local_dir;
    unlink(tempfile);
    return -1;
}

int
Downloader::file_download(void)
{
    int i;
    int ret = 0;
    QString errorMsg;

    init_local_file_name();
    if(file_exist(localPath)){
        cout<<"File already exist: "<<localPath<<endl;
        errorMsg = QString(tr("The file already exists: "));
        errorMsg += QDir::toNativeSeparators(localPath);
        emit errorHappened(errorMsg);
        return 0;
    }
    cout<<"Begin to download: "
       <<(task.get_local_file() ? task.get_local_file() : task.get_file())<<endl;
    char buf[6];
    double time = get_current_time();
    convert_size(buf, task.get_file_size());
    cout<<"Filesize: "<<buf<<endl;
    emit set_GuiLabelTotal(QString(buf));

    if(task.get_file_size() == 0){
        int fd;
        if( _sopen_s( &fd, localPath, _O_RDWR | _O_CREAT | _O_BINARY, _SH_DENYNO,
                      _S_IREAD | _S_IWRITE ) == 0)
        {
            cerr << "Created data file: " << localPath << endl;
            _close(fd);
            return 0;
        }
        else{
            cerr << "error when creat file: " << localPath << endl;
            emit errorHappened(QString("error when creat file"));
            return -1;
        }
    }

    if(!task.get_resumeSupported() || task.get_file_size() < 0){
        threadNum = 1;
        delete[] blocks;
        blocks = new Block[1];
        blocks[0].size = task.get_file_size();
        blocks[0].bufferFile.open(localMg);
    }else if(file_exist(localMg)){
        ret = init_threads_from_mg();
    }else{
        ret = init_threads_from_info();
    }
    if(ret < 0){
        cerr<<"Init threads failed"<<endl;
        return ret;
    }

    for(i = 0; i < threadNum; i ++){
        if(thread_create() < 0){
            perror("Create thread failed");
            emit errorHappened("Create thread failed");
            return -1;
        }
    }

    qint64 *data;
    data = new qint64[threadNum];

    for(i = 0; i < threadNum; i ++){
        data[i] = blocks[i].startPoint;
    }
    pb->init();

    pb->set_total_size(task.get_file_size());
    cerr << "file_download: task.fileSize" << task.get_file_size();
    emit set_GuiProgressBarMaximum(100);
    pb->set_block_num(threadNum);
    pb->set_start_point(data);

    // update loop
    is_downloading = true;
    while(1){
        if(sigint_received){
            delete[] data;
            save_temp_file_exit();
            return 0;
        }

        for(i = 0; i < threadNum; i ++){
            data[i] = blocks[i].downloaded;
        }
        pb->update(data);
        float downloadedRatio = (float)(pb->get_curr_downloaded()) / (float)(task.get_file_size()) * 100;
        cerr << "downloadedRatio: " << downloadedRatio;
        emit set_GuiProgressBarValue(downloadedRatio);
        emit set_GuiLabelDownloaded(QString(pb->get_downloaded()));
        emit set_GuiLabelSpeed(QString(pb->get_downloadRate()) + QString("/S"));
        emit set_GuiLabelRemainingTime(QString(pb->get_eta()));

        if(schedule() == 0){
            cerr << "all the thread are exit!" << endl;
            break; // all the thread are exit
        }
        usleep(250000);
    }

    delete[] data;
    // recheck the size of the file if possible
    if(task.get_file_size() >= 0){
        qint64 downloaded;
        downloaded = 0;
        for(i = 0; i < threadNum; i ++){
            downloaded += blocks[i].downloaded;
        }
        // the downloaded maybe bigger than the filesize
        // because the overlay of the data
        if(downloaded < task.get_file_size()){
            cerr<<"!!!Some error happend when downloaded"<<endl;
            cerr<<"!!!Redownloading is recommended"<<endl;
            emit errorHappened("!!!Some error happend when downloaded. !!!Redownloading is recommended");
            save_temp_file_exit();
            return (-1);
        }
    }

    if(rename(localMg, localPath) < 0){
        cerr << "Rename failed" << endl;
        emit errorHappened(QString("Rename failed"));
        return -1;
    }
    else
    {
        cerr << "Rename succeed." << endl;
        QFile downloadedFile(localPath);

        if (!downloadedFile.resize(task.get_file_size())) // Do we really need !sigint_received?
        {
            cerr<<"Resize the downloaed file failed"<<endl;
            emit errorHappened(QString("Resize the downloaed file failed"));
        }
    }
    is_downloading = false;

    time = get_current_time() - time;
    convert_time(buf, time);
    emit set_GuiProgressBarValue(100);

    cout<< endl << "Download successfully in "<<buf<<endl;
    errorMsg = QString(tr("Download successfully in "));
    errorMsg += QString(buf);
    emit errorHappened(errorMsg);
    emit done();
    return 0;
}

void
Downloader::run(void)
{
    int ret;

    ret = init_task();  // Maybe we could put this init_task() function into the class Task?
    if(ret < 0){
        cerr<<"Can not get the info of the file "<<endl;
        emit errorHappened(QString(tr("Can not get the info of the file ")));

        return;
    }

    if(task.get_isDirectory()){
        cerr<<"This is a directory: "<<task.get_url()<<endl;
        directory_download();
        return;
    }

    file_download();

    return;
}

void
Downloader::runMyself(QString QUrl)
{
    if (!QUrl.isEmpty()) {
        task.set_url(QUrl.toUtf8().constData());
    }else {
        cerr << "runMyself: QUrl is empty!" << endl;
    }
    start();
}

void
Downloader::resumeTask(void)
{
    sigint_received = false;
    start();
}

void
Downloader::setLocalDirectory(QString QDir)
{
    task.set_local_dir(QDir.toUtf8().constData());
    is_dirSetted = true;
}

void
Downloader::setLocalFileName(QString QFileName)
{
    task.set_local_file(QFileName.toUtf8().constData());
}

void
Downloader::quit(void)
{
    catch_ctrl_c(2);
}

void
Downloader::setThreadNum(int num)
{
    task.set_threadNum(num);
}
