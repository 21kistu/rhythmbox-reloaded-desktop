#ifndef ROUNDEDPIXMAP_H
#define ROUNDEDPIXMAP_H

#include <QGraphicsPixmapItem>
#include <QGraphicsItem>
#include <QPixmap>
#include <QPainter>
#include <QBrush>

class RoundedPixmap : public QGraphicsPixmapItem {
public:

    const char* DEFAULT_PIXMAP = ":/resources/default-graphic.svg";

    /*!
     * \brief RoundedPixmap
     * \param maxSize Minimum sidelength available for viewing image.
     * \param radius Radius of image corner rounding.
     * \param scale Factor to further scale image.
     */
    RoundedPixmap(const char*, int maxSize, int radius, float scale)
        : QGraphicsPixmapItem(), cornerRadius(radius) {

        QPixmap original(DEFAULT_PIXMAP);

        // Square crop to center
        int size = qMin(original.width(), original.height());
        int x = (original.width() - size)/2;
        int y = (original.height() - size)/2;
        QPixmap cropped = original.copy(x, y, size, size);

        // Scale (may need add check to prevent lossy scaling)
        pix = cropped.scaled(maxSize*scale, maxSize*scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        setPixmap(pix);
    }

    /*!
     * \brief paint
     * \param painter
     * \param option
     * \param widget
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
        Q_UNUSED(option);
        Q_UNUSED(widget);
        painter->setRenderHint(QPainter::Antialiasing, true);
        QRectF rect = boundingRect();
        QBrush brush(pixmap());
        painter->setBrush(brush);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(rect, cornerRadius, cornerRadius);
    }

private:
    int cornerRadius;
    QPixmap pix;
};



#endif // ROUNDEDPIXMAP_H
