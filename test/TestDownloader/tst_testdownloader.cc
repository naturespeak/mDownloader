#include <QString>
#include <QtTest>
#include <QThread>
#include <QSignalSpy>
#include "../../downloader.h"
#include "hashworker.h"


#ifdef HAVE_SSL
#	include <openssl/ssl.h>
#endif

#define TEST_HASH "6f2c0ff4e3cab35bb49312ce88e1a690"
#define TEST_URL "10.231.14.85/uploads/burpsuite_free_v1.6.jar"
#define TEMP_FILE "test.test"
#define REPEAT_TIMES 3

class TestDownloader : public QObject
{
    Q_OBJECT

public:
    TestDownloader();
    ~TestDownloader();
    QThread hashWorkerThread;

private Q_SLOTS:
    void initTestCase();
    void testCase1_data();
    void testCase1();
    void init();

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
}

TestDownloader::~TestDownloader()
{
    WSACleanup();
}

void TestDownloader::on_done()
{
    emit operate(TEMP_FILE, "Md5");
}

void TestDownloader::setHash(QString hashVal)
{
    qDebug() << "setHash called: " << hashVal;
    m_hash = hashVal;
}

void TestDownloader::initTestCase()
{
    QFile file(TEMP_FILE);
    file.remove();
    QCOMPARE(file.exists(), false);
}

void TestDownloader::init()
{
    QFile file(TEMP_FILE);
    QCOMPARE(file.exists(), false);
}

void TestDownloader::testCase1_data()
{
    QTest::addColumn<QString>("URLVal");
    QTest::addColumn<QString>("nThreads");
    QTest::addColumn<QString>("hashVal");

//    QString https_qstr = QString("https://") + QString(TEST_URL);
    QString http_qstr = QString("http://") + QString(TEST_URL);
    QString ftp_qstr = QString("ftp://") + QString(TEST_URL);

    for(int i = 1; i < REPEAT_TIMES; i++)
    {
        QString nThreads;
        nThreads.setNum(i);

//        QString https_string = QString("https_") + nThreads + QString("_thread");
        QString http_string = QString("http_") + nThreads + QString("_thread");
        QString ftp_string = QString("ftp_") + nThreads + QString("_thread");

//        QTest::newRow(https_string.toStdString().c_str()) << https_qstr    << nThreads  << TEST_HASH;
        QTest::newRow(http_string.toStdString().c_str()) << http_qstr     << nThreads  << TEST_HASH;
        QTest::newRow(ftp_string.toStdString().c_str()) << ftp_qstr      << nThreads  << TEST_HASH;
    }
}

void TestDownloader::testCase1()
{
    m_hash = QString("");

    QFETCH(QString, URLVal);
    QFETCH(QString, nThreads);
    QFETCH(QString, hashVal);

    dloader = new Downloader();
    dloader->setLocalDirectory(QDir::currentPath());
    dloader->setLocalFileName(TEMP_FILE);
    dloader->setThreadNum(nThreads.toInt());
    connect(dloader, SIGNAL(done()), this, SLOT(on_done()));

    dloader->runMyself(URLVal);

    worker = new HashWorker;
    worker->moveToThread(&hashWorkerThread);
    connect(&hashWorkerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(operate(QString,QString)), worker, SLOT(doHashWork(QString,QString)));
    connect(worker, SIGNAL(resultReady(QString)), this, SLOT(setHash(QString)), Qt::BlockingQueuedConnection);
    hashWorkerThread.start();

    QSignalSpy spy_on_worker(worker, SIGNAL(resultReady(QString)));
    while (spy_on_worker.count() == 0)
    {
        QTest::qWait(200);
    }

    worker->removeFile();

    hashWorkerThread.terminate();
    delete worker;
    delete dloader;

    QCOMPARE(m_hash , QString(hashVal));
}


QTEST_MAIN(TestDownloader)

#include "tst_testdownloader.moc"
