#include "menue.h"
#include <QPainterPath>
#include <QString>
#include <QString>
#include <QRegion>

#include <QPainter>
#include <QRectF>
#include <QTime>


Menu::Menu(QWidget * parent)
    : QMenu(parent) {
}

// void Menu::resizeEvent(QResizeEvent *event)
// {
//     // rounded menu window widget edges
//     int radius = 6;

//     // Use device-independent pixel values
//     qreal pixelRatio = devicePixelRatioF();
//     qreal adjust = 1.0 * pixelRatio;

//     // The rectangle must be translated and adjusted by 1 pixel in order to
//     // correctly map the rounded shape
//     QRectF rect = QRectF(this->rect()).adjusted(adjust, adjust, -adjust, -adjust);

//     QPainterPath path;
//     path.addRoundedRect(rect, radius * pixelRatio, radius * pixelRatio);

//     // QRegion is bitmap-based, so the returned QPolygonF (which uses float
//     // values) must be transformed to an integer-based QPolygon
//     auto region = QRegion(path.toFillPolygon(QTransform()).toPolygon());
//     this->setMask(region);
// }


void Menu::resizeEvent(QResizeEvent *event)
{
    // rounded menue window widget edges
    int radius = 6;
    //qDebug() << __FUNCTION__ << QTime::currentTime();
    QPainterPath path = QPainterPath();

    // the rectangle must be translated and adjusted by 1 pixel in order to
    // correctly map the rounded shape
    QRectF rect = QRectF(this->rect()).adjusted(.5, .5, -1.5, -1.5);

    //QRegion is bitmap based, so the returned QPolygonF (which uses float
    // values must be transformed to an integer based QPolygon
    path.addRoundedRect(rect, radius, radius);

    auto region = QRegion(path.toFillPolygon(QTransform()).toPolygon());



    this->setMask(region);
}
