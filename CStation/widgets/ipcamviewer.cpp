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
    if (!isVisible() || !isEnabled()) return;
    if (!show_next_img) return;
    show_next_img = false;
    QPainter p;
    QImage *img = IPCamThread::Instance()->getCurrentFrame();
    if (img && img->width()>1 && img->height()>1) {
        if (ui->label_status->isVisible()) ui->label_status->setVisible(false);
        p.begin(this);
        p.drawImage(QPointF(width()/2 - img->width()/2, height()/2 - img->height()/2), *img);
        p.end();
    } else {
        ui->label_status->setText(IPCamThread::Instance()->getStatusText());
        ui->label_status->setVisible(true);
    }
}
