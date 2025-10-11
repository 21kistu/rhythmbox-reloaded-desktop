#include "playlistmanager.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

std::string applicationDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + QString(QDir::separator()).toStdString() + "RhythmboxReloaded";
std::string PLAYLISTSFILE = applicationDir + QString(QDir::separator()).toStdString() + "playlists.xml";
std::string playlistsDir = applicationDir + QString(QDir::separator()).toStdString() + "playlists";
std::string recordsDir = applicationDir + QString(QDir::separator()).toStdString() + "recorder";

ma_engine engine;
ma_result result;
ma_sound sound;

std::vector<PlaylistManager::Track> currentPlaylist;
int currentTrack;

std::string currentPlaylistName;
std::string browsingPlaylistName;

bool calculatedSinceLoaded = false;
int trackDuration;

PlaylistManager::PlaylistManager() {
    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) { printf("Failed to init engine."); }
}

bool PlaylistManager::loadFile(std::string fileName){
    calculatedSinceLoaded = false;
    ma_sound_uninit(&sound);
    result = ma_sound_init_from_file(&engine, fileName.c_str(), 0, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        QTextStream qStdOut(stdout);
        qStdOut << "Failed to load file"  <<  "\n";
        QMessageBox box;
        box.critical(0, "Error", "Failed to load file.");
        return FAILED;
    } else {
        // Register callback to record when track ends
        ma_sound_set_end_callback(&sound, trackCompleted, this);
        return OK;
    }
}

void PlaylistManager::onSoundEnd(ma_sound*, void*){
    int timestamp = QDateTime::currentSecsSinceEpoch();
    recordTrackCompleted(currentPlaylistName, currentPlaylist.at(currentTrack), timestamp);
    emit trackCompletedSignal();
    QTextStream qStdOut(stdout);
    qStdOut << "Completed playback at " << timestamp << "\n";
}

void PlaylistManager::playPause(){
    if(ma_sound_is_playing(&sound)){
        ma_sound_stop(&sound);
    } else {
        ma_sound_start(&sound);
    }
}

bool PlaylistManager::isPlaying(){
    return ma_sound_is_playing(&sound);
}

bool PlaylistManager::setCurrentTrack(PlaylistManager::Track track, bool startPlaying, std::string playlistName){
    currentTrack = track.index;
    setCurrentPlaylistName(playlistName);
    bool exitStatus = loadFile(track.fileLocation);
    if (startPlaying && !isPlaying()){
        ma_sound_start(&sound);
    }
    return exitStatus;
}

PlaylistManager::Track PlaylistManager::getNextTrack(){
    try {
        return currentPlaylist.at(currentTrack + 1);
    } catch (std::out_of_range &ex){
        // Loop around to start of playlist
        return currentPlaylist.at(0);
    }
}

PlaylistManager::Track PlaylistManager::getPreviousTrack(){
    try {
        return currentPlaylist.at(currentTrack - 1);
    } catch (std::out_of_range &ex){
        // Loop around to end of playlist
        return currentPlaylist.at(currentPlaylist.size() - 1);
    }
}

PlaylistManager::Track PlaylistManager::getCurrentTrack(){
    return currentPlaylist.at(currentTrack);
}

std::vector<std::string> PlaylistManager::getAllPlaylists(){

    std::vector<std::string> playlists;

    if(QFile::exists(PLAYLISTSFILE.c_str())){
        using namespace tinyxml2;

        XMLDocument doc;
        XMLError result = doc.LoadFile(PLAYLISTSFILE.c_str());

        if (result == XML_SUCCESS){
            XMLElement* root = doc.RootElement();
            for (XMLElement* element = root->FirstChildElement("playlist"); element != nullptr; element = element->NextSiblingElement("playlist") ){
                playlists.push_back(element->Attribute("name"));
            }
        } else {
            QTextStream qStdOut(stdout);
            qStdOut << "Failed to load playlists file: " << PLAYLISTSFILE.c_str() << ("\n");
        }
        return playlists;
    } else {
        QTextStream qStdOut(stdout);
        qStdOut << "Playlists file (" << PLAYLISTSFILE.c_str() << ") does not exist.\n";
        return playlists;
    }
}

