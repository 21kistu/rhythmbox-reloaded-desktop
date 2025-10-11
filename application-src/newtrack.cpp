#include "newtrack.h"
#include "ui_newtrack.h"

#include <QFileDialog>
#include <QTextStream>

NewTrack::NewTrack(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NewTrack)
{
    ui->setupUi(this);
}

NewTrack::~NewTrack()
{
    delete ui;
}

void NewTrack::on_fileSelectionButton_clicked() {
    QString fileSelection = QFileDialog::getOpenFileName(this, "Select new track", "");
    QTextStream qStdOut(stdout);
    qStdOut << fileSelection;
}

