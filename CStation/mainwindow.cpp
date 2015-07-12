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

void MainWindow::get_message(QString message)
{
    ui->statusBar->showMessage(message, 10000);
    ui->textEdit_log->append(message);
}

void MainWindow::get_error(QString message)
{
    QMessageBox::information(this, tr("CStation"), message);
    ui->textEdit_log->append(message);
}

void MainWindow::on_pushButton_send_clicked()
{
    if (!server) {
        get_error(tr("Server is not running"));
        return;
    }
    server->SendData(ui->lineEdit_ip->text(), ui->lineEdit_message->text());
}

void MainWindow::on_pushButton_listen_clicked()
{
    if (!server) {
        server = new Server();
        QObject::connect(server, SIGNAL(error(QString)), this, SLOT(get_error(QString)));
        QObject::connect(server, SIGNAL(write_message(QString)), this, SLOT(get_message(QString)));
        server->StartServer(ui->lineEdit_port->text().toInt());
    } else {
        server->Reset(ui->lineEdit_port->text().toInt());
    }
}