std::vector<PlaylistManager::Track> PlaylistManager::getPlaylistTracks(std::string playlistName){
    std::vector<Track> playlist;
    if(QFile::exists((playlistsDir + QString(QDir::separator()).toStdString() + playlistName + ".xml").c_str())){
        using namespace tinyxml2;

        std::string fileName = playlistsDir + QString(QDir::separator()).toStdString() + playlistName + ".xml";
        XMLDocument doc;
        XMLError result = doc.LoadFile(fileName.c_str());

        int index = 0;
        if (result == XML_SUCCESS){
            XMLElement* root = doc.RootElement();
            for (XMLElement* element = root->FirstChildElement("track"); element != nullptr; element = element->NextSiblingElement("track") ){
                struct PlaylistManager::Track t;
                t.index = index;
                t.title = element->Attribute("title");
                t.artist = element->Attribute("artist");
                t.fileLocation = element->Attribute("location");
                playlist.push_back(t);
                index++;
            }
        } else {
            QTextStream qStdOut(stdout);
            qStdOut << "Failed to load playlist (file not found): " << QString::fromStdString(fileName) << ("\n");
            return playlist;
        }

        for (PlaylistManager::Track track : playlist){
            QTextStream qStdOut(stdout);
            qStdOut << QString::fromStdString(track.title) << "\t" << QString::fromStdString(track.artist) << "\t" << QString::fromStdString(track.fileLocation) << ("\n");
        }
        return playlist;
    } else {
        QTextStream qStdOut(stdout);
        qStdOut << "Playlist (" << playlistName.c_str() << ") does not exist.\n";
        return playlist;
    }
}

void PlaylistManager::createPlaylist(const char* playlistName){
    using namespace tinyxml2;
    // check if display file exists
    if(QFile::exists(PLAYLISTSFILE.c_str())){
        XMLDocument doc;
        XMLError result = doc.LoadFile(PLAYLISTSFILE.c_str());
        bool playlistExists = false;
        if (result == XML_SUCCESS){
            XMLElement* root = doc.RootElement();
            for (XMLElement* element = root->FirstChildElement("playlist"); element != nullptr; element = element->NextSiblingElement("playlist") ){
                if (strcmp(playlistName, element->Attribute("name")) == 0){
                    playlistExists = true;
                    break;
                }
            }
            if (playlistExists){
                QTextStream qStdOut(stdout);
                qStdOut << "Playlist with the same name (" << playlistName << ") already exists.\n";
            } else {
                XMLElement* child = doc.NewElement("playlist");
                child->SetAttribute("name", playlistName);
                root->InsertEndChild(child);

                doc.SaveFile(PLAYLISTSFILE.c_str());
            }
        } else {
            QTextStream qStdOut(stdout);
            qStdOut << ("Failed to load playlists file: ") << PLAYLISTSFILE.c_str() << ("\n");
        }
    } else {
        QTextStream qStdOut(stdout);
        qStdOut << ("Playlist file does not exist.\n");
        // create file

        XMLDocument playlistFile;
        XMLDeclaration* declaration = playlistFile.NewDeclaration(NULL);
        playlistFile.InsertEndChild(declaration);
        XMLElement* root = playlistFile.NewElement("playlists");
        playlistFile.InsertEndChild(root);

        XMLElement* child = playlistFile.NewElement("playlist");
        child->SetAttribute("name", playlistName);
        root->InsertEndChild(child);

        playlistFile.SaveFile(PLAYLISTSFILE.c_str());
    }
}

void PlaylistManager::deletePlaylist(char* playlistName){
    // remove playlist and save displayfile
    // prompt to delete playlist file
    using namespace tinyxml2;
    // check if display file exists
    if(QFile::exists(PLAYLISTSFILE.c_str())){
        XMLDocument doc;
        XMLError result = doc.LoadFile(PLAYLISTSFILE.c_str());
        if (result == XML_SUCCESS){
            XMLElement* root = doc.RootElement();
            for (XMLElement* element = root->FirstChildElement("playlist"); element != nullptr; element = element->NextSiblingElement("playlist") ){
                if (strcmp(playlistName, element->Attribute("name")) == 0){
                    root->DeleteChild(element);
                }
            }
            doc.SaveFile(PLAYLISTSFILE.c_str());
        } else {
            QTextStream qStdOut(stdout);
            qStdOut <<  "Failed to load playlists file: " << PLAYLISTSFILE.c_str() << ("\n");
        }
    } else {
        QTextStream qStdOut(stdout);
        qStdOut << ("Playlist file does not exist. Nothing to do.\n");
    }
}

