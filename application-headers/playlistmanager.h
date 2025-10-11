#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <stdio.h>
#include <vector>
#include <iostream>
#include <ctime>

#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QObject>
#include <QStandardPaths>

#include "tinyxml2.h"
#include "miniaudio.h"

class PlaylistManager: public QObject
{
    Q_OBJECT

public:
    PlaylistManager();

    static const bool FAILED = false;
    static const bool OK = true;

    struct Track {
        int index;
        std::string title;
        std::string artist;
        std::string fileLocation;
    };

    // Audio player management
    bool loadFile(std::string fileLocation);
    void playPause();
    bool isPlaying();

    bool setCurrentTrack(Track track, bool startPlaying, std::string playlistName);

    void onSoundEnd(ma_sound* pSound, void* pUserData);

    static void trackCompleted(void* pUserData, ma_sound* pSound){
        if(pUserData){
            PlaylistManager* playlistManager = static_cast<PlaylistManager*>(pUserData);
            if (playlistManager){
                playlistManager->onSoundEnd(pSound, pUserData);
            }
        }
    }

    // Track information
    Track getNextTrack();
    Track getPreviousTrack();
    Track getCurrentTrack();

    // Playlist information
    std::string getCurrentPlaylistName();
    void setCurrentPlaylistName(std::string playlistName);
    std::string getBrowsingPlaylistName();
    void setBrowsingPlaylistName(std::string playlistName);

    std::vector<std::string> getAllPlaylists();
    std::vector<Track> getPlaylistTracks(std::string playlistName);

    // Playlist modifications
    void createPlaylist(const char* playlistName);
    void deletePlaylist(char* playlistName);
    void addTrack(std::string playlist, std::string title, std::string artist, std::string filePath);
    void removeTrack();

    void renamePlaylist(const char* oldPlaylistName, const char* newPlaylistName, bool deleteIt);
    void editTrack(const char* playlistName, Track oldTrack, Track newTrack, bool deleteIt);

    bool atLastTrack();

    void recordTrackCompleted(std::string playlistName, Track track, int timestamp);

    double getCurrentPosition();
    double getTrackDuration();
    void setTrackPosition(int position);

    double getPrecalculatedDuration();

signals:
    void trackCompletedSignal();
};

#endif // PLAYLISTMANAGER_H
