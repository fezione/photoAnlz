#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Анализ изображений");

    ui->centralWidget->setMinimumHeight(480);
    ui->centralWidget->setMinimumWidth(640);

    fileOpenMenu = new QMenu(tr("Изображения"), this);
    act_chsImage = fileOpenMenu->addAction(tr("Выбрать изображение"));
    connect( act_chsImage, SIGNAL(triggered(bool)), this, SLOT(slt_OpenImage()) );
    act_analyzeImage = fileOpenMenu->addAction(tr("Запустить анализ изображения"));
    connect( act_analyzeImage, SIGNAL(triggered(bool)), this, SLOT(slt_analyzeImage()) );
    act_analyzeImage->setEnabled(false);

    tokenMenu = new QMenu( tr("Токен"), this );
    act_setToken = tokenMenu->addAction( tr("Задать токен") );
    connect( act_setToken, SIGNAL(triggered(bool)), this, SLOT(slt_setToken()));
    act_getTokenByValue = tokenMenu->addAction( tr("Получить токен") );
    connect( act_getTokenByValue, SIGNAL(triggered(bool)), this, SLOT(slt_getToken()));

    ui->menuBar->addMenu( tokenMenu );
    ui->menuBar->addMenu( fileOpenMenu );
    ui->mainToolBar->hide();

    parser = new parserjson( this );
    view = new myView(this);

    prBar = new QProgressBar(this);

    te_inform = new QTextEdit(this);
    te_inform->setReadOnly(true);
    te_inform->setMaximumHeight(80);

    connect( parser, SIGNAL(sgn_getTokenHappily()), this, SLOT(slt_getTokenHappily()) );
    connect( parser, SIGNAL(sgn_analyzeImageHappily()), this, SLOT(slt_analyzeImageHappily()));
    connect( parser, SIGNAL(sgn_progressChanged(int)), this, SLOT( slt_progressChanged(int)));
    connect( parser, SIGNAL(sgn_Error()), this, SLOT(slt_parserParamConnection()));
    connect( parser, SIGNAL(sgn_NoFaceOnImage()), this, SLOT(slt_noFaceOnImage()));
    connect( parser, SIGNAL(sgn_tokenIsEmpty()), this, SLOT( slt_tokenIsEmpty()));

    QVBoxLayout *lay = new QVBoxLayout(this);
        lay->addWidget( view );
        lay->addWidget( prBar );
        lay->addWidget( te_inform );

    ui->centralWidget->setLayout(lay);

    razdelStr = "----------------------------------";
}

MainWindow::~MainWindow()
{
    delete parser;
    delete ui;
}

void MainWindow::initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

bool MainWindow::loadImage(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    imageInput = newImage;
    fileNameImageInput = fileName;

    act_analyzeImage->setEnabled(true);

    return true;
}

void MainWindow::appendImageOnView()
{
    view->clearScene();
    view->setImage( imageInput );
    view->show();
}

bool MainWindow::checkBoxTextToBool(const QString &str)
{
     return (str == "true") ? true : false;
}

void MainWindow::slt_OpenImage()
{
    act_analyzeImage->setEnabled(false);

    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadImage(dialog.selectedFiles().first())) {}

    appendImageOnView();
}

