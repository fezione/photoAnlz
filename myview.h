#ifndef MYVIEW_H
#define MYVIEW_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QWheelEvent>
#include <QDebug>
#include "paramImageAfterAnalyze.h"


class myView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit myView( QWidget *parent = nullptr );
    ~myView() {
        delete group_image;
        delete group_faceDot;
        delete group_faceSqr;
        delete group_faceMsk;
        delete scene;
    }

    void setParamAfterAnalyze();            ///< приём параметров после обработки изображения

    void setImage(const QImage &image);     ///< добавление исходного изображения

    void clearScene();                      ///< очистка сцены от старых данных

    void setParamImageAfterAnalyze(const paramImageAfterAnalyze &params );      ///< задаём параметры, полученные после анализа изображения

    void setParamsImageGet(bool newParamsImageGet);

signals:

private slots:
    void slt_updateSceneAfterScale();   ///< обновление сцены после изменений

private:
    void createQGraphicsPixmapItem();       ///< создаём item под изображение

    void createFaceSquare();                ///< создаём полигон для отрисовки положения лица

    void createTextAboutMask();             ///< отображает вероятности наличия маски

    void wheelEvent( QWheelEvent *evnt );   ///< метод изменения масштаба колёсиком мыши

    void resizeParamAfterAnalyze();         ///< подгонка координат ключевых точек изображения к текущему масштабу сцены

    void deleteItemsFromSceneByGroup(QGraphicsItemGroup *gr);     ///< метод удаления элементов из группы со сцены

    bool paramsImageGet;                ///< признак получения параметров анализа изображения

    double curScale;                    ///< масштаб для изменения размеров текста

    int wImage,hImage;                  ///< ширина и высота изображения

    double scaleImageByView;            ///< масштаб подгонки изображения под размер окна

    QVector<QPoint> vec_faceDot;        ///< набор ключевых точек после анализа изображения

    QGraphicsScene *scene;              ///< сцена для отрисовки
    QGraphicsItemGroup *group_image;    ///< item для хранения изображения
    QGraphicsItemGroup *group_faceDot;  ///< item для хранения ключевых точек лица
    QGraphicsItemGroup *group_faceSqr;  ///< item для хранения квадрата лица
    QGraphicsItemGroup *group_faceMsk;  ///< item для хранения признаков наличия маски

    QVector<QGraphicsEllipseItem*> lst_GrpItemfaceDot;   ///< набор ключевых точек в виде item-м лица после анализа изображения

    QImage imageInput;

    paramImageAfterAnalyze paramsImage;
};

#endif // MYVIEW_H
