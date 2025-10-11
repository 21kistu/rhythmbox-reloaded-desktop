#ifndef NEWTRACK_H
#define NEWTRACK_H

#include <QWidget>

namespace Ui {
class NewTrack;
}

class NewTrack : public QWidget
{
    Q_OBJECT

public:
    explicit NewTrack(QWidget *parent = nullptr);
    ~NewTrack();

private slots:
    void on_fileSelectionButton_clicked();

private:
    Ui::NewTrack *ui;
};

#endif // NEWTRACK_H