void MainWindow::slt_analyzeImage()
{
    view->setParamsImageGet(false);

    QDialog *dial = new QDialog(this);
    dial->setWindowTitle(tr("Параметры анализа"));

    QLabel *lab_minHeight = new QLabel(tr("Минимальная высота лица, которую будет искать детектор"));
    QLabel *lab_maxHeight = new QLabel(tr("Максимальная высота лица, которое будет искать детектор.\nПРИМЕЧАНИЕ: нулевое значение отключает это ограничение."));
    QLabel *lab_threshold = new QLabel(tr("Пороговое значение, которое будет применяться к найденным лицам.\nПРИМЕЧАНИЕ: меньшие значения увеличивают отклик детектора, но снижают точность."));
    QLabel *lab_rotate = new QLabel(tr("Передайте true, чтобы применить поворот к входному изображению\nдо тех пор, пока не будут найдены некоторые лица,\nпорядок поворота: против часовой стрелки 90 градусов,\nпо часовой стрелке 90 градусов, 180 градусов"));
    QLabel *lab_orientation = new QLabel(tr("Передайте true, чтобы применить классификатор ориентации,\nчтобы попытаться исправить фотографии лица с неправильной\nориентацией до обнаружения."));
    QLabel *lab_demographic = new QLabel(tr("Передайте true, чтобы включить демографическую информацию\nв ответ"));
    QLabel *lab_attributes = new QLabel(tr("Передайте true, чтобы включить информацию об атрибутах лица\nв ответ"));
    QLabel *lab_landmarks = new QLabel(tr("Передайте значение true, чтобы включить в ответ положения\nориентиров лица"));
    QLabel *lab_liveness = new QLabel(tr("Передайте true, чтобы включить вероятность фотографии живого\nчеловека в ответ"));
    QLabel *lab_quality = new QLabel(tr("Передайте true, чтобы включить качество фотографии человека\nв ответ"));
    QLabel *lab_masks = new QLabel(tr("Передайте true, чтобы включить маску фотографии человека\nв ответ"));

    QLineEdit *le_minHeight = new QLineEdit( "0", dial);
        le_minHeight->setInputMask("9999");
    QLineEdit *le_maxHeight = new QLineEdit( "0",dial);
        le_maxHeight->setInputMask("9999");
    QLineEdit *le_threshold = new QLineEdit("0.8", dial);
        le_threshold->setInputMask("9.999");
    QComboBox *cbox_rotate = new QComboBox(dial);
        cbox_rotate->addItem("true");
        cbox_rotate->addItem("false");
    QComboBox *cbox_orientation = new QComboBox(dial);
        cbox_orientation->addItem("true");
        cbox_orientation->addItem("false");
    QComboBox *cbox_demographic = new QComboBox(dial);
        cbox_demographic->addItem("true");
        cbox_demographic->addItem("false");
    QComboBox *cbox_attributes = new QComboBox(dial);
        cbox_attributes->addItem("true");
        cbox_attributes->addItem("false");
    QComboBox *cbox_landmarks = new QComboBox(dial);
        cbox_landmarks->addItem("true");
        cbox_landmarks->addItem("false");
    QComboBox *cbox_liveness = new QComboBox(dial);
        cbox_liveness->addItem("true");
        cbox_liveness->addItem("false");
    QComboBox *cbox_quality = new QComboBox(dial);
        cbox_quality->addItem("true");
        cbox_quality->addItem("false");
    QComboBox *cbox_masks = new QComboBox(dial);
        cbox_masks->addItem("true");
        cbox_masks->addItem("false");

    QPushButton *but = new QPushButton(tr("Запуск анализа"),dial);
    connect( but, SIGNAL(clicked(bool)), dial, SLOT(close()));

    QGridLayout *lay = new QGridLayout(dial);
        lay->addWidget( lab_minHeight, 0, 0);
        lay->addWidget( le_minHeight, 0, 1 );
        lay->addWidget( lab_maxHeight, 1, 0 );
        lay->addWidget( le_maxHeight, 1, 1);
        lay->addWidget( lab_threshold, 2, 0);
        lay->addWidget( le_threshold, 2, 1);
        lay->addWidget( lab_rotate, 3, 0);
        lay->addWidget( cbox_rotate, 3, 1);
        lay->addWidget( lab_orientation, 4, 0);
        lay->addWidget( cbox_orientation, 4, 1);
        lay->addWidget( lab_demographic, 5, 0);
        lay->addWidget( cbox_demographic, 5, 1);
        lay->addWidget( lab_attributes, 6, 0);
        lay->addWidget( cbox_attributes, 6, 1);
        lay->addWidget( lab_landmarks, 7, 0);
        lay->addWidget( cbox_landmarks, 7, 1);
        lay->addWidget( lab_liveness, 8, 0);
        lay->addWidget( cbox_liveness, 8, 1);
        lay->addWidget( lab_quality, 9, 0);
        lay->addWidget( cbox_quality, 9, 1);
        lay->addWidget( lab_masks, 10, 0);
        lay->addWidget( cbox_masks, 10, 1);
        lay->addWidget( but, 11, 0, 1, 2);
    dial->setLayout(lay);

    dial->exec();

    if ( le_minHeight->text().isEmpty() || le_maxHeight->text().isEmpty() || le_threshold->text().isEmpty() ) {
        QMessageBox msgBox;
            msgBox.setWindowFlags( Qt::WindowType_Mask | Qt::WindowCloseButtonHint );
            msgBox.setWindowTitle( "Информация" );
            msgBox.setText("Введите корректные данные");
            msgBox.exec();
    }else{
        inputJsonData data;
            data.sign_rotate = cbox_rotate->currentText();
            data.sign_orientation = cbox_orientation->currentText();
            data.sign_demographic = cbox_demographic->currentText();
            data.sign_attributes = cbox_attributes->currentText();
            data.sign_landmarks = cbox_landmarks->currentText();
            data.sign_liveness = cbox_liveness->currentText();
            data.sign_quality = cbox_quality->currentText();
            data.sign_masks = cbox_masks->currentText();
            data.minHeight = le_minHeight->text().toInt();
            data.maxHeight = le_maxHeight->text().toInt();
            data.threshold = le_threshold->text().toDouble();
        parser->setParamUrlAndImage(data, fileNameImageInput );
    }
}

