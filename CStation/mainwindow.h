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
    void on_pushButton_listen_clicked(bool checked);
    void get_message(QString message);
    void get_error(QString message);
private:
    Ui::MainWindow *ui;
    Server* server;
};

#endif // MAINWINDOW_H
