#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColorDialog>
#include <QMessageBox>
#include <QToolButton>
#include "server.h"
#include "./widgets/sensorsdisplayform.h"

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
    void update_blocks_list();
    void update_sensors_values(quint16 block_id);

    void sensors_form_destroyed();

    void on_pushButton_send_clicked();
    void on_pushButton_listen_clicked();
    void on_pushButton_clearlog_clicked();
    void on_pushButton_write_clicked();
    void on_pushButton_reboot_clicked();
    void on_pushButton_setup_clicked();
    void on_pushButton_start_tone_clicked();
    void on_pushButton_stop_tone_clicked();
    void on_listWidget_devices_currentTextChanged(const QString &currentText);
    void on_pushButton_set_lcd_text_clicked();
    void on_pushButton_reset_lcd_text_clicked();
    void on_pushButton_sensors_display_show_clicked();

    void on_spinBox_nextpage_delay_valueChanged(int arg1);

    void on_toolButton_color_label_clicked();

    void on_toolButton_color_value_clicked();

    void on_toolButton_color_bg_clicked();

    void on_toolButton_color_graphics_clicked();

private:
    Ui::MainWindow *ui;
    Server* server;
    SensorsDisplayForm* sensors_form;

    Server *getServer();
    void save_settings(QString filename);
    void load_settings(QString filename);
    void closeEvent(QCloseEvent *event);

    void setBtnColor(QToolButton* button, QColor color);
    void colorPick(QToolButton* button);
};

#endif // MAINWINDOW_H