void PlaylistManager::addTrack(std::string playlistName, std::string title, std::string artist, std::string filePath){
    // Create playlist folder if it doesn't exist
    if(!QDir(playlistsDir.c_str()).exists()){
        QDir().mkdir(playlistsDir.c_str());
    }

    // add track to playlist file
    using namespace tinyxml2;
    // check if display file exists
    if(QFile::exists((playlistsDir + QString(QDir::separator()).toStdString() + playlistName + ".xml").c_str())){
        XMLDocument tracksFile;
        XMLError result = tracksFile.LoadFile((playlistsDir + QString(QDir::separator()).toStdString() + playlistName + ".xml").c_str());
        bool trackExists = false;
        if (result == XML_SUCCESS){

            XMLElement* root = tracksFile.RootElement();
            for (XMLElement* element = root->FirstChildElement("playlist"); element != nullptr; element = element->NextSiblingElement("playlist") ){
                if (strcmp(filePath.c_str(), element->Attribute("location")) == 0){
                    trackExists = true;
                    break;
                }
            }
            if (trackExists){
                QTextStream qStdOut(stdout);
                qStdOut << "Track with the same location (" << playlistName.c_str() << ") already exists in this playlist.\n";
            } else {
                XMLElement* child = tracksFile.NewElement("track");
                child->SetAttribute("title", title.c_str());
                child->SetAttribute("artist", artist.c_str());
                child->SetAttribute("location", filePath.c_str());
                root->InsertEndChild(child);

                tracksFile.SaveFile((playlistsDir + QString(QDir::separator()).toStdString() + playlistName + ".xml").c_str());
            }
        } else {
            QTextStream qStdOut(stdout);
            qStdOut << "Failed to load playlists file: " << (playlistsDir + playlistName).c_str() << "\n";
        }
    } else {
        QTextStream qStdOut(stdout);
        qStdOut << "Playlist file for " << playlistName.c_str() << " does not exist.\n";
        // create file

        XMLDocument playlistFile;
        XMLDeclaration* declaration = playlistFile.NewDeclaration(NULL);
        playlistFile.InsertEndChild(declaration);
        XMLElement* root = playlistFile.NewElement("playlist");
        playlistFile.InsertEndChild(root);

        XMLElement* child = playlistFile.NewElement("track");
        child->SetAttribute("title", title.c_str());
        child->SetAttribute("artist", artist.c_str());
        child->SetAttribute("location", filePath.c_str());
        root->InsertEndChild(child);

        playlistFile.SaveFile((playlistsDir + QString(QDir::separator()).toStdString() + playlistName + ".xml").c_str());
    }
}

void PlaylistManager::removeTrack(){
    // remove track from playlist file
}

std::string PlaylistManager::getCurrentPlaylistName(){ return currentPlaylistName; }
void PlaylistManager::setCurrentPlaylistName(std::string playlistName){
    currentPlaylistName = playlistName;
    currentPlaylist = getPlaylistTracks(playlistName);
}
std::string PlaylistManager::getBrowsingPlaylistName(){ return browsingPlaylistName; }
void PlaylistManager::setBrowsingPlaylistName(std::string playlistName){ browsingPlaylistName = playlistName; }

bool PlaylistManager::atLastTrack(){
    QTextStream qStdOut(stdout);
    qStdOut << "Current index is " << currentTrack << " of " << currentPlaylist.size() << "\n" ;
    if ((unsigned)(currentTrack + 1) >= currentPlaylist.size()){
        return true;
    }
    return false;
}

