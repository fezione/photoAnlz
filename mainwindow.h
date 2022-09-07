#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHttpPart>
#include <QHttpMultiPart>
#include <QFile>
#include <QTextCodec>
#include <QMessageBox>
#include <QUrlQuery>
#include <QUrl>

#include "parserjson.h"
#include "myview.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:

    void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode);

    bool loadImage(const QString &fileName);

    void appendImageOnView();                           ///< добавление изображения на view

    bool checkBoxTextToBool(const QString &str);

    Ui::MainWindow *ui;

    QMenu *fileOpenMenu;
    QMenu *tokenMenu;

    QAction *act_chsImage;
    QAction *act_analyzeImage;
    QAction *act_setToken;
    QAction *act_getTokenByValue;

    QProgressBar *prBar;

    QTextEdit *te_inform;               ///< текстовое поле для отображения результатов соединения с сервером и парсинга Json документа

    QImage imageInput;                  ///< выбранное изображение

    QString fileNameImageInput;         ///< путь к изображению

    parserjson *parser;

    myView *view;

    QString razdelStr;                  ///< набор символов для разделения пакетов данных в текстовом поле

private slots:

    void slt_OpenImage();
    void slt_analyzeImage();
    void slt_getToken();
    void slt_setToken();

    void slt_parserParamConnection();

    void slt_noFaceOnImage();

    void slt_getTokenHappily();

    void slt_analyzeImageHappily();

    void slt_tokenIsEmpty();

    void slt_progressChanged(int val);
};

#endif // MAINWINDOW_H
