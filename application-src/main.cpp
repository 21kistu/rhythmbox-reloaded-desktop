#include "mainwindow.h"

#include <QApplication>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("RhythmboxReloaded");

    // Create application directory
    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString().c_str()).exists()){
        QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString().c_str());
        QTextStream qStdOut(stdout);
        qStdOut << "Application data directory does not exist. Creating folder.";
    }

    // Apply styling
    QFile file(":/resources/gruvbox-stylesheet.css");
    if(file.open(QFile::ReadOnly | QFile::Text)){
        QTextStream ts(&file);
        a.setStyleSheet(ts.readAll());
    }
    MainWindow w;
    w.show();
    return a.exec();
}
