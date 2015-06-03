#ifndef HASHWORKER_H
#define HASHWORKER_H

#include <QObject>
#include <QCryptographicHash>
#include <QString>
#include <QFile>

class HashWorker : public QObject
{
    Q_OBJECT

public:
    explicit HashWorker(QObject *parent = 0);

public slots:
    void doHashWork(QString FileName, QString HashAlgorithm);

signals:
    void resultReady(QString result);
    void currentProgress(int progress);

private:
    int m_currentProgress;
};

#endif // HASHWORKER_H
