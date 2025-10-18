#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <taglib/taglib.h>

PlaylistManager pm;

// Current side view tracking
const int PLAYLISTS = 0;
const int TRACKS = 1;
bool minified = false;
bool trackOnly = false;
int currentView = PLAYLISTS;
int previousView = PLAYLISTS;

// Playlist playback mode
const int TILL_END = 0;
const int SINGLE = 1;
const int LOOP = 2;
int playbackMode = TILL_END;

bool isPrivate = false;

bool inEditMode = false;

SpectralView* spectral;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Update button to show playback status
    if (pm.isPlaying()){
        ui->playPauseButton->setText("Pause");
    } else {
        ui->playPauseButton->setText("Play");
    }

    // Determine if minified
    int width = MainWindow::size().width();
    if (width < 750){
        minified = true;
    } else {
        minified = false;
        ui->trackBackButton->setVisible(false);
    }

    // Connect to track completed signal
    connect(&pm, &PlaylistManager::trackCompletedSignal, this, &MainWindow::playbackEndUpdate);

    showAllPlaylists();

    // timer to poll for playback updates
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updatePlaybackProgress);
    timer->start(1000);

    // Hide track view
    ui->currentTrackWidget->setVisible(false);
    // Hide track details in bottom bar
    ui->bottomTrack->setVisible(false);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_previousButton_clicked() {
    setTrack(pm.getCurrentPlaylistName(), pm.getPreviousTrack());
}

void MainWindow::on_nextButton_clicked() {
    setTrack(pm.getCurrentPlaylistName(), pm.getNextTrack());
}

void MainWindow::on_playPauseButton_clicked() {
    pm.playPause();

    // Update button to show playback status
    if (pm.isPlaying()){
        ui->playPauseButton->setText("Pause");
    } else {
        ui->playPauseButton->setText("Play");
    }
}

void MainWindow::on_sideBackButton_clicked(){ navigateBack(); }

void MainWindow::on_trackBackButton_clicked(){ navigateBack(); }

