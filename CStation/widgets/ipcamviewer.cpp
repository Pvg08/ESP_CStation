#include "ipcamviewer.h"
#include "ui_ipcamviewer.h"

IPCamViewer::IPCamViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IPCamViewer)
{
    ui->setupUi(this);
}

IPCamViewer::~IPCamViewer()
{
    delete ui;
}
