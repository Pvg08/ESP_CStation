#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    server = NULL;
    sensors_form = NULL;
    actions_form = NULL;

    load_settings(QCoreApplication::instance()->applicationDirPath()+"/config.cfg");
}

MainWindow::~MainWindow()
{
    if (actions_form) delete actions_form;
    delete ui;
    if (server) delete server;
}

Server *MainWindow::getServer()
{
    if (!server) ui->pushButton_listen->click();
    return server;
}

void MainWindow::save_settings(QString filename)
{
    QSettings settings(filename, QSettings::IniFormat);

    settings.setValue("main/keep_log", ui->checkBox_log->isChecked());
    settings.setValue("main/server_port", ui->lineEdit_port->text());
    settings.setValue("main/remote_port", ui->lineEdit_remote_port->text());
    settings.setValue("main/autostart_server", ui->checkBox_autostart->isChecked());
    settings.setValue("main/fullscreen_display", ui->checkBox_fullscreen->isChecked());
    settings.setValue("main/display_opened", sensors_form ? true : false);
    settings.setValue("main/next_page_timeout", ui->spinBox_nextpage_delay->value());
    settings.setValue("main/graphics_timeinterval", ui->spinBox_graphics_timeinterval->value());
    settings.setValue("main/sensor_codes", ui->lineEdit_sensor_codes->text());

    settings.setValue("main/display_color_l", ui->toolButton_color_label->palette().background().color().name());
    settings.setValue("main/display_color_v", ui->toolButton_color_value->palette().background().color().name());
    settings.setValue("main/display_color_bg", ui->toolButton_color_bg->palette().background().color().name());
    settings.setValue("main/display_color_gr", ui->toolButton_color_graphics->palette().background().color().name());

    settings.setValue("ip_camera/url", ui->lineEdit_ipcam_url->text());
    settings.setValue("ip_camera/sensors_display_show", ui->checkBox_ipcam_show->isChecked());
    settings.setValue("ip_camera/fps", ui->spinBox_ipcam_fps->value());

    settings.setValue("window/maximized", isMaximized());
    settings.setValue("window/minimized", isMinimized());
    if (!isMaximized() && !isMinimized()) {
        QRect gg = this->geometry();
        settings.setValue("window/left", gg.left());
        settings.setValue("window/top", gg.top());
        settings.setValue("window/width", gg.width());
        settings.setValue("window/height", gg.height());
    }

    if (sensors_form) {
        settings.setValue("sensors_window/maximized", sensors_form->isMaximized());
        settings.setValue("sensors_window/minimized", sensors_form->isMinimized());
        if (!sensors_form->isMaximized() && !sensors_form->isMinimized()) {
            QRect gg = sensors_form->geometry();
            settings.setValue("sensors_window/left", gg.left());
            settings.setValue("sensors_window/top", gg.top());
            settings.setValue("sensors_window/width", gg.width());
            settings.setValue("sensors_window/height", gg.height());
        }
    }
}

