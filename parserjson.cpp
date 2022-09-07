#include "parserjson.h"

parserjson::parserjson(QObject *parent) :
    QObject(parent), sign_reply(0), token(""), signJsonError(""),
    signServerError(""), mailUser(""), passUser(""), checkConnectPrBarByReply(false)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(slt_finishedReply(QNetworkReply *)));

    file = new QFile();
}

void parserjson::setDataForToken(const QString &mail, const QString &pass)
{
    mailUser = mail;
    passUser = pass;

    createNetworkForToken();
}

void parserjson::createNetworkForToken()
{
    QString baseUrl = "https://backend.facecloud.tevian.ru/api/v1/login";
    QUrl url(baseUrl);

    QJsonObject json;
        json.insert("email", mailUser);
        json.insert("password", passUser);

    QJsonDocument document;
        document.setObject(json);
    QByteArray dataArray = document.toJson(QJsonDocument::Compact);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setUrl(url);

    typeConnection = 100;

    reply = manager->post( request, dataArray );
    if ( !checkConnectPrBarByReply ) {
        connect(reply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(slt_progressChanged(qint64, qint64)));
    }
}

void parserjson::setParamUrlAndImage( const inputJsonData &data, const QString &pathImg )
{
    if ( token == "" ) {
        emit sgn_tokenIsEmpty();
    }else{
        QString urlByJson;
        urlByJson.append( QString("https://backend.facecloud.tevian.ru/api/v1/detect?")
                          + "fd_min_size="  + QString::number(data.minHeight)
                          + "&fd_max_size="  + QString::number(data.maxHeight)
                          + "&fd_threshold=" + QString::number(data.threshold)
                          + "&rotate_until_faces_found=" + data.sign_rotate
                          + "&orientation_classifier=" + data.sign_orientation
                          + "&demographics=" + data.sign_demographic
                          + "&attributes="   + data.sign_attributes
                          + "&landmarks="    + data.sign_landmarks
                          + "&liveness="     + data.sign_liveness
                          + "&quality="      + data.sign_quality
                          + "&masks="        + data.sign_masks);

        QUrl url(urlByJson);

        QNetworkRequest request(url);
        request.setRawHeader(QByteArray("Authorization"),
                                        QString("Bearer " + token).toUtf8());
        request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));

        file->setFileName(pathImg);
        if (!file->exists() || !file->open(QIODevice::ReadOnly)) {
            return;
        }

        typeConnection = 101;

        reply = manager->post( request, file );
        if ( !checkConnectPrBarByReply ) {
            connect(reply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(slt_progressChanged(qint64, qint64)));
        }
    }
}

void parserjson::slt_finishedReply(QNetworkReply *reply)
{
    if ( checkNetWorkReplyError(reply) ) {
        if ( typeConnection == 100 ) {
            if ( getToken(reply) ) {
                emit sgn_getTokenHappily();
            }
        }else if ( typeConnection == 101 ) {
            if ( getParamAnalyzeImage(reply) ) {
                emit sgn_analyzeImageHappily();
            }
        }
    }

    if ( file->isOpen() ) {
        file->close();
    }

    emit sgn_progressChanged(100);

    emit sgn_Error();
}

void parserjson::slt_progressChanged(qint64 a, qint64 b)
{
    if (b > 0) {
        emit sgn_progressChanged( 100*a/b );
    }
}

bool parserjson::checkNetWorkReplyError(QNetworkReply *reply)
{
    QNetworkReply::NetworkError error = reply->error();
    if ( !error ) {
        signServerError = "Соединение с сервером успешно";
    }else{
        signServerError = "Соедениние с сервером неудачно";
    }

    QVariant variant = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if ( variant.isValid() ) {
        sign_reply = variant.toInt();
    }

    return true;
}

bool parserjson::checkInputJsonDocument(const QByteArray &bytes)
{
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson( bytes, &jsonError );
    if ( jsonError.error != QJsonParseError::NoError ) {
        signJsonError = "Не удалось проанализировать Json";
        return false;
    }
    return true;
}

bool parserjson::getToken( QNetworkReply *reply )
{
    QByteArray bytes = reply->readAll();

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson( bytes, &jsonError );
    if ( jsonError.error != QJsonParseError::NoError ) {
        signJsonError = "Не удалось проанализировать Json";
        return false;
    }

    if ( doc.isObject() ) {

        QJsonValue value = doc.object().value("data");

        if ( value.isObject() ) {
            QJsonObject obj = value.toObject();
            token = obj["access_token"].toString();
            signJsonError = "Json успешно проанализирован";
        }else{
            signJsonError = "Не удалось проанализировать Json";
            emit sgn_tokenIsEmpty();
            return false;
        }
    }else{
        signJsonError = "Не удалось проанализировать Json";
        emit sgn_tokenIsEmpty();
        return false;
    }
    return true;
}

bool parserjson::getParamAnalyzeImage( QNetworkReply *reply )
{
    QByteArray bytes = reply->readAll();

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        signJsonError = "Не удалось проанализировать Json";
        return false;
    }

    if ( doc.isObject() ) {

        QJsonObject json = doc.object();

        QJsonArray jsonArray = json["data"].toArray();
        QJsonValue value = jsonArray.first();
        if (value.isObject())
        {
            QJsonObject obj = value.toObject();

            int height, width, x, y;
            double fullMask, lowerMask, noMask, otherMask;
            QVector<QPoint> vec;

            if ( obj.contains("bbox") ) {
                QJsonValue ara;
                ara = obj.take("bbox");
                height = ara["height"].toInt();
                width = ara["width"].toInt();
                x = ara["x"].toInt();
                y = ara["y"].toInt();
            }

            if ( obj.contains("landmarks") ) {
                QJsonArray ara;
                ara = obj.take("landmarks").toArray();
                for ( int q = 0; q < ara.size(); ++q ) {
                    QJsonValue val = ara.at(q);
                    int x = val["x"].toInt();
                    int y = val["y"].toInt();
                    vec.append( QPoint( x, y ) );
                }
            }

            if ( obj.contains("masks") ) {
                QJsonValue maskVal;
                maskVal = obj.take("masks");
                fullMask = maskVal["full_face_mask"].toDouble();
                lowerMask = maskVal["lower_face_mask"].toDouble();
                noMask = maskVal["no_mask"].toDouble();
                otherMask = maskVal["other_mask"].toDouble();
            }

            if ( fullMask < 0.0001 ) {
                fullMask = 0.0;
            }

            if ( lowerMask < 0.0001 ) {
                lowerMask = 0.0;
            }

            if ( noMask < 0.0001 ) {
                noMask = 0.0;
            }

            if ( otherMask < 0.0001 ) {
                otherMask = 0.0;
            }

            marksFace.setData( width, height, x, y, fullMask,
                               lowerMask, noMask, otherMask,
                               vec );
            signJsonError = "Данные успешно получены";
        }else{
            signJsonError = "Информация о найденных лицах на изображении не получена";
            emit sgn_NoFaceOnImage();
            return false;
        }
    }
    return true;
}

void parserjson::setToken(const QString &newToken)
{
    token = newToken;
}

const QString parserjson::getSignServerError() const
{
    return signServerError;
}

const QString parserjson::getSignJsonError() const
{
    return signJsonError;
}

int parserjson::getSign_reply() const
{
    return sign_reply;
}

paramImageAfterAnalyze parserjson::getMarksFace() const
{
    return marksFace;
}