void PlaylistManager::renamePlaylist(const char* oldPlaylistName, const char* newPlaylistName, bool deleteIt){
    using namespace tinyxml2;
    // check if display file exists
    if(QFile::exists(PLAYLISTSFILE.c_str())){
        XMLDocument doc;
        XMLError result = doc.LoadFile(PLAYLISTSFILE.c_str());
        bool playlistNameUpdated = false;
        if (result == XML_SUCCESS){
            XMLElement* root = doc.RootElement();
            for (XMLElement* element = root->FirstChildElement("playlist"); element != nullptr; element = element->NextSiblingElement("playlist") ){
                if (strcmp(oldPlaylistName, element->Attribute("name")) == 0){
                    if(deleteIt){
                        element->Parent()->DeleteChild(element);
                        playlistNameUpdated = true;
                    } else {
                        element->SetAttribute("name", newPlaylistName);
                        playlistNameUpdated = true;
                    }
                    break;
                }
            }
            if (playlistNameUpdated){
                // move playlist file
                if (QFile::exists((playlistsDir + QString(QDir::separator()).toStdString() + oldPlaylistName + ".xml").c_str())){
                    if(deleteIt){
                        QFile::remove((playlistsDir + QString(QDir::separator()).toStdString() + oldPlaylistName + ".xml").c_str());
                    } else {
                        QFile::rename((playlistsDir + QString(QDir::separator()).toStdString() + oldPlaylistName + ".xml").c_str(), (playlistsDir + QString(QDir::separator()).toStdString() + newPlaylistName + ".xml").c_str());
                    }
                } else {
                    QTextStream qStdOut(stdout);
                    qStdOut << "Could not find playlist file for " << oldPlaylistName << "\n";
                    qStdOut << "Possible data loss! Please create a backup of the data directory and investigate.\n";
                }
                doc.SaveFile(PLAYLISTSFILE.c_str());
            } else {
                QTextStream qStdOut(stdout);
                qStdOut << "Could not find existing playlist with name " << oldPlaylistName << " when saving edit.\n";
            }
        } else {
            QTextStream qStdOut(stdout);
            qStdOut << ("Failed to load playlists file: ") << PLAYLISTSFILE.c_str() << (". Unable to save changes.\n");
        }
    } else {
        QTextStream qStdOut(stdout);
        qStdOut << "Playlist file does not exist. Nothing to do.\n";
    }
}

void PlaylistManager::editTrack(const char* playlistName, PlaylistManager::Track oldTrack, PlaylistManager::Track newTrack, bool deleteIt){
    // Create playlist folder if it doesn't exist
    if(!QDir(playlistsDir.c_str()).exists()){
        QDir().mkdir(playlistsDir.c_str());
    }

    // add track to playlist file
    using namespace tinyxml2;
    // check if display file exists
    if(QFile::exists((playlistsDir + QString(QDir::separator()).toStdString() + playlistName + ".xml").c_str())){
        XMLDocument tracksFile;
        XMLError result = tracksFile.LoadFile((playlistsDir + QString(QDir::separator()).toStdString() + playlistName + ".xml").c_str());
        bool trackUpdated = false;
        if (result == XML_SUCCESS){
            XMLElement* root = tracksFile.RootElement();
            for (XMLElement* element = root->FirstChildElement("track"); element != nullptr; element = element->NextSiblingElement("track") ){
                if (strcmp((oldTrack.fileLocation).c_str(), element->Attribute("location")) == 0){
                    if (deleteIt){
                        element->Parent()->DeleteChild(element);
                        trackUpdated = true;
                        tracksFile.SaveFile((playlistsDir + QString(QDir::separator()).toStdString() + playlistName + ".xml").c_str());
                    } else {
                        element->SetAttribute("title", newTrack.title.c_str());
                        element->SetAttribute("artist", newTrack.artist.c_str());
                        element->SetAttribute("location", newTrack.fileLocation.c_str());
                        tracksFile.SaveFile((playlistsDir + QString(QDir::separator()).toStdString() + playlistName + ".xml").c_str());

                        // Update record file
                        if(QFile::exists((recordsDir + QString(QDir::separator()).toStdString() + playlistName + QString(QDir::separator()).toStdString() + oldTrack.title  + " - " + oldTrack.artist + ".xml").c_str())){
                            QFile::rename((recordsDir + QString(QDir::separator()).toStdString() + playlistName + QString(QDir::separator()).toStdString() + oldTrack.title  + " - " + oldTrack.artist + ".xml").c_str(), (recordsDir + QString(QDir::separator()).toStdString() + playlistName + QString(QDir::separator()).toStdString() + newTrack.title  + " - " + newTrack.artist + ".xml").c_str());
                        }
                        QTextStream qStdOut(stdout);
                        qStdOut << "Track found and updated.\n";
                        trackUpdated = true;
                    }
                    break;
                }
            }
            if (!trackUpdated){
                QTextStream qStdOut(stdout);
                qStdOut << "No track with old location was found in playlist " << playlistName << ". Edits not saved.\n";
            }
        } else {
            QTextStream qStdOut(stdout);
            qStdOut << "Failed to load playlists file: " << (playlistsDir + QString(QDir::separator()).toStdString() + playlistName).c_str() << ". Edits not saved.\n";
        }
    } else {
        QTextStream qStdOut(stdout);
        qStdOut << "Playlist file for " << playlistName << " does not exist. Nothing to do.\n";
    }
}

