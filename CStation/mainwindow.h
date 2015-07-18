#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "server.h"

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

    void on_pushButton_send_clicked();
    void on_pushButton_listen_clicked();

    void on_pushButton_clearlog_clicked();

    void on_pushButton_write_clicked();

    void on_pushButton_reboot_clicked();

    void on_pushButton_setup_clicked();

    void on_pushButton_start_tone_clicked();

    void on_pushButton_stop_tone_clicked();

private:
    Ui::MainWindow *ui;
    Server* server;
};

#endif // MAINWINDOW_H
