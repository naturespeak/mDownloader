#include <QString>
#include <QtTest>
#include "../../header.h"

class HeaderTest : public QObject
{
    Q_OBJECT

public:
    HeaderTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCase1_data();
    void testCase1();

private:
    HeadData *t_header;
};

HeaderTest::HeaderTest()
{
}

void HeaderTest::initTestCase()
{
    t_header = new HeadData;
}

void HeaderTest::cleanupTestCase()
{
    delete t_header;
}

void HeaderTest::testCase1_data()
{
    QTest::addColumn<QString>("attrName");
    QTest::addColumn<QString>("attrValue");
    QTest::newRow("set_attr") << QString("Connection") << QString("close");
}

void HeaderTest::testCase1()
{
    QFETCH(QString, attrName);
    QFETCH(QString, attrValue);
    t_header->set_attr(attrName.toStdString().c_str(), attrValue.toStdString().c_str());

    QCOMPARE(QString(t_header->head->attrName), QString("Connection"));
    QCOMPARE(QString(t_header->head->attrValue), QString("close"));
    QVERIFY2(t_header->head->next == NULL, "next is not NULL");
}

QTEST_APPLESS_MAIN(HeaderTest)

#include "tst_headertest.moc"
