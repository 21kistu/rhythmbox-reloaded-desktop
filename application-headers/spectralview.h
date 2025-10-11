#ifndef SPECTRALVIEW_H
#define SPECTRALVIEW_H

#include <QWidget>
#include <QFile>
#include <QScrollBar>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QScreen>
#include <QProcess>
#include <QStringList>
#include <QPalette>
#include <QColor>
#include <QPropertyAnimation>

#include "playlistmanager.h"

class SpectralView : public QWidget
{
public:

    std::string spectralPath;

    QGraphicsView* view;

    SpectralView();

    /*!
     * \brief SpectralView
     */
    SpectralView(std::string, PlaylistManager::Track);

    /*!
     * \brief getTransformedPixmap
     * \param filePath File location of spectral.
     * \return Scaled image.
     */
    QGraphicsPixmapItem* getTransformedPixmap(std::string filePath);

    /*!
     * \brief updateSpectral
     * \param playlistName Name of playlist where spectral is stored.
     * \param track Track object.
     */
    void updateSpectral(std::string playlistName, PlaylistManager::Track track);

    /*!
     * \brief createSpectral
     * \param playlistName Name of playlist to store spectral under.
     * \param track Track object.
     * \return
     */
    bool createSpectral(std::string playlistName, PlaylistManager::Track track);

protected:
    /*!
     * \brief paintEvent
     */
    void paintEvent(QPaintEvent *) override{
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor(0,0,0));
        painter.setPen(QPen(Qt::darkRed, 3));

        int centerX = width()/2;
        painter.drawLine(centerX, 0, centerX, height());
    }

    /*!
     * \brief resizeEvent
     * \param event
     */
    void resizeEvent(QResizeEvent* event) override{
        QGraphicsScene *scene = new QGraphicsScene();
        scene -> addItem(getTransformedPixmap(spectralPath));
        view->setScene(scene);
        QWidget::resizeEvent(event);
    }

public slots:

    /*!
     * \brief playbackProgressed
     * \param currentPosition Current position of playing media (ranges 0.0 to 1.0).
     */
    void playbackProgressed(double currentPosition);

};

#endif // SPECTRALVIEW_H
