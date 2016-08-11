#include "ipcamviewer.h"
#include "ui_ipcamviewer.h"

IPCamViewer::IPCamViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IPCamViewer)
{
    ui->setupUi(this);
    show_next_img = false;
    connect(IPCamThread::Instance(), SIGNAL(new_frame_ready()), this, SLOT(new_frame_ready()));
}

IPCamViewer::~IPCamViewer()
{
    delete ui;
}

void IPCamViewer::new_frame_ready()
{
    show_next_img = true;
    repaint();
}

void IPCamViewer::paintEvent(QPaintEvent *)
{
    if (!show_next_img) return;
    show_next_img = false;
    QPainter p;
    p.begin(this);
    p.drawImage(QPointF(0, 0), *(IPCamThread::Instance()->getCurrentFrame()));
    p.end();
}
