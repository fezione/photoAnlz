#include "myview.h"

myView::myView(QWidget *parent) :
    paramsImageGet(false), wImage(0), hImage(0), scaleImageByView(0)
{
    setAlignment( Qt::AlignCenter );

    setMinimumHeight(parent->height()-10);
    setMinimumWidth(parent->width()-10);

    scene = new QGraphicsScene();
    setScene( scene );

    group_image = new QGraphicsItemGroup();
    group_faceDot = new QGraphicsItemGroup();
    group_faceSqr = new QGraphicsItemGroup();
    group_faceMsk = new QGraphicsItemGroup();

    scene->addItem( group_image );
    scene->addItem( group_faceDot );
    scene->addItem( group_faceSqr );
    scene->addItem( group_faceMsk );
}

void myView::setImage( const QImage &image )
{
    imageInput = image;
    wImage = imageInput.width();
    hImage = imageInput.height();

    qreal temp1 = width()*1.0/wImage;
    qreal temp2 = height()*1.0/hImage;
    if(temp1>temp2)
    {
        scaleImageByView = temp2;
    }
    else
    {
        scaleImageByView = temp1;
    }

    slt_updateSceneAfterScale();
}

void myView::clearScene()
{
    deleteItemsFromSceneByGroup( group_image );
    deleteItemsFromSceneByGroup( group_faceDot );
    deleteItemsFromSceneByGroup( group_faceSqr );
    deleteItemsFromSceneByGroup( group_faceMsk );
    vec_faceDot.clear();
    lst_GrpItemfaceDot.clear();
    paramsImageGet = false;
    paramsImage.clear();
}

void myView::setParamImageAfterAnalyze(const paramImageAfterAnalyze &params)
{
    paramsImage = params;
    paramsImageGet = true;

    vec_faceDot.clear();

    for ( int q = 0; q < paramsImage.cord.size(); ++q ) {
        vec_faceDot << paramsImage.cord.at(q);
    }

    slt_updateSceneAfterScale();
}

void myView::slt_updateSceneAfterScale()
{
    deleteItemsFromSceneByGroup( group_image );
    deleteItemsFromSceneByGroup( group_faceDot );
    deleteItemsFromSceneByGroup( group_faceSqr );
    deleteItemsFromSceneByGroup( group_faceMsk );
    lst_GrpItemfaceDot.clear();

    createQGraphicsPixmapItem();

    scene->setSceneRect( 0, 0, wImage*scaleImageByView, hImage*scaleImageByView );

    if ( paramsImageGet ) {
        createFaceSquare();

        createTextAboutMask();

        resizeParamAfterAnalyze();
        for (int q = 0; q < lst_GrpItemfaceDot.size(); ++q) {
            group_faceDot->addToGroup( lst_GrpItemfaceDot.at(q) );
        }
    }
}

void myView::createQGraphicsPixmapItem()
{
    QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(imageInput));
    pixmapItem->setFlags(QGraphicsItem::ItemIsMovable);
    pixmapItem->setScale( scaleImageByView );

    group_image->addToGroup( pixmapItem );
}

void myView::createFaceSquare()
{
    int x = paramsImage.x*scaleImageByView;
    int y = paramsImage.y*scaleImageByView;
    int wdth = paramsImage.width*scaleImageByView;
    int hght = paramsImage.height*scaleImageByView;

    QPen penRed(Qt::red);

    QPolygon poly;
    poly << QPoint(x,y)
         << QPoint(x + wdth, y)
         << QPoint(x + wdth, y + hght )
         << QPoint( x, y + hght );
    group_faceSqr->addToGroup(scene->addPolygon(poly, penRed) );

}

void myView::createTextAboutMask()
{
    int sizeFont = 16 * curScale/2;
    if ( sizeFont < 16 ) {
        sizeFont = 16;
    }

    QGraphicsTextItem *item = new QGraphicsTextItem("Вероятность маски на лице - " + QString::number(paramsImage.fullFaceMask));
        item->setPos(0,0);
        item->setFont(QFont("Helvetica", sizeFont, QFont::Bold));
        item->setDefaultTextColor( QColor(Qt::red) );
        item->setScale( scaleImageByView );

    QGraphicsTextItem *item1 = new QGraphicsTextItem("Вероятность мед. маски на лице - " + QString::number(paramsImage.lowerFaceMask));
        item1->setPos(0,30*scaleImageByView);
        item1->setFont(QFont("Helvetica", sizeFont, QFont::Bold));
        item1->setDefaultTextColor( QColor(Qt::red) );
        item1->setScale( scaleImageByView );

    QGraphicsTextItem *item2 = new QGraphicsTextItem("Вероятность лицa без маски - " + QString::number(paramsImage.noMask ));
        item2->setPos(0,60*scaleImageByView);
        item2->setFont(QFont("Helvetica", sizeFont, QFont::Bold));
        item2->setDefaultTextColor( QColor(Qt::red) );
        item2->setScale( scaleImageByView );

    QGraphicsTextItem *item3 = new QGraphicsTextItem("Вероятность лица с иным типом маски - " + QString::number(paramsImage.otherMask));
        item3->setPos(0,90*scaleImageByView);
        item3->setFont(QFont("Helvetica", sizeFont, QFont::Bold));
        item3->setDefaultTextColor( QColor(Qt::red) );
        item3->setScale( scaleImageByView );
    group_faceMsk->addToGroup( item );
    group_faceMsk->addToGroup( item1 );
    group_faceMsk->addToGroup( item2 );
    group_faceMsk->addToGroup( item3 );
}

void myView::wheelEvent(QWheelEvent *evnt)
{
    if ( dragMode() != QGraphicsView::ScrollHandDrag )  ///< включаем перетаскивание
    {
        setDragMode( QGraphicsView::ScrollHandDrag );
        setInteractive( true );
    }

    setTransformationAnchor( QGraphicsView::AnchorUnderMouse );
    static const double scaleFactor = 1.15;
    static double currentScale = 1.0;           ///< текущее значение масштаба
    static const double scaleMin = 1.0;         ///< минимальный масштаб

    if ( evnt->delta() > 0 ) {
        scale( scaleFactor, scaleFactor );
        currentScale *= scaleFactor;
        slt_updateSceneAfterScale();
    }else if ( currentScale > scaleMin ) {
        scale( 1/scaleFactor, 1/scaleFactor );
        currentScale /= scaleFactor;
        slt_updateSceneAfterScale();
    }

    curScale = currentScale;
}

void myView::resizeParamAfterAnalyze()
{
    for ( int q = 0; q < vec_faceDot.size(); ++q ) {
        QRect rect( vec_faceDot.at(q).x()*scaleImageByView,
                    vec_faceDot.at(q).y()*scaleImageByView,
                    10*scaleImageByView, 10*scaleImageByView );
        QGraphicsEllipseItem *item = new QGraphicsEllipseItem( rect );
        item->setBrush( QBrush(Qt::red) );
        lst_GrpItemfaceDot.append( item );
    }
}

void myView::deleteItemsFromSceneByGroup(QGraphicsItemGroup *gr)
{
    foreach (QGraphicsItem *item, scene->items( gr->boundingRect() ) ) {
        if ( item->group() == gr ) {
            delete item;
        }
    }
}

void myView::setParamsImageGet(bool newParamsImageGet)
{
    paramsImageGet = newParamsImageGet;
}
