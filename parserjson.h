#ifndef PARSERJSON_H
#define PARSERJSON_H

#include <QObject>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPoint>
#include <QFile>
#include "paramImageAfterAnalyze.h"

class parserjson : public QObject
{
    Q_OBJECT
public:
    explicit parserjson(QObject *parent = nullptr);
    ~parserjson() {
        delete reply;
        delete manager;
        delete file;
    }

    void setDataForToken(const QString &mail, const QString &pass ); ///< получаем данные, по которым получим токен

    void setParamUrlAndImage(const inputJsonData &data, const QString &pathImg);     ///< задаём параметры запроса и изображение для анализа

    paramImageAfterAnalyze getMarksFace() const;

    int getSign_reply() const;

    const QString getSignJsonError() const;

    const QString getSignServerError() const;

    void setToken(const QString &newToken);

signals:
    void sgn_getTokenHappily();

    void sgn_analyzeImageHappily();

    void sgn_progressChanged(int);

    void sgn_Error();

    void sgn_NoFaceOnImage();

    void sgn_tokenIsEmpty();

private slots:
    void slt_finishedReply( QNetworkReply *reply);

    void slt_progressChanged(qint64 a, qint64 b);

private:

    void createNetworkForToken();

    bool checkNetWorkReplyError(QNetworkReply *reply);          ///< проверяем, есть ли ошибки при соединении с сервером

    bool checkInputJsonDocument( const QByteArray &bytes );     ///< проверяем, пришёл ли читабельный json

    bool getToken(QNetworkReply *reply);                        ///< получаем токен

    bool getParamAnalyzeImage(QNetworkReply *reply);            ///< получаем параметры анализа изображения

    bool checkConnectPrBarByReply;

    int typeConnection;
    int sign_reply;                 ///< код-тип соединения
    QString token;
    QString signJsonError;          ///< тип ошибки Json
    QString signServerError;        ///< тип ошибки соединения
    QString mailUser, passUser;     ///< параметры пользователя

    paramImageAfterAnalyze marksFace;

    QFile *file;

    QNetworkAccessManager *manager = nullptr;
    QNetworkReply *reply = nullptr;
};
#endif // PARSERJSON_H
