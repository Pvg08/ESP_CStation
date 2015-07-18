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

void MainWindow::update_blocks_list()
{
    if (server) {
        ui->comboBox_ip->clear();
        ui->comboBox_ip->insertItems(0, server->getIPsList());
    }
}

void MainWindow::on_pushButton_send_clicked()
{
    if (!server) {
        get_error(tr("Server is not running"));
        return;
    }
    server->SendData(ui->comboBox_ip->currentText(), ui->lineEdit_message->text());
}

void MainWindow::on_pushButton_listen_clicked()
{
    if (!server) {
        server = new Server();
        QObject::connect(server, SIGNAL(error(QString)), this, SLOT(get_error(QString)));
        QObject::connect(server, SIGNAL(write_message(QString)), this, SLOT(get_message(QString)));
        QObject::connect(server, SIGNAL(blocks_change()), this, SLOT(update_blocks_list()));
        server->StartServer(ui->lineEdit_port->text().toInt());
    } else {
        server->Reset(ui->lineEdit_port->text().toInt());
    }
}

void MainWindow::on_pushButton_clearlog_clicked()
{
    ui->textEdit_log->clear();
    ui->statusBar->clearMessage();
}

void MainWindow::on_pushButton_write_clicked()
{
    if (!server) {
        get_error(tr("Server is not running"));
        return;
    }

    server->SendData(ui->comboBox_ip->currentText(), "SET_SSID="+ui->lineEdit_ssid->text()+"\r\n") &&
    server->SendData(ui->comboBox_ip->currentText(), "SET_PSWD="+ui->lineEdit_passw->text()+"\r\n") &&
    server->SendData(ui->comboBox_ip->currentText(), "SET_SERV="+ui->lineEdit_serv->text()+"\r\n") &&
    server->SendData(ui->comboBox_ip->currentText(), "SET_STID="+ui->lineEdit_cid->text()+"\r\n") &&
    server->SendData(ui->comboBox_ip->currentText(), "SERV_RST=1\r\n");
}

void MainWindow::on_pushButton_reboot_clicked()
{
    if (!server) {
        get_error(tr("Server is not running"));
        return;
    }
    server->SendData(ui->comboBox_ip->currentText(), "SERV_RST=1\r\n");
}

void MainWindow::on_pushButton_setup_clicked()
{
    if (!server) {
        get_error(tr("Server is not running"));
        return;
    }
    server->SendData(ui->comboBox_ip->currentText(), "SERV_CONF=1\r\n");
}
