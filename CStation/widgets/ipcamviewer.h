#ifndef IPCAMVIEWER_H
#define IPCAMVIEWER_H

#include <QWidget>
#include <QPainter>
#include "../ipcamthread.h"

namespace Ui {
class IPCamViewer;
}

class IPCamViewer : public QWidget
{
    Q_OBJECT

public:
    explicit IPCamViewer(QWidget *parent = 0);
    ~IPCamViewer();

private slots:
    void new_frame_ready();

private:
    Ui::IPCamViewer *ui;
    QImage tmpimage;
    bool show_next_img;

    virtual void paintEvent(QPaintEvent *);
};

#endif // IPCAMVIEWER_H
