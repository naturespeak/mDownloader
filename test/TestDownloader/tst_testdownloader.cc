#include <QString>
#include <QtTest>
#include <QThread>
#include <QSignalSpy>
#include "../../downloader.h"
#include "hashworker.h"


#ifdef HAVE_SSL
#	include <openssl/ssl.h>
#endif


class TestDownloader : public QObject
{
    Q_OBJECT

public:
    TestDownloader();
    ~TestDownloader();
    QThread hashWorkerThread;

private Q_SLOTS:
     void testCase1();

signals:
    void operate(QString, QString);

public slots:
    void on_done();
    void setHash(QString);

private:
    QString m_hash;
    Downloader *dloader;
    HashWorker *worker;
};

TestDownloader::TestDownloader()
{
    WSADATA     wsaData;
    WSAStartup(0x0202, &wsaData);

    #ifdef HAVE_SSL
    SSL_load_error_strings();
    SSLeay_add_ssl_algorithms();
    #endif
    m_hash = QString("");

    ::remove("test.test");
    dloader = new Downloader();
    dloader->setLocalDirectory(QDir::currentPath());
    dloader->setLocalFileName("test.test");
    dloader->setThreadNum(11);
    connect(dloader, SIGNAL(done()), this, SLOT(on_done()));
    dloader->runMyself("http://qinchuan.me/downloads/mDownloader-Setup-1.0.1Build005.exe");

    worker = new HashWorker;
    worker->moveToThread(&hashWorkerThread);
    connect(&hashWorkerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(operate(QString,QString)), worker, SLOT(doHashWork(QString,QString)));
    connect(worker, SIGNAL(resultReady(QString)), this, SLOT(setHash(QString)));
    hashWorkerThread.start();
}

TestDownloader::~TestDownloader()
{
    WSACleanup();
}

void TestDownloader::on_done()
{
    emit operate("test.test", "Md5");
}

void TestDownloader::setHash(QString hashVal)
{
    m_hash = hashVal;
    hashWorkerThread.terminate();
}

void TestDownloader::testCase1()
{
    QSignalSpy spy_on_worker(worker, SIGNAL(resultReady(QString)));
    while (spy_on_worker.count() == 0)
    {
        QTest::qWait(200);
    }
    QVERIFY2(m_hash == QString("ff21716e7d15774c2d52e51b02ed5994"), "Failed: hash not equal!");
}


QTEST_MAIN(TestDownloader)

#include "tst_testdownloader.moc"
