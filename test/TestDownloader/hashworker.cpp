#include "hashworker.h"
#include "../../macro.h"
//#include <QDateTime>
#include <QDebug>

HashWorker::HashWorker(QObject *parent) :
    QObject(parent),
    m_currentProgress(0)
{
}

void HashWorker::doHashWork(QString FileName, QString HashAlgorithm)
{
    QByteArray resultByteArray;
    QByteArray hashHexByteArray;
    QString resultQString = "";
//    QString resultQString = QDateTime::currentDateTime().toString(QString("[hh:mm:ss, ddd MMMM d yyyy]\t")) + "["
//            + FileName + "]" + "\t";
    QCryptographicHash *hash = NULL;

    QFile *file = new QFile(FileName, this);

    if(file->open(QIODevice::ReadOnly)) {   //ToDo: what if the file is a directory?
        if (HashAlgorithm == "Md5")
        {
            hash = new QCryptographicHash(QCryptographicHash::Md5);
        }
        else if (HashAlgorithm == "Sha1")
        {
            hash = new QCryptographicHash(QCryptographicHash::Sha1);
        }
#ifdef QT5_ABOVE
        else if (HashAlgorithm == "Sha256")
        {
            hash = new QCryptographicHash(QCryptographicHash::Sha256);
        }
#endif
        int steps = 0;
        while (!file->atEnd()) {
            hash->addData(file->read(readingBlockSize));
            m_currentProgress += readingBlockSize;
            emit currentProgress(++steps);
        }
        emit currentProgress(file->size()/readingBlockSize);
        resultByteArray = hash->result();
        delete hash;
        hashHexByteArray = resultByteArray.toHex();

//        if (HashAlgorithm == "Md5")
//        {
//            resultQString += "[MD5]\t";
//        }
//        else if (HashAlgorithm == "Sha1")
//        {
//            resultQString += "[SHA1]\t";
//        }
//        else if (HashAlgorithm == "Sha256")
//        {
//            resultQString += "[SHA256]\t";
//        }
//        resultQString += "[";
        resultQString += hashHexByteArray.data();
//        resultQString += "]";
    }else {
        // file open error
    }

    emit resultReady(resultQString);
}
