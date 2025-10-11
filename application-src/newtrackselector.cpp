#include "newtrackselector.h"
#include "ui_newtrackselector.h"

#include <QString>
#include <QFileDialog>

bool initialSet = false;

NewTrackSelector::NewTrackSelector(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewTrackSelector)
{
    ui->setupUi(this);
    ui->deleteButton->setVisible(false);
}

NewTrackSelector::~NewTrackSelector()
{
    delete ui;
}

void NewTrackSelector::on_fileSelectionButton_clicked()
{

    QString fileSelection;
    if (initialSet){
        fileSelection = QFileDialog::getOpenFileName(this, "New track selection", getFilePath().c_str());
    } else {
        fileSelection = QFileDialog::getOpenFileName(this, "New track selection", "");
    }
    QTextStream qStdOut(stdout);
    ui->fileLocationLabel->setText(fileSelection);

    QFileInfo fileInfo(fileSelection);
    QString baseName = fileInfo.baseName();

    int delimiterIndex = baseName.indexOf(" - ");

    if(delimiterIndex != -1){
        // At least one occurence exists
        ui->titleEdit->setText(baseName.left(delimiterIndex));
        ui->artistEdit->setText(baseName.mid(delimiterIndex + 3));
    } else {
        // Delimiter does not exist in filename
        ui->titleEdit->setText(baseName);
    }
}

std::string NewTrackSelector::getTitle(){
    return ui->titleEdit->text().toStdString();
}

std::string NewTrackSelector::getArtist(){
    return ui->artistEdit->text().toStdString();
}

std::string NewTrackSelector::getFilePath(){
    return ui->fileLocationLabel->text().toStdString();
}

void NewTrackSelector::enableDelete(){
    ui->deleteButton->setVisible(true);
}

void NewTrackSelector::on_deleteButton_clicked(){
    PlaylistManager::Track track{0, getTitle(), getArtist(), getFilePath()};
    this->close();
    emit deleteThisTrack(track);
}

void NewTrackSelector::setInitial(const char* title, const char* artist, const char* fileLocation){
    ui->titleEdit->setText(title);
    ui->artistEdit->setText(artist);
    ui->fileLocationLabel->setText(fileLocation);
    initialSet = true;
}

