#include "httpdownloader.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QEventLoop>
#include <QNetworkSession>
#include <qftp.h>
#include <QTimer>
#include "dblogin.h"
HTTPdownloader::HTTPdownloader () {
    failed = false;
    targetfile = NULL;
    fileLink = QString();
    finished = false;
}
HTTPdownloader::~HTTPdownloader () {}

bool HTTPdownloader::getFile (QString link, QString target) {
    fSize = 0;
    failed = false;

    if (link.length () < 5)
        return false;
    fileString = target;
    fileLink = link;
    fileLink.replace (QRegExp (QStringLiteral("(ftp://)*.*[^/]@")), QStringLiteral("ftp://login@")); //Put that RegExp in global var and optimize ftp replace used more than once
    QUrl url (link);
    QNetworkRequest NR (url);
    targetfile = new QFile (target);
    if (!targetfile->open (QFile::WriteOnly)){
        emit MsgCritical(" Modpack Updater", "Konnte Zieldatei nicht öffnen \nFehler: " + targetfile->errorString().replace (QRegExp (QStringLiteral("(ftp://)*.*[^/]@")), QStringLiteral("ftp://login@")) + "\nDatei: " + target);
        //MU->log(QStringLiteral("HTTPdownloader::getMod cant open target File ")+target);
        failed = false;
        targetfile->close ();
        targetfile->remove ();
        emit finishEvent ();
        return 0;
    }
    reply = this->get (NR);
    reply->waitForBytesWritten (200);
    bool error1 = targetfile->error () != QFile::NoError;
    bool error2 = reply->error () != QNetworkReply::NoError;
    bool error3 = reply->attribute (QNetworkRequest::HttpStatusCodeAttribute).toString ().contains ("404") || (reply->attribute (QNetworkRequest::HttpReasonPhraseAttribute).isValid () && reply->attribute (QNetworkRequest::HttpReasonPhraseAttribute).toString ().contains ("Not Found"));
    if (error1 || error2 || error3) {
        if (error1) {
            emit MsgCritical(" Modpack Updater", "Konnte Zieldatei nicht öffnen \nFehler: " + targetfile->errorString ().replace (QRegExp (QStringLiteral("(ftp://)*.*[^/]@")), QStringLiteral("ftp://login@")).append("\nDatei: ").append(target));
        } else if (error2) {
            emit MsgCritical(" Modpack Updater", QString ("Konnte Datei nicht herunterladen \nFehler: ") + reply->errorString ().replace (QRegExp (QStringLiteral("(ftp://)*.*[^/]@")),QStringLiteral("ftp://login@")).append("\nDatei: ").append(fileLink));
        } else if (error3) {
            emit MsgCritical(" Modpack Updater", QString ("Konnte Datei nicht herunterladen \nFehler: 404 - Not Found").append("\nDatei: ").append(fileLink));
        }
        failed = false;
        targetfile->close ();
        targetfile->remove ();
        reply->close ();
        emit finishEvent ();
        return 0;
    }
    connect (reply, SIGNAL (downloadProgress (qint64, qint64)), this, SLOT (progressfile (qint64, qint64)));
    connect (reply, SIGNAL (readyRead ()), SLOT (progressfile ()));
    connect (reply, SIGNAL (finished ()), SLOT (finishfile ()));
    connect (reply, SIGNAL (error (QNetworkReply::NetworkError)), SLOT (error (QNetworkReply::NetworkError)));
    return 1;
}


bool HTTPdownloader::uploadfile (QString file, QString target) {
    QEventLoop loop;
    QTimer timeout;

    QUrl uploadurl("ftp://"+FTPBASE+"/"+target);
    uploadurl.setUserName(FTPUSER);
    uploadurl.setPassword(FTPPW);
    uploadurl.setPort(21);
    QNetworkRequest upload(uploadurl);
    QNetworkAccessManager *uploadman = new QNetworkAccessManager(this);
    connect(uploadman,&QNetworkAccessManager::finished,&loop,&QEventLoop::quit);





    data = new QFile (file, this);
    if (data->open (QIODevice::ReadOnly)) {
        QNetworkReply* reply = uploadman->put(upload, data->readAll());
        connect(reply,&QNetworkReply::uploadProgress,[this](qint64 bytesSent, qint64 bytesTotal){
            if (bytesTotal == 0) return;
            emit setProgress (((float)bytesSent/(float)bytesTotal)*100);
        });
        qDebug() << reply->errorString();
        loop.exec();
        qDebug() << reply->errorString();
        data->close();
        delete data;
        delete uploadman;
        return true;
    } else{
        qDebug () << "Problem" << data->errorString();
        return false;
    }

}

