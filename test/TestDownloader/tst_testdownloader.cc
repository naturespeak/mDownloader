#include <QString>
#include <QtTest>
#include <QThread>
#include <QSignalSpy>
#include "../../downloader.h"
#include "hashworker.h"


#ifdef HAVE_SSL
#	include <openssl/ssl.h>
#endif

#define TEST_HASH "3aced4e6e5740c085d743aa2d042c6a4"
#define TEST_URL "192.168.1.211/test.tar.bz2"

class TestDownloader : public QObject
{
    Q_OBJECT

public:
    TestDownloader();
    ~TestDownloader();
    QThread hashWorkerThread;

private Q_SLOTS:
    void testCase1_data();
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
}

void TestDownloader::testCase1_data()
{
    QTest::addColumn<QString>("URLVal");
    QTest::addColumn<QString>("nThreads");
    QTest::addColumn<QString>("hashVal");

    QString https_qstr = QString("https://") + QString(TEST_URL);
    QString http_qstr = QString("http://") + QString(TEST_URL);
    QString ftp_qstr = QString("ftp://") + QString(TEST_URL);

    QTest::newRow("https_1_thread")  << https_qstr    << "1"  << TEST_HASH;
    QTest::newRow("http_1_thread")   << http_qstr     << "1"  << TEST_HASH;
    QTest::newRow("ftp_1_thread")    << ftp_qstr      << "1"  << TEST_HASH;

    QTest::newRow("https_11_threads")  << https_qstr    << "11"  << TEST_HASH;
    QTest::newRow("http_11_threads")   << http_qstr     << "11"  << TEST_HASH;
    QTest::newRow("ftp_11_threads")    << ftp_qstr      << "11"  << TEST_HASH;

    QTest::newRow("https_10_threads")  << https_qstr    << "10"  << TEST_HASH;
    QTest::newRow("http_10_threads")   << http_qstr     << "10"  << TEST_HASH;
    QTest::newRow("ftp_10_threads")    << ftp_qstr      << "10"  << TEST_HASH;
}

void TestDownloader::testCase1()
{
    QFETCH(QString, URLVal);
    QFETCH(QString, nThreads);
    QFETCH(QString, hashVal);

    m_hash = QString("");

    ::remove("test.test");
    dloader = new Downloader();
    dloader->setLocalDirectory(QDir::currentPath());
    dloader->setLocalFileName("test.test");
    dloader->setThreadNum(nThreads.toInt());
    connect(dloader, SIGNAL(done()), this, SLOT(on_done()));

    dloader->runMyself(URLVal);

    worker = new HashWorker;
    worker->moveToThread(&hashWorkerThread);
    connect(&hashWorkerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(operate(QString,QString)), worker, SLOT(doHashWork(QString,QString)));
    connect(worker, SIGNAL(resultReady(QString)), this, SLOT(setHash(QString)));
    hashWorkerThread.start();

    QSignalSpy spy_on_worker(worker, SIGNAL(resultReady(QString)));
    while (spy_on_worker.count() == 0)
    {
        QTest::qWait(200);
    }
    QVERIFY2(m_hash == QString(hashVal), "Failed: hash not equal!");
    delete worker;
    delete dloader;
    hashWorkerThread.terminate();
    QTest::qWait(500);
}


QTEST_MAIN(TestDownloader)

#include "tst_testdownloader.moc"
