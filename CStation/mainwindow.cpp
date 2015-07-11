#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    server = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
    if (server) delete server;
}

void MainWindow::on_pushButton_listen_clicked(bool checked)
{
    if (server) {
        delete server;
    }
    ui->statusBar->showMessage(tr("Starting server..."), 10000);
    server = new Server();
    QObject::connect(server, SIGNAL(error(QString)), this, SLOT(get_error(QString)));
    QObject::connect(server, SIGNAL(write_message(QString)), this, SLOT(get_message(QString)));
    server->StartServer(ui->lineEdit_port->text().toInt());
}

void MainWindow::get_message(QString message)
{
    ui->statusBar->showMessage(message, 10000);
}

void MainWindow::get_error(QString message)
{
    QMessageBox::information(this, tr("CStation"), message);
}
