#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColorDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QComboBox>
#include <QtSerialPort/QSerialPortInfo>

#include "./mainpccontroller.h"
#include "./widgets/sensorsdisplayform.h"
#include "./widgets/clientblockslistactionsform.h"
#include "./widgets/ipcamviewer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void get_message(QString message);
    void get_error(QString message);
    void update_blocks_list(quint16 new_block_id);
    void update_sensors_values(quint16 block_id);
    void getReadyToClose();

    void sensors_form_destroyed();

    void on_pushButton_send_clicked();
    void on_pushButton_listen_clicked();
    void on_pushButton_clearlog_clicked();
    void on_pushButton_write_clicked();
    void on_listWidget_devices_currentTextChanged(const QString &currentText);
    void on_pushButton_sensors_display_show_clicked();
    void on_spinBox_nextpage_delay_valueChanged(int arg1);
    void on_toolButton_color_label_clicked();
    void on_toolButton_color_value_clicked();
    void on_toolButton_color_bg_clicked();
    void on_toolButton_color_graphics_clicked();
    void on_spinBox_graphics_timeinterval_valueChanged(int arg1);
    void on_comboBox_ip_currentTextChanged(const QString &arg1);
    void on_checkBox_log_clicked(bool checked);
    void on_spinBox_evt_from_valueChanged(int arg1);
    void on_spinBox_evt_to_valueChanged(int arg1);
    void on_lineEdit_port_textChanged(const QString &arg1);
    void on_lineEdit_remote_port_textChanged(const QString &arg1);
    void on_toolButton_refreshC_clicked();
    void on_toolButton_refreshLS_clicked();
    void on_toolButton_refreshSRV_clicked();
    void on_toolButton_refreshLR_clicked();
    void on_comboBox_portC_activated(int index);
    void on_comboBox_portLS_activated(int index);
    void on_comboBox_portSRV_activated(int index);
    void on_comboBox_portLR_activated(int index);

    void on_pushButton_shutdown_clicked();

private:
    Ui::MainWindow *ui;
    MainPCController* controller;

    SensorsDisplayForm* sensors_form;
    ClientBlocksListActionsForm* actions_form;

    void save_settings(QString filename);
    void load_settings(QString filename);
    void closeEvent(QCloseEvent *event);

    void setBtnColor(QToolButton* button, QColor color);
    void colorPick(QToolButton* button);
    void setCBUSBPorts(QComboBox* cb, int unitcode, QString curr_usb = "");
    void pickUSBPort(QComboBox* cb, int unitcode);

    QString convertComName(QString comname);
};

#endif // MAINWINDOW_H