void PlaylistManager::recordTrackCompleted(std::string playlistName, Track track, int timestamp){
    // Create record folder if it doesn't exist
    if(!QDir(recordsDir.c_str()).exists()){
        QDir().mkdir(recordsDir.c_str());
    }

    // Create playlist record folder if it doesn't exist
    if(!QDir((recordsDir + QString(QDir::separator()).toStdString() + playlistName).c_str()).exists()){
        QDir().mkdir((recordsDir + QString(QDir::separator()).toStdString() + playlistName).c_str());
    }

    using namespace tinyxml2;
    // check if record file exists
    if(QFile::exists((recordsDir + QString(QDir::separator()).toStdString() + playlistName + QString(QDir::separator()).toStdString() + track.title  + " - " + track.artist + ".xml").c_str())){
        XMLDocument recordFile;
        XMLError result = recordFile.LoadFile((recordsDir + QString(QDir::separator()).toStdString() + playlistName + QString(QDir::separator()).toStdString() + track.title  + " - " + track.artist + ".xml").c_str());
        if (result == XML_SUCCESS){
            XMLElement* root = recordFile.RootElement();
            XMLElement* child = recordFile.NewElement("record");
            child->SetAttribute("timestamp", timestamp);
            root->InsertEndChild(child);
            recordFile.SaveFile((recordsDir + QString(QDir::separator()).toStdString() + playlistName + QString(QDir::separator()).toStdString() + track.title  + " - " + track.artist + ".xml").c_str());
        } else {
            QTextStream qStdOut(stdout);
            qStdOut << "Failed to load existing record file: " << (recordsDir + QString(QDir::separator()).toStdString() + playlistName  + QString(QDir::separator()).toStdString() + playlistName).c_str() << "\n";
        }
    } else {
        QTextStream qStdOut(stdout);
        qStdOut << "Record file for " << (track.title).c_str() << " - " << (track.artist).c_str() << " does not exist yet.\n";
        // create file

        XMLDocument recordFile;
        XMLDeclaration* declaration = recordFile.NewDeclaration(NULL);
        recordFile.InsertEndChild(declaration);
        XMLElement* root = recordFile.NewElement("records");
        recordFile.InsertEndChild(root);

        XMLElement* child = recordFile.NewElement("record");
        child->SetAttribute("timestamp", timestamp);
        root->InsertEndChild(child);

        recordFile.SaveFile((recordsDir + QString(QDir::separator()).toStdString() + playlistName + QString(QDir::separator()).toStdString() + track.title  + " - " + track.artist + ".xml").c_str());
    }
}

double PlaylistManager::getCurrentPosition(){
    ma_uint64 currentFrame = 0;
    ma_result result = ma_sound_get_cursor_in_pcm_frames(&sound, &currentFrame);
    if (result == MA_SUCCESS){ return (double) currentFrame / engine.sampleRate; }
    return 0;
}

double PlaylistManager::getTrackDuration(){
    ma_uint64 totalFrames = 0;
    ma_result result = ma_sound_get_length_in_pcm_frames(&sound, &totalFrames);
    trackDuration = (double) totalFrames / engine.sampleRate;
    calculatedSinceLoaded = true;
    if (result == MA_SUCCESS){ return trackDuration; }
    return 0;
}

void PlaylistManager::setTrackPosition(int position){
    QTextStream qStdOut(stdout);
    int frame = (int)(((double)position/100) * getTrackDuration() * engine.sampleRate);
    qStdOut << "Requested to move to frame " << QString::number(frame) << " of " << QString::number(getTrackDuration()*engine.sampleRate) << " derived from " << QString::number(position) << "\n";
    ma_sound_seek_to_pcm_frame(&sound, frame);
}

double PlaylistManager::getPrecalculatedDuration(){
    if (!calculatedSinceLoaded){
        return getTrackDuration();
    }
    return trackDuration;
}
