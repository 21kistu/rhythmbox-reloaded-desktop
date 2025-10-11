#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QInputDialog>
#include <QObject>
#include <QGraphicsBlurEffect>
#include <QGuiApplication>
#include <QTimer>
#include <QScrollBar>

#include "trackitem.h"
#include "playlistmanager.h"
#include "newtrackselector.h"
#include "roundedpixmap.h"
#include "spectralview.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /*!
     * \brief resizeEvent
     * \param e
     */
    void resizeEvent(QResizeEvent *e);

    /*!
     * \brief onTrackItemClicked
     * \param trackItem TrackItem (with track attributes) that was clicked.
     */
    void onTrackItemClicked(TrackItem* trackItem);

    /*!
     * \brief onPlaylistItemClicked
     * \param trackItem TrackItem (with playlist attributes) that was clicked.
     */
    void onPlaylistItemClicked(TrackItem* trackItem);

    /*!
     * \brief playbackEndUpdate
     */
    void playbackEndUpdate();

    /*!
     * \brief setTrack
     * \param playlistName Name of playlist that track belongs to.
     * \param track Track object as defined by Track struct.
     * \return Succeses status.
     */
    bool setTrack(std::string playlistName, PlaylistManager::Track track);

    /*!
     * \brief setPlaylist
     * \param title Playlist name.
     * \param filter String to filter tracks by partially matching attributes.
     */
    void setPlaylist(std::string title, std::string filter="");

    /*!
     * \brief showAllPlaylists
     * \param filter String to filter playlists by partially matching playlist name.
     */
    void showAllPlaylists(std::string filter="");

    /*!
     * \brief setArt
     * \param graphicsFile File containing image/embedded graphics.
     */
    void setArt(const char* graphicsFile);

    /*!
     * \brief navigateBack
     */
    void navigateBack();

    /*!
     * \brief updateViewTracker
     * \param newView View number as defined by allocated constants.
     */
    void updateViewTracker(int newView);

    /*!
     * \brief deletePlaylist
     * \param playlistName Name of playlist to delete.
     */
    void deletePlaylist(const char* playlistName);

    /*!
     * \brief deleteTrack
     * \param track Track object to delete.
     */
    void deleteTrack(PlaylistManager::Track track);

    /*!
     * \brief updatePlaybackProgress
     */
    void updatePlaybackProgress();

signals:

    /*!
     * \brief playbackProgressed
     */
    void playbackProgressed(double);

private slots:

    void on_previousButton_clicked();

    void on_playPauseButton_clicked();

    void on_nextButton_clicked();

    void on_addButton_clicked();

    void on_sideBackButton_clicked();

    void on_trackBackButton_clicked();
    void on_privacyButton_clicked();

    void on_bottomOne_clicked();

    void on_editButton_clicked();

    /*!
     * \brief on_seekBar_sliderMoved
     * \param position New position of slider.
     */
    void on_seekBar_sliderMoved(int position);

    void on_spectrogramButton_clicked();

    void on_searchBar_returnPressed();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