void MainWindow::load_settings(QString filename)
{
    QSettings settings(filename, QSettings::IniFormat);

    ui->checkBox_log->setChecked(settings.value("main/keep_log", true).toBool());
    on_checkBox_log_clicked(ui->checkBox_log->isChecked());

    ui->lineEdit_port->setText(QString::number(settings.value("main/server_port", 51015).toInt()));
    ui->lineEdit_remote_port->setText(QString::number(settings.value("main/remote_port", 51016).toInt()));
    ui->checkBox_autostart->setChecked(settings.value("main/autostart_server", false).toBool());
    ui->checkBox_fullscreen->setChecked(settings.value("main/fullscreen_display", false).toBool());
    ui->spinBox_nextpage_delay->setValue(settings.value("main/next_page_timeout", 5000).toInt());
    ui->spinBox_graphics_timeinterval->setValue(settings.value("main/graphics_timeinterval", 86400).toInt());
    ui->lineEdit_sensor_codes->setText(settings.value("main/sensor_codes", "ATPHLRNOBMxMyMz").toString());

    setBtnColor(ui->toolButton_color_label, settings.value("main/display_color_l", "#000000").toString());
    setBtnColor(ui->toolButton_color_value, settings.value("main/display_color_v", "#000000").toString());
    setBtnColor(ui->toolButton_color_bg, settings.value("main/display_color_bg", "#ffffff").toString());
    setBtnColor(ui->toolButton_color_graphics, settings.value("main/display_color_gr", "#000000").toString());

    ui->lineEdit_ipcam_url->setText(settings.value("ip_camera/url", ui->lineEdit_ipcam_url->text()).toString());
    ui->checkBox_ipcam_show->setChecked(settings.value("ip_camera/sensors_display_show", ui->checkBox_ipcam_show->isChecked()).toBool());
    ui->spinBox_ipcam_fps->setValue(settings.value("ip_camera/fps", ui->spinBox_ipcam_fps->value()).toInt());

    QWidget::move(settings.value("window/left", 300).toInt(), settings.value("window/top", 300).toInt());
    QWidget::resize(settings.value("window/width", 640).toInt(), settings.value("window/height", 480).toInt());

    if (settings.value("window/maximized", false).toBool()) {
        showMaximized();
    }
    if (settings.value("window/minimized", false).toBool()) {
        showMinimized();
    }

    if (ui->checkBox_autostart->isChecked()) {
        ui->pushButton_listen->click();
        if (settings.value("main/display_opened", false).toBool()) {
            ui->pushButton_sensors_display_show->click();
            if (sensors_form) {
                sensors_form->move(settings.value("sensors_window/left", 300).toInt(), settings.value("sensors_window/top", 300).toInt());
                sensors_form->resize(settings.value("sensors_window/width", 640).toInt(), settings.value("sensors_window/height", 480).toInt());

                if (settings.value("sensors_window/maximized", false).toBool()) {
                    sensors_form->showMaximized();
                }
                if (settings.value("sensors_window/minimized", false).toBool()) {
                    sensors_form->showMinimized();
                }
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    save_settings(QCoreApplication::instance()->applicationDirPath()+"/config.cfg");
}

void MainWindow::get_message(QString message)
{
    ui->statusBar->showMessage(message, 10000);
    if (ui->checkBox_log->isChecked()) {
        QString dateTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        ui->textEdit_log->append(dateTimeString + ": " + message);
    }
}

void MainWindow::get_error(QString message)
{
    ui->statusBar->showMessage(message, 30000);
    if (ui->checkBox_log->isChecked()) {
        QString dateTimeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QMessageBox::information(this, tr("CStation"), message);
        ui->textEdit_log->append(dateTimeString + ": " + message);
    }
}

void MainWindow::update_blocks_list(quint16 new_block_id)
{
    if (server) {
        int sel_index;
        sel_index = ui->comboBox_ip->currentIndex();
        QStringList devices = server->getIPsList();
        if (sel_index<0 && devices.count()>0) sel_index = 0;
        ui->comboBox_ip->clear();
        ui->comboBox_ip->addItems(devices);
        ui->comboBox_ip->setCurrentIndex(sel_index);
        on_comboBox_ip_currentTextChanged(ui->comboBox_ip->currentText());

        sel_index = ui->listWidget_devices->currentItem() ? ui->listWidget_devices->currentRow() : 0;
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
            ClientSensors::const_iterator j = active_bl->getSensors()->constBegin();
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

    ui->tableWidget_sensors->resizeColumnsToContents();
}

void MainWindow::on_pushButton_send_clicked()
{
    if (!server) {
        get_error(tr("Server is not running"));
        return;
    }
    getServer()->SendData(ui->comboBox_ip->currentText(), ui->lineEdit_message->text());
}

void MainWindow::on_pushButton_listen_clicked()
{
    if (!server) {
        server = new Server();
        QObject::connect(server, SIGNAL(error(QString)), this, SLOT(get_error(QString)));
        QObject::connect(server, SIGNAL(write_message(QString)), this, SLOT(get_message(QString)));
        QObject::connect(server, SIGNAL(new_block_ready(quint16)), this, SLOT(update_blocks_list(quint16)));
        QObject::connect(server, SIGNAL(sensors_change(quint16)), this, SLOT(update_sensors_values(quint16)));
        server->setPort(ui->lineEdit_port->text().toInt());
        server->setRemotePort(ui->lineEdit_remote_port->text().toInt());
        server->StartServer();
        ui->pushButton_sensors_display_show->setEnabled(true);

        actions_form = new ClientBlocksListActionsForm(ui->widget_actions, server);
        actions_form->setIPString(ui->comboBox_ip->currentText());
        ui->verticalLayout_actions->addWidget(actions_form);
    } else {
        server->setPort(ui->lineEdit_port->text().toInt());
        server->setRemotePort(ui->lineEdit_remote_port->text().toInt());
        server->Reset();
    }
}

void MainWindow::on_pushButton_clearlog_clicked()
{
    ui->textEdit_log->clear();
    ui->statusBar->clearMessage();
}

void MainWindow::on_pushButton_write_clicked()
{
    getServer()->SendSetConfigsAndReset(ui->comboBox_ip->currentText(), ui->lineEdit_ssid->text(), ui->lineEdit_passw->text(), ui->lineEdit_serv->text(), ui->spinBox_cid->value(), ui->spinBox_i2c_addr->value());
}

void MainWindow::on_listWidget_devices_currentTextChanged(const QString &currentText)
{
    QString sitem = currentText;
    sitem.replace(QRegExp("^([^\\(\\)]*)\\(DS([0-9]*)\\).*$"), "\\2");
    int block_code = sitem.toInt();
    update_sensors_values(block_code);
}

void MainWindow::sensors_form_destroyed()
{
    if (IPCamThread::Instance()->isRunning()) {
        IPCamThread::Instance()->do_stop();
    }
    ui->pushButton_sensors_display_show->setEnabled(true);
    ui->pushButton_listen->setEnabled(true);
    sensors_form = NULL;
}

void MainWindow::on_pushButton_sensors_display_show_clicked()
{
    ui->pushButton_sensors_display_show->setEnabled(false);
    ui->pushButton_listen->setEnabled(false);

    sensors_form = new SensorsDisplayForm(server, this);
    sensors_form->setAttribute(Qt::WA_DeleteOnClose);
    sensors_form->setNextPageTimeout(ui->spinBox_nextpage_delay->value());
    sensors_form->setSensorCodes(ui->lineEdit_sensor_codes->text());
    sensors_form->setLabel_color(ui->toolButton_color_label->palette().background().color());
    sensors_form->setValue_color(ui->toolButton_color_value->palette().background().color());
    sensors_form->setBg_color(ui->toolButton_color_bg->palette().background().color());
    sensors_form->setGraphics_color(ui->toolButton_color_graphics->palette().background().color());
    sensors_form->setSensorGraphicsLogInterval((quint64)ui->spinBox_graphics_timeinterval->value() * 1000);
    sensors_form->setIPCamVisibility(ui->checkBox_ipcam_show->isChecked());

    if (ui->checkBox_ipcam_show->isChecked() && !IPCamThread::Instance()->isRunning()) {
        IPCamThread::Instance()->listen(ui->lineEdit_ipcam_url->text(), ui->spinBox_ipcam_fps->value());
    }

    QObject::connect(sensors_form, SIGNAL(destroyed()), this, SLOT(sensors_form_destroyed()));

    if (ui->checkBox_fullscreen->isChecked()) {
        sensors_form->showFullScreen();
    } else {
        sensors_form->show();
    }
}

void MainWindow::on_spinBox_nextpage_delay_valueChanged(int arg1)
{
    if (sensors_form) sensors_form->setNextPageTimeout(ui->spinBox_nextpage_delay->value());
}

void MainWindow::setBtnColor(QToolButton* button, QColor color)
{
    QString qss = QString("background-color: %1").arg(color.name());
    button->setStyleSheet(qss);
    QPalette p = button->palette();
    p.setColor(QPalette::Background, color);
    button->setPalette(p);
}

void MainWindow::colorPick(QToolButton *button)
{
    QColor color = QColorDialog::getColor(button->palette().background().color(), this);
    if(color.isValid()) {
        setBtnColor(button, color);
    }
}

void MainWindow::on_toolButton_color_label_clicked()
{
    colorPick(ui->toolButton_color_label);
    if (sensors_form) sensors_form->setLabel_color(ui->toolButton_color_label->palette().background().color());
}

void MainWindow::on_toolButton_color_value_clicked()
{
    colorPick(ui->toolButton_color_value);
    if (sensors_form) sensors_form->setValue_color(ui->toolButton_color_value->palette().background().color());
}

void MainWindow::on_toolButton_color_bg_clicked()
{
    colorPick(ui->toolButton_color_bg);
    if (sensors_form) sensors_form->setBg_color(ui->toolButton_color_bg->palette().background().color());
}

void MainWindow::on_toolButton_color_graphics_clicked()
{
    colorPick(ui->toolButton_color_graphics);
    if (sensors_form) sensors_form->setGraphics_color(ui->toolButton_color_graphics->palette().background().color());
}

void MainWindow::on_spinBox_graphics_timeinterval_valueChanged(int arg1)
{
    if (sensors_form) sensors_form->setSensorGraphicsLogInterval((quint64)arg1*1000);
}

void MainWindow::on_comboBox_ip_currentTextChanged(const QString &arg1)
{
    if (actions_form) {
        actions_form->setIPString(arg1);
        ClientBlock* cblock = getServer()->getClientBlock(ui->comboBox_ip->currentText());
        if (cblock) {
            ui->lineEdit_ssid->setText(cblock->getWifiSSID());
            ui->lineEdit_passw->setText(cblock->getWifiPassw());
            ui->lineEdit_serv->setText(cblock->getDSServerAddr());
            ui->spinBox_cid->setValue(cblock->getblockId());
        }
    }
}

void MainWindow::on_checkBox_log_clicked(bool checked)
{
    if (!checked) on_pushButton_clearlog_clicked();
    ui->textEdit_log->setEnabled(checked);
    ui->pushButton_clearlog->setEnabled(checked);
}