void MainWindow::on_addButton_clicked() {
    // Functionality depends on current view
    if (currentView == PLAYLISTS){
        // Dialog to create new playlist
        QString playlistName = QInputDialog::getText(0, "New Playlist", "Playlist name:", QLineEdit::Normal, "", NULL);
        if (strcmp(playlistName.toStdString().c_str(), "") != 0){
            QTextStream qStdOut(stdout); qStdOut << "Request to create playlist: " << playlistName << "\n";
            pm.createPlaylist(playlistName.toStdString().c_str());
            showAllPlaylists();
        }
    } else if (currentView == TRACKS){
        // Dialog to create new track
        NewTrackSelector trackSelection;
        int result = trackSelection.exec();
        if (result == QDialog::Accepted){
            // add track to playlist
            pm.addTrack(pm.getBrowsingPlaylistName(), trackSelection.getTitle(), trackSelection.getArtist(), trackSelection.getFilePath());
            setPlaylist(pm.getBrowsingPlaylistName());
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *){
    // Hide side view when width is less than 750 pixels
    int width = MainWindow::size().width();
    if (width < 750){
        if (!minified){
            if (pm.isPlaying()){
                ui->playlistWidget->setVisible(false);
                ui->currentTrackWidget->setVisible(true);
                ui->trackBackButton->setVisible(true);
            } else {
                ui->playlistWidget->setVisible(true);
                ui->currentTrackWidget->setVisible(false);
                ui->trackBackButton->setVisible(false);
            }
        }
        minified = true;
        trackOnly = true;
    } else {
        if (minified){
            ui->playlistWidget->setVisible(true);
            ui->currentTrackWidget->setVisible(true);
            ui->trackBackButton->setVisible(false);
            ui->bottomTrack->setVisible(false);
        }
        minified = false;
        trackOnly = false;
    }
}

void MainWindow::onTrackItemClicked(TrackItem* trackItem){
    if(inEditMode){
        NewTrackSelector trackSelection;
        trackSelection.enableDelete();
        PlaylistManager::Track track = trackItem->getTrack();
        trackSelection.setInitial((track.title).c_str(), (track.artist).c_str(), (track.fileLocation).c_str());
        connect(&trackSelection, &NewTrackSelector::deleteThisTrack, this, &MainWindow::deleteTrack);
        int result = trackSelection.exec();
        if (result == QDialog::Accepted){
            // add track to playlist
            PlaylistManager::Track newTrack{0, trackSelection.getTitle(), trackSelection.getArtist(), trackSelection.getFilePath()};
            pm.editTrack(pm.getBrowsingPlaylistName().c_str(), trackItem->getTrack(), newTrack, false);
            setPlaylist(pm.getBrowsingPlaylistName());
        }
    } else {
        bool exitStatus = setTrack(pm.getBrowsingPlaylistName(), trackItem->getTrack());
        if (exitStatus == PlaylistManager::OK){
            if(minified){
                ui->playlistWidget->setVisible(false);
            } else {
                ui->trackBackButton->setVisible(false);
            }
            ui->currentTrackWidget->setVisible(true);
        }
    }
}

void MainWindow::onPlaylistItemClicked(TrackItem* trackItem){
    if(inEditMode){
        std::string oldPlaylistName = trackItem->getTrack().title;

        QInputDialog inputDialog(this);
        inputDialog.setLabelText("Edit Playlist");
        inputDialog.setTextValue(QString::fromStdString(oldPlaylistName));
        inputDialog.show();
        QApplication::processEvents();
        QDialogButtonBox *buttonBox = inputDialog.findChild<QDialogButtonBox*>();
        if (buttonBox){
            QPushButton *deleteButton = new QPushButton("Delete");
            buttonBox->addButton(deleteButton, QDialogButtonBox::ActionRole);
            connect(deleteButton, &QPushButton::clicked, this, [this, &inputDialog]{
                pm.renamePlaylist(inputDialog.textValue().toStdString().c_str(), inputDialog.textValue().toStdString().c_str(), true);
                inputDialog.close();
                showAllPlaylists();
            });
        }
        if (inputDialog.exec() == QDialog::Accepted){
            QString newPlaylistName = inputDialog.textValue();
            if (strcmp(newPlaylistName.toStdString().c_str(), "") != 0){
                QTextStream qStdOut(stdout); qStdOut << "Request to rename playlist from " << oldPlaylistName.c_str() << " to " << newPlaylistName.toStdString().c_str() << "\n";
                pm.renamePlaylist(oldPlaylistName.c_str(), newPlaylistName.toStdString().c_str(), false);
                showAllPlaylists();
            }
        }
    } else {
        PlaylistManager::Track track = trackItem->getTrack();
        std::string trackTitle = track.title.data();
        const char* playlistName = trackTitle.c_str();
        setPlaylist(playlistName);
        pm.setBrowsingPlaylistName(playlistName);
        QTextStream qStdOut(stdout);
        qStdOut << "Navigating into playlist: " << playlistName << "\n";
    }
}

bool MainWindow::setTrack(std::string playlistName, PlaylistManager::Track track){

    // Update playlist manager
    bool exitStatus = pm.setCurrentTrack(track, true, playlistName);

    if (exitStatus == PlaylistManager::OK){
        setArt(track.fileLocation.c_str());

        // Log to console
        QTextStream qStdOut(stdout);
        qStdOut << "New track set: " << (track.index) << "\t" << QString::fromStdString(track.title) << "\t" << QString::fromStdString(track.artist) << "\t" << QString::fromStdString(track.fileLocation)  << "\n";

        // Update track interface
        ui -> currentTitleLabel -> setText(QString::fromStdString(track.title));
        ui -> currentArtistLabel -> setText(QString::fromStdString(track.artist));

        // Update track details in bottom bar
        ui->bottomTrackTitle->setText(QString::fromStdString(track.title));

        if (minified && currentView == TRACKS){
            ui->bottomTrack->setVisible(false);
        }

        // Set total duration
        if(pm.isPlaying()){
            double current = pm.getTrackDuration();
            int minutes = (int)(current/60);
            int seconds = (int)(current)%60;
            char formattedTime[6];
            snprintf(formattedTime, sizeof(formattedTime), "%02d:%02d", minutes, seconds);
            ui->totalTimeLabel->setText(formattedTime);
        }

        if(minified){
            trackOnly = true;
        }

        if (spectral){
            spectral->updateSpectral(pm.getCurrentPlaylistName(), pm.getCurrentTrack());
        }

    }

    return exitStatus;
}

void MainWindow::setPlaylist(std::string title, std::string filter){

    // Delete old area
    QList<QScrollArea*> oldAreas = ui->playlistWidget->findChildren<QScrollArea*>();
    for (QScrollArea* oldArea : oldAreas){
        ui->playlistWidget->layout()->removeWidget(oldArea);
        oldArea->deleteLater();
    }

    QScrollArea *area = new QScrollArea;
    QWidget *container = new QWidget();

    std::vector<PlaylistManager::Track> playlist = pm.getPlaylistTracks(title);

    if (playlist.size() == 0){

        QTextStream qStdOut(stdout);
        qStdOut << "Playlist size is zero (no tracks in playlist).\n";

        // Label to inform user that playlist needs to be created
        QHBoxLayout *layout = new QHBoxLayout(container);
        QLabel *label = new QLabel;
        label->setText("Please add a track to the playlist.");
        layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
        layout->addWidget(label);
        layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
        container->setLayout(layout);
        area->setWidget(container);
        area->setWidgetResizable(true);
        area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    } else {
        QVBoxLayout *layout = new QVBoxLayout(container);

        for (PlaylistManager::Track track : playlist){
            TrackItem *ri = new TrackItem();
            bool addTrack = true;
            if (filter != ""){
                bool titleNotMatch = (track.title).find(filter) == std::string::npos;
                bool artistNotMatch = (track.artist).find(filter) == std::string::npos;
                if (titleNotMatch && artistNotMatch){
                    addTrack = false;
                }
            }
            if (addTrack){
                ri->setAttributes(track.index, track.title, track.artist, track.fileLocation);
                layout->addWidget(ri);
                connect(ri, &TrackItem::clicked, this, &MainWindow::onTrackItemClicked);
            }
        }

        layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
        container->setLayout(layout);
        area->setWidget(container);
        area->setWidgetResizable(true);
        area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    ui->playlistWidget->layout()->addWidget(area);

    updateViewTracker(TRACKS);

}

void MainWindow::showAllPlaylists(std::string filter){

    // Delete old area
    QList<QScrollArea*> oldAreas = ui->playlistWidget->findChildren<QScrollArea*>();
    for (QScrollArea* oldArea : oldAreas){
        ui->playlistWidget->layout()->removeWidget(oldArea);
        oldArea->deleteLater();
    }

    QScrollArea *area = new QScrollArea;
    QWidget *container = new QWidget();

    std::vector<std::string> playlists = pm.getAllPlaylists();
    if (playlists.size() == 0){

        QTextStream qStdOut(stdout);
        qStdOut << "Playlist size is zero (no playlists created).\n";

        // Label to inform user that playlist needs to be created
        QHBoxLayout *layout = new QHBoxLayout(container);
        QLabel *label = new QLabel;
        label->setText("Please create a playlist.");
        layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
        layout->addWidget(label);
        layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
        container->setLayout(layout);
        area->setWidget(container);
        area->setWidgetResizable(true);
        area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    } else {

        QVBoxLayout *layout = new QVBoxLayout(container);

        int index = 0;
        for (std::string playlist : playlists){
            TrackItem *ti = new TrackItem();
            bool addPlaylist = true;
            if (filter != ""){
                bool titleNotMatch = (playlist).find(filter) == std::string::npos;
                if (titleNotMatch){
                    addPlaylist = false;
                }
            }
            if (addPlaylist){
                ti->setAttributes(index, playlist.c_str());
                layout->addWidget(ti);
                connect(ti, &TrackItem::clicked, this, &MainWindow::onPlaylistItemClicked);
            }
            index++;
        }

        layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
        container->setLayout(layout);
        area->setWidget(container);
        area->setWidgetResizable(true);
        area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    ui->playlistWidget->layout()->addWidget(area);

    updateViewTracker(PLAYLISTS);

}

void MainWindow::setArt(const char* mediaFile){
    QGraphicsScene *scene = new QGraphicsScene();
    int width = MainWindow::size().width();
    int height = MainWindow::size().height();
    int maxSize = qMin(width, height);
    scene -> addItem(new RoundedPixmap(mediaFile, maxSize, 10, 0.3));
    ui -> currentGraphics -> setScene(scene);
}

void MainWindow::navigateBack(){
    QTextStream qStdOut(stdout);
    qStdOut << "Current view is " << currentView << "\n" ;
    ui->searchBar->setText("");
    switch(currentView){
        case PLAYLISTS:
            if(trackOnly){
                setPlaylist(pm.getCurrentPlaylistName());
                updateViewTracker(TRACKS); trackOnly = false;
                ui->currentTrackWidget->setVisible(false);
                ui->playlistWidget->setVisible(true);
                ui->bottomTrack->setVisible(true);
            } else {
                showAllPlaylists();
            }
            break;
        case TRACKS:
            if (minified && trackOnly){
                setPlaylist(pm.getCurrentPlaylistName());
                updateViewTracker(TRACKS);
                ui->currentTrackWidget->setVisible(false);
                ui->playlistWidget->setVisible(true);
                ui->bottomTrack->setVisible(true);
                trackOnly = false;
            } else {
                showAllPlaylists();
                updateViewTracker(PLAYLISTS);
            }
            break;
    }
}

void MainWindow::updateViewTracker(int newView){
    if (currentView != newView){
        previousView = currentView;
        currentView = newView;
    }
}

void MainWindow::playbackEndUpdate(){
    switch(playbackMode){
        case TILL_END: if (!pm.atLastTrack()){setTrack(pm.getCurrentPlaylistName(), pm.getNextTrack());}; break;
        case SINGLE: setTrack(pm.getCurrentPlaylistName(), pm.getCurrentTrack());; // reset trackview
        case LOOP: setTrack(pm.getCurrentPlaylistName(), pm.getNextTrack()); break;
    }
}

void MainWindow::on_privacyButton_clicked(){
    if(isPrivate){
        isPrivate = false;
        ui->currentTitleLabel->setGraphicsEffect(0);
        ui->currentArtistLabel->setGraphicsEffect(0);
        ui->currentGraphics->setGraphicsEffect(0);
    } else {
        isPrivate = true;
        QGraphicsBlurEffect* titleBlur = new QGraphicsBlurEffect();
        titleBlur->setBlurRadius(7);
        QGraphicsBlurEffect* artistBlur = new QGraphicsBlurEffect();
        artistBlur->setBlurRadius(7);
        QGraphicsBlurEffect* graphicsBlur = new QGraphicsBlurEffect();
        graphicsBlur->setBlurRadius(50);
        ui->currentTitleLabel->setGraphicsEffect(titleBlur);
        ui->currentArtistLabel->setGraphicsEffect(artistBlur);
        ui->currentGraphics->setGraphicsEffect(graphicsBlur);
    }
}

void MainWindow::on_bottomOne_clicked(){
    switch(playbackMode){
        case TILL_END: playbackMode = SINGLE; break;
        case SINGLE: playbackMode = LOOP; break;
        case LOOP: playbackMode = TILL_END; break;
    }
    QTextStream qStdOut(stdout);
    qStdOut << "Playback mode is now " << playbackMode << "\n";
}

void MainWindow::on_editButton_clicked(){
    if (inEditMode){
        ui->editButton->setText("Edit");
        inEditMode = false;
    } else {
        ui->editButton->setText("Save");
        inEditMode = true;
    }
}

void MainWindow::deleteTrack(PlaylistManager::Track track){
    QTextStream qStdOut(stdout);
    qStdOut << "Delete track triggered for " << track.title.c_str() << " by " << track.artist.c_str() << " at " << track.fileLocation.c_str();
    pm.editTrack(pm.getBrowsingPlaylistName().c_str(), track, track, true);
    setPlaylist(pm.getBrowsingPlaylistName());
}


void MainWindow::updatePlaybackProgress(){
    if(pm.isPlaying()){
        double current = pm.getCurrentPosition();
        int minutes = (int)(current/60);
        int seconds = (int)(current)%60;
        char formattedTime[6];
        snprintf(formattedTime, sizeof(formattedTime), "%02d:%02d", minutes, seconds);
        ui->currentTimeLabel->setText(formattedTime);

        ui->seekBar->setValue((int)(current/pm.getPrecalculatedDuration()*100));

        emit playbackProgressed(current/pm.getPrecalculatedDuration());

    }
}


void MainWindow::on_seekBar_sliderMoved(int position) {
    pm.setTrackPosition(position);
}

void MainWindow::on_spectrogramButton_clicked(){
    spectral = new SpectralView(pm.getCurrentPlaylistName(), pm.getCurrentTrack());
    spectral->showMaximized();
    connect(this, &MainWindow::playbackProgressed, spectral, &SpectralView::playbackProgressed);
}

void MainWindow::on_searchBar_returnPressed() {
    if (currentView == PLAYLISTS){
        showAllPlaylists(ui->searchBar->text().toStdString());
    } else if (currentView == TRACKS){
        setPlaylist(pm.getBrowsingPlaylistName(), ui->searchBar->text().toStdString());
    }
}

void MainWindow::on_bottomTrackTitle_clicked() {
    ui->currentTrackWidget->setVisible(true);
    ui->playlistWidget->setVisible(false);
    ui->trackBackButton->setVisible(true);
    ui->bottomTrack->setVisible(false);
    trackOnly = true; minified = true;
}

