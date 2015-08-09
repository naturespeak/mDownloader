#include <QString>
#include <QtTest>
#include "../../header.h"

class HeaderTest : public QObject
{
    Q_OBJECT

public:
    HeaderTest();

private Q_SLOTS:
    void init();
    void cleanup();
    void test_set_attr_data();
    void test_set_attr();
    void test_get_attr_data();
    void test_get_attr();

private:
    HeadData *t_header;
};

HeaderTest::HeaderTest()
{
}

void HeaderTest::init()
{
    t_header = new HeadData;
}

void HeaderTest::cleanup()
{
    delete t_header;
}

void HeaderTest::test_set_attr_data()
{
    QTest::addColumn<QString>("attrName");
    QTest::addColumn<QString>("attrValue");
    QTest::newRow("set_attr") << QString("Connection") << QString("close");
}

void HeaderTest::test_set_attr()
{
    QFETCH(QString, attrName);
    QFETCH(QString, attrValue);
    t_header->set_attr(attrName.toStdString().c_str(), attrValue.toStdString().c_str());

    QCOMPARE(QString(t_header->head->attrName), QString("Connection"));
    QCOMPARE(QString(t_header->head->attrValue), QString("close"));
    QVERIFY2(t_header->head->next == NULL, "next is not NULL");
}

void HeaderTest::test_get_attr_data()
{
    QTest::addColumn<QString>("attrName");
    QTest::addColumn<QString>("attrValue");
    QTest::newRow("get_attr") << QString("Connection") << QString("close");
}

void HeaderTest::test_get_attr()
{
    QFETCH(QString, attrName);
    QFETCH(QString, attrValue);
    t_header->set_attr(attrName.toStdString().c_str(), attrValue.toStdString().c_str());

    QCOMPARE(QString(t_header->get_attr("Connection")), QString("close"));
}

QTEST_APPLESS_MAIN(HeaderTest)

#include "tst_headertest.moc"