void HTTPdownloader::uploadFinished() {
    qDebug () << "Finished" << reply->error () << reply->errorString ();
    data->deleteLater ();
    reply->deleteLater ();

}


void HTTPdownloader::progressfile (qint64 one, qint64 twi) {
    if (reply->attribute (QNetworkRequest::HttpStatusCodeAttribute).toString ().contains ("404") || (reply->attribute (QNetworkRequest::HttpReasonPhraseAttribute).isValid () && reply->attribute (QNetworkRequest::HttpReasonPhraseAttribute).toString ().contains ("Not Found"))) {
        emit MsgCritical(" Modpack Updater",QString ("Konnte Datei nicht herunterladen \nFehler: 404 - Not Found").append("\nDatei: ").append(fileLink));
        disconnect (reply, SIGNAL (downloadProgress (qint64, qint64)));
        failed = false;
        targetfile->close ();
        targetfile->remove ();
        reply->close ();
        emit finishEvent ();
        return;
    }
    //if (MU->exit){
    //    failed=false;
    //    emit finishEvent ();
    //    return;
    //}

    if (twi == 0) {
        return;
    }
    int prc = ((float) one / twi) * 100;
    emit setProgress (prc);
    if (fSize == 0) {
        emit fileSize (twi);
        fSize = twi;
    }
}


void HTTPdownloader::progressfile () {
    if (reply->attribute (QNetworkRequest::HttpStatusCodeAttribute).toString ().contains ("404") || (reply->attribute (QNetworkRequest::HttpReasonPhraseAttribute).isValid () && reply->attribute (QNetworkRequest::HttpReasonPhraseAttribute).toString ().contains ("Not Found"))) {
        emit MsgCritical(" Modpack Updater",QString ("Konnte Datei nicht herunterladen \nFehler: 404 - Not Found").append("\nDatei: ").append(fileLink));
        disconnect (reply, SIGNAL (downloadProgress (qint64, qint64)));
        failed = false;
        targetfile->close ();
        targetfile->remove ();
        reply->close ();//todo close and delete reply on exit
        emit finishEvent ();
        return;
    }
    emit bytesDownloaded (reply->bytesAvailable ());
    targetfile->write (reply->readAll ());
}

void HTTPdownloader::finishfile () {
    if (reply->attribute (QNetworkRequest::HttpStatusCodeAttribute).toString ().contains ("404") || (reply->attribute (QNetworkRequest::HttpReasonPhraseAttribute).isValid () && reply->attribute (QNetworkRequest::HttpReasonPhraseAttribute).toString ().contains ("Not Found"))) {
        emit MsgCritical(" Modpack Updater",QString ("Konnte Datei nicht herunterladen \nFehler: 404 - Not Found").append("\nDatei: ").append(fileLink));
        failed = false;
        targetfile->close ();
        targetfile->remove ();
        reply->disconnect ();
        reply->close ();
        reply->deleteLater ();
        emit finishEvent ();
        return;
    }
    if (!targetfile->isOpen ()) {
        emit finishEvent ();
        return;
    }


    targetfile->write (reply->readAll ());
    targetfile->close ();
    emit finishEvent ();
}

void HTTPdownloader::error (QNetworkReply::NetworkError code) {
    //qDebug() << "http error" << code;
    if (code == QNetworkReply::ContentAccessDenied || code == QNetworkReply::ContentNotFoundError) { failed = false; } else {
        if (code != 5)
            emit MsgCritical (" Modpack Updater", QString ("Konnte Datei nicht herunterladen \nFehler:") + reply->errorString().replace (QRegExp (QStringLiteral("(ftp://)*.*[^/]@")), QStringLiteral("ftp://login@")).append("\nDatei: ").append(fileLink));

    }
    disconnect (reply, 0, 0, 0);
    //if (!fileLink.contains(QStringLiteral("updater/")))
    //    MU->log ("HTTPError " + QString::number (code) + fileLink);
    if (targetfile != nullptr) {
        targetfile->close ();
    }
    //if (reply->isOpen())
    //reply->abort();
    emit finishEvent ();
    return;
}
