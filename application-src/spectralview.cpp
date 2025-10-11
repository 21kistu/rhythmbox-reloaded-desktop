#include "spectralview.h"

bool initialized = false;

std::string spectralDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + QString(QDir::separator()).toStdString() + "RhythmboxReloaded" + QString(QDir::separator()).toStdString() + "spectrals";

SpectralView::SpectralView() {}

SpectralView::SpectralView(std::string playlistName, PlaylistManager::Track track){

    setWindowFlags(Qt::Window);
    setWindowTitle("Spectral View");

    bool spectralFine = createSpectral(playlistName, track);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    view = new QGraphicsView();
    QGraphicsScene *scene = new QGraphicsScene();
    if (spectralFine){
        scene -> addItem(getTransformedPixmap(spectralPath));
    }
    view->setScene(scene);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainLayout->addWidget(view);

    view->horizontalScrollBar()->setValue(0);

    initialized = true;

}

QGraphicsPixmapItem* SpectralView::getTransformedPixmap(std::string filePath){
    if (QFile::exists(filePath.c_str())){

        double widthOffset = 0.99;
        double heightOffset = 0.98;

        QRect size = QGuiApplication::primaryScreen()->availableGeometry();
        int maxWidth = size.width()*widthOffset;
        int maxHeight = size.height()*heightOffset;
        int maxSize = maxHeight;

        if (initialized){
            maxWidth = int(double(this->width())*widthOffset);
            maxHeight = int(double(this->height())*heightOffset);
            maxSize = maxHeight;
        }

        if(maxWidth < maxHeight){
            maxSize = maxHeight;
        }

        QPixmap original(filePath.c_str());
        QPixmap scaled = original.scaled(maxSize, maxSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap padded(scaled.width() + maxWidth, scaled.height());

        padded.fill(Qt::black);
        QPainter painter(&padded);
        painter.drawPixmap(((int)maxWidth/2), 0, scaled);
        painter.end();

        return new QGraphicsPixmapItem(padded);

    } else {
        QTextStream qStdOut(stdout);
        qStdOut << "Error! Spectral file not found.\n";
        return new QGraphicsPixmapItem();
    }
}

void SpectralView::playbackProgressed(double currentPosition){
    QScrollBar* hScroll = view->horizontalScrollBar();
    QPropertyAnimation *animation = new QPropertyAnimation(hScroll, "value");
    animation->setDuration(1000);
    animation->setStartValue(hScroll->value());
    animation ->setEndValue(hScroll->maximum()*currentPosition);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

bool SpectralView::createSpectral(std::string playlistName, PlaylistManager::Track track){
    QTextStream qStdOut(stdout);

    // Create master spectrals folder
    if(!QDir(spectralDir.c_str()).exists()){
        QDir().mkdir(spectralDir.c_str());
    }

    // Create spectrals folder for playlist
    if(!QDir((spectralDir + QString(QDir::separator()).toStdString() + playlistName).c_str()).exists()){
        QDir().mkdir((spectralDir + QString(QDir::separator()).toStdString() + playlistName).c_str());
    }

    spectralPath = (spectralDir + QString(QDir::separator()).toStdString() + playlistName + QString(QDir::separator()).toStdString() + track.title  + " - " + track.artist + ".png").c_str();

    if(!QFile::exists(spectralPath.c_str())){
        // Create spectral with ffmpeg
        QProcess ffmpegProcess;
        QStringList args;
        if (QFile::exists(track.fileLocation.c_str())){
            args << "-i" << track.fileLocation.c_str() << "-lavfi" << "showspectrumpic=s=3840x2160:legend=0" << spectralPath.c_str();
            ffmpegProcess.start("ffmpeg", args);
            if(!ffmpegProcess.waitForFinished(-1)){
                qStdOut << "ffmpeg process failed to finish in infinite wait.\n";
                return false;
            }
            qStdOut << "ffmpeg std::out : " << ffmpegProcess.readAllStandardOutput();
            qStdOut << "ffmpeg std::err : " << ffmpegProcess.readAllStandardError();
            return true;
        } else {
            qStdOut << "Cannot create spectral. Media file not found.\n";
            throw std::runtime_error("Invalid state. Tried to create spectral for file that doesn't exist.");
            return false;
        }
    } else {
        qStdOut << "Spectral already exists.";
        return true;
    }
}

void SpectralView::updateSpectral(std::string playlistName, PlaylistManager::Track track){
    createSpectral(playlistName, track);
    QGraphicsScene *scene = new QGraphicsScene();
    scene -> addItem(getTransformedPixmap(spectralPath));
    view->setScene(scene);
}
