#ifndef HTTPDOWNLOADER_H
#define HTTPDOWNLOADER_H

#include <QObject>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QEventLoop>
class HTTPdownloader : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit HTTPdownloader();
    ~HTTPdownloader();
    bool getFile(QString link,QString target);
    bool uploadfile(QString file, QString target);
    QNetworkReply *reply;
    bool finished;
    QString returnString;
    QString fileString;
    QString fileLink;
    QFile *targetfile;
    bool failed;
    QFile * data;
    int fSize;
signals:
    void finishEvent();
    void MsgCritical(QString title,QString message);
    void setProgress(int prc);
    void bytesDownloaded(quint64 one);
    void fileSize(quint64);
public slots:
    void progressfile(qint64 one, qint64 twi);
    void progressfile();
    void error(QNetworkReply::NetworkError code);
    void finishfile();
    void uploadFinished();
private:

};

#endif // HTTPDOWNLOADER_H
