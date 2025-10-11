#ifndef NEWTRACKSELECTOR_H
#define NEWTRACKSELECTOR_H

#include <QDialog>
#include <QFileInfo>

#include "playlistmanager.h"

namespace Ui {
class NewTrackSelector;
}

class NewTrackSelector : public QDialog
{
    Q_OBJECT

public:
    explicit NewTrackSelector(QWidget *parent = nullptr);
    ~NewTrackSelector();

    std::string getTitle();
    std::string getArtist();
    std::string getFilePath();

    void enableDelete();

    /*!
     * \brief setInitial
     * \param title Title of track/playlist.
     * \param artist Artist of track.
     * \param fileLocation File location of track.
     */
    void setInitial(const char* title, const char* artist, const char* fileLocation);

signals:
    /*!
     * \brief deleteThisTrack
     * \param track Track to delete.
     */
    void deleteThisTrack(PlaylistManager::Track track);

private slots:
    void on_fileSelectionButton_clicked();

    void on_deleteButton_clicked();

private:
    Ui::NewTrackSelector *ui;
};

#endif // NEWTRACKSELECTOR_H
