#ifndef CLIENTBLOCKACTIONFORM_H
#define CLIENTBLOCKACTIONFORM_H

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include "../clientaction.h"

namespace Ui {
class ClientBlockActionForm;
}

class ClientBlockActionForm : public QWidget
{
    Q_OBJECT

public:
    explicit ClientBlockActionForm(QWidget *parent, ClientAction *action);
    ~ClientBlockActionForm();

    ClientAction* getAction() const;
    bool isJustButton() const;

private slots:
    void on_pushButton_send_clicked();

    void pushButton_custom_clicked();

private:
    Ui::ClientBlockActionForm *ui;
    ClientAction *client_action;
    QVector<QWidget*> *params_controls;
    QVector<QPushButton*> *button_controls;
    bool is_just_button;

    void createControls();
    void sendParams(ClientParamButton* cbtn);
};

#endif // CLIENTBLOCKACTIONFORM_H