void MainWindow::slt_getToken()
{
    QDialog *dial = new QDialog(this);
    dial->setWindowTitle( tr("Получить токен по адресу почты и паролю") );

    QLabel *lab_mail = new QLabel( tr("Почта"), dial );
    QLabel *lab_pass = new QLabel( tr("Пароль"), dial );

    QLineEdit *le_mail = new QLineEdit( dial );
    QLineEdit *le_pass = new QLineEdit( dial );

    QPushButton *but = new QPushButton( tr("Получить токен"), dial );
    connect( but, SIGNAL(clicked(bool)), dial, SLOT(close()));

    QGridLayout *lay = new QGridLayout(dial);
        lay->addWidget( lab_mail, 0, 0);
        lay->addWidget( le_mail, 0, 1 );
        lay->addWidget( lab_pass, 1, 0);
        lay->addWidget( le_pass, 1, 1 );
        lay->addWidget( but, 2, 0, 1, 2);
    dial->setLayout( lay );

    dial->exec();

    if ( !le_mail->text().isEmpty() && !le_pass->text().isEmpty() ) {
        parser->setDataForToken( le_mail->text(), le_pass->text() );
    }
}

void MainWindow::slt_setToken()
{
    QDialog *dial = new QDialog(this);
    dial->setWindowTitle( tr("Задать токен") );

    QLabel *lab_token = new QLabel(tr("Введите токен"),dial);

    QLineEdit *le_token = new QLineEdit(dial);

    QPushButton *but = new QPushButton( tr("Ввод"), dial );
    connect( but, SIGNAL(clicked(bool)), dial, SLOT(close()));

    QGridLayout *lay = new QGridLayout(dial);
        lay->addWidget( lab_token, 0, 0 );
        lay->addWidget( le_token, 0, 1 );
        lay->addWidget( but, 1,0, 1, 2 );
    dial->setLayout( lay );

    dial->exec();

    if ( !le_token->text().isEmpty() ) {
        parser->setToken( le_token->text() );
    }else{
    }
}

void MainWindow::slt_parserParamConnection()
{
    te_inform->append( "Код-тип соединения - " + QString::number(parser->getSign_reply()) );
    te_inform->append( "Ошибка соединения с сервером - " + parser->getSignServerError());
    te_inform->append( "Ошибка анализа Json - " + parser->getSignJsonError() );
    te_inform->append(razdelStr);
}

void MainWindow::slt_noFaceOnImage()
{
    QMessageBox msgBox;
        msgBox.setWindowFlags( Qt::WindowType_Mask | Qt::WindowCloseButtonHint );
        msgBox.setWindowTitle( "Информация" );
        msgBox.setText("Не найдено лиц на изображении.");
        msgBox.exec();
}

void MainWindow::slt_getTokenHappily()
{
    te_inform->append( "Токен успешно получен" );
    fileOpenMenu->show();
}

void MainWindow::slt_analyzeImageHappily()
{
    te_inform->append( "Изображение успешно проанализированно" );
    view->setImage( imageInput );
    view->setParamImageAfterAnalyze( parser->getMarksFace() );
}

void MainWindow::slt_tokenIsEmpty()
{
    QMessageBox msgBox;
        msgBox.setWindowFlags( Qt::WindowType_Mask | Qt::WindowCloseButtonHint );
        msgBox.setWindowTitle( "Информация" );
        msgBox.setText("Токен для анализа не получен.\nВведите новые данные пользователя или задайте токен.");
        msgBox.exec();
}

void MainWindow::slt_progressChanged(int val)
{
    prBar->setValue( val );
}
