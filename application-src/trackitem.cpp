#include "trackitem.h"
#include "ui_trackitem.h"

TrackItem::TrackItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::trackItem)
{
    ui->setupUi(this);
}

TrackItem::~TrackItem()
{
    delete ui;
}

void TrackItem::setAttributes(int index, std::string title, std::string artist, std::string fileLocation){
    ui->trackTitle->setText(QString::fromStdString(title));
    ui->trackArtist->setText(QString::fromStdString(artist));
    track.index = index;
    track.title = title;
    track.artist = artist;
    track.fileLocation = fileLocation;

    QGraphicsScene *scene = new QGraphicsScene();
    scene->addItem(new RoundedPixmap(fileLocation.c_str(), 125, 2, 0.3));
    ui->trackGraphics->setScene(scene);
}

void TrackItem::setAttributes(int index, const char* title){
    // Playlist item; don't show artist field
    ui->trackTitle->setText(title);
    ui->trackArtist->setVisible(false);
    track.index = index;
    track.title = title;
    track.artist = "";
    track.fileLocation = "";

    QGraphicsScene *scene = new QGraphicsScene();
    scene->addItem(new RoundedPixmap("", 125, 2, 0.3));
    ui->trackGraphics->setScene(scene);
}

PlaylistManager::Track TrackItem::getTrack(){
    return this->track;
}
