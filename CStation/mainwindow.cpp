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
        QStringList devices = server->getIPsList();
        ui->comboBox_ip->clear();
        ui->comboBox_ip->addItems(devices);

        int sel_index = ui->listWidget_devices->currentItem() ? ui->listWidget_devices->currentRow() : 0;
        ui->listWidget_devices->clear();
        ui->listWidget_devices->addItems(devices);
        if (ui->listWidget_devices->count()>sel_index) {
            ui->listWidget_devices->setCurrentRow(sel_index);
        }
    }
}

void MainWindow::update_sensors_values(quint16 block_id)
{
    if (server && block_id && ui->listWidget_devices->currentItem() && ui->listWidget_devices->currentItem()->text().contains("DS"+QString::number(block_id))) {
        ClientBlock *active_bl = server->getClientBlockByID(block_id);
        int rows_cnt = ui->tableWidget_sensors->rowCount();
        if (active_bl) {
            int i = 0;
            if (rows_cnt > active_bl->getSensors()->size()) {
                for(int i=rows_cnt - active_bl->getSensors()->size(); i>0; i--) {
                    ui->tableWidget_sensors->removeRow(i);
                }
            }
            QMap<char, Sensor *>::const_iterator j = active_bl->getSensors()->constBegin();
            while (j != active_bl->getSensors()->constEnd()) {
                if (rows_cnt <= i) ui->tableWidget_sensors->insertRow(i);

                QTableWidgetItem *ritem;

                ritem = ui->tableWidget_sensors->item(i, 0);
                if (!ritem) {
                    ritem = new QTableWidgetItem("");
                    ui->tableWidget_sensors->setItem(i, 0, ritem);
                }
                ritem->setText(QString::number(block_id));

                ritem = ui->tableWidget_sensors->item(i, 1);
                if (!ritem) {
                    ritem = new QTableWidgetItem("");
                    ui->tableWidget_sensors->setItem(i, 1, ritem);
                }
                ritem->setText(QString(j.key()));

                ritem = ui->tableWidget_sensors->item(i, 2);
                if (!ritem) {
                    ritem = new QTableWidgetItem("");
                    ui->tableWidget_sensors->setItem(i, 2, ritem);
                }
                ritem->setText(j.value()->getUpdTime().toString("dd.MM.yyyy hh:mm:ss"));

                ritem = ui->tableWidget_sensors->item(i, 3);
                if (!ritem) {
                    ritem = new QTableWidgetItem("");
                    ui->tableWidget_sensors->setItem(i, 3, ritem);
                }
                ritem->setText(j.value()->getSensorName());

                ritem = ui->tableWidget_sensors->item(i, 4);
                if (!ritem) {
                    ritem = new QTableWidgetItem("");
                    ui->tableWidget_sensors->setItem(i, 4, ritem);
                }
                ritem->setText(j.value()->getValue());

                ritem = ui->tableWidget_sensors->item(i, 5);
                if (!ritem) {
                    ritem = new QTableWidgetItem("");
                    ui->tableWidget_sensors->setItem(i, 5, ritem);
                }
                ritem->setText(j.value()->getSensorEM());

                ++j;
                i++;
            }
        } else {
            for(int i=rows_cnt-1; i>0; i--) {
                ui->tableWidget_sensors->removeRow(i);
            }
        }
    } else {
        int rows_cnt = ui->tableWidget_sensors->rowCount();
        for(int i=rows_cnt-1; i>0; i--) {
            ui->tableWidget_sensors->removeRow(i);
        }
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
        QObject::connect(server, SIGNAL(sensors_change(quint16)), this, SLOT(update_sensors_values(quint16)));
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
    if (!server) ui->pushButton_listen->click();
    server->SendSetConfigsAndReset(ui->comboBox_ip->currentText(), ui->lineEdit_ssid->text(), ui->lineEdit_passw->text(), ui->lineEdit_serv->text(), ui->spinBox_cid->value());
}

void MainWindow::on_pushButton_reboot_clicked()
{
    if (!server) ui->pushButton_listen->click();
    server->SendReboot(ui->comboBox_ip->currentText());
}

void MainWindow::on_pushButton_setup_clicked()
{
    if (!server) ui->pushButton_listen->click();
    server->SendRunSetup(ui->comboBox_ip->currentText());
}

void MainWindow::on_pushButton_start_tone_clicked()
{
    if (!server) ui->pushButton_listen->click();
    server->SendTone(ui->comboBox_ip->currentText(), ui->spinBox_frequency->value());
}

void MainWindow::on_pushButton_stop_tone_clicked()
{
    if (!server) ui->pushButton_listen->click();
    server->SendTone(ui->comboBox_ip->currentText(), 0);
}

void MainWindow::on_listWidget_devices_currentTextChanged(const QString &currentText)
{
    QString sitem = currentText;
    sitem.replace(QRegExp("^([^\\(\\)]*)\\(DS([0-9]*)\\).*$"), "\\2");
    int block_code = sitem.toInt();
    update_sensors_values(block_code);
}
