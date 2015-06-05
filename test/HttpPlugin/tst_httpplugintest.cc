#include <QString>
#include <QtTest>
#include <QCoreApplication>

class HttpPluginTest : public QObject
{
    Q_OBJECT

public:
    HttpPluginTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCase1_data();
    void testCase1();
};

HttpPluginTest::HttpPluginTest()
{
}

void HttpPluginTest::initTestCase()
{
}

void HttpPluginTest::cleanupTestCase()
{
}

void HttpPluginTest::testCase1_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

void HttpPluginTest::testCase1()
{
    QFETCH(QString, data);
    QVERIFY2(true, "Failure");
}

QTEST_MAIN(HttpPluginTest)

#include "tst_httpplugintest.moc"
