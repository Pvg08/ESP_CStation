#ifndef IPCAMVIEWER_H
#define IPCAMVIEWER_H

#include <QWidget>

namespace Ui {
class IPCamViewer;
}

class IPCamViewer : public QWidget
{
    Q_OBJECT

public:
    explicit IPCamViewer(QWidget *parent = 0);
    ~IPCamViewer();

private:
    Ui::IPCamViewer *ui;
};

#endif // IPCAMVIEWER_H
