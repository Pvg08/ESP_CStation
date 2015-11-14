#include "clientblockactionform.h"
#include "ui_clientblockactionform.h"

ClientBlockActionForm::ClientBlockActionForm(QWidget *parent, ClientAction *action) :
    QWidget(parent),
    ui(new Ui::ClientBlockActionForm)
{
    ui->setupUi(this);
    client_action = action;
    is_just_button = false;
    params_controls = new QVector<QWidget*>();
    button_controls = new QVector<QPushButton*>();
    createControls();
}

ClientBlockActionForm::~ClientBlockActionForm()
{
    delete ui;
    delete params_controls;
    delete button_controls;
}

ClientAction* ClientBlockActionForm::getAction() const
{
    return client_action;
}

void ClientBlockActionForm::createControls()
{
    if (!client_action) return;

    int controls_count = 0;
    int column = 0;
    int sep_column;
    QLabel * new_lt = NULL;

    ClientActionParamsList *params = client_action->getParameters();
    if (params) {

        ui->gridLayout_controls->setColumnMinimumWidth(0, 20);
        ui->gridLayout_controls->setColumnMinimumWidth(1, 75);

        new_lt = new QLabel(tr(client_action->getActionName().toLocal8Bit().data())+":", this);
        new_lt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        ui->gridLayout_controls->addWidget(new_lt, 0, 1, 2, 1, Qt::AlignLeft | Qt::AlignVCenter);
        column+=2;

        for(int i=0; i<params->size(); i++) {
            ClientActionParam param = params->at(i);
            QWidget *new_widget = NULL;
            if (param.type!="CONST") {
                if (param.type == "BOOL") {
                    ui->gridLayout_controls->setColumnStretch(column, 1);

                    QCheckBox *new_cb = new QCheckBox(tr(param.name.toLocal8Bit().data()), this);
                    new_cb->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
                    new_cb->setMinimumHeight(ui->pushButton_send->minimumHeight()-5);
                    ui->gridLayout_controls->addWidget(new_cb, 0, column++, 2, 1, Qt::AlignRight | Qt::AlignVCenter);
                    if (!param.default_value.isEmpty() && param.default_value!="0") {
                        new_cb->setChecked(true);
                    }
                    new_widget = new_cb;
                } else if (param.type == "INT" || param.type == "UINT") {
                    ui->gridLayout_controls->setColumnStretch(column, 2);

                    QLabel * new_lb = new QLabel(tr(param.name.toLocal8Bit().data()), this);
                    new_lb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
                    ui->gridLayout_controls->addWidget(new_lb, 0, column);

                    QSpinBox *new_sb = new QSpinBox(this);
                    new_sb->setMinimumHeight(ui->pushButton_send->minimumHeight()-5);
                    new_sb->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
                    if (param.type == "UINT") {
                        new_sb->setMinimum(0);
                    }
                    new_sb->setMaximum(1000000000);
                    if (!param.default_value.isEmpty()) {
                        new_sb->setValue(param.default_value.toInt());
                    }
                    ui->gridLayout_controls->addWidget(new_sb, 1, column++);
                    new_widget = new_sb;
                } else if (param.type == "DOUBLE" || param.type == "TIMESTAMP") {
                    ui->gridLayout_controls->setColumnStretch(column, 2);

                    QLabel * new_lb = new QLabel(tr(param.name.toLocal8Bit().data()), this);
                    new_lb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
                    ui->gridLayout_controls->addWidget(new_lb, 0, column);

                    QDoubleSpinBox *new_sb = new QDoubleSpinBox(this);
                    new_sb->setMinimumHeight(ui->pushButton_send->minimumHeight()-5);
                    new_sb->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
                    if (param.type == "TIMESTAMP") {
                        new_sb->setMinimum(0);
                        new_sb->setDecimals(0);
                    } else {
                        new_sb->setMinimum(-100000000000000);
                        new_sb->setDecimals(4);
                    }
                    new_sb->setMaximum(100000000000000);
                    if (!param.default_value.isEmpty()) {
                        new_sb->setValue(param.default_value.toInt());
                    }
                    ui->gridLayout_controls->addWidget(new_sb, 1, column++);
                    new_widget = new_sb;
                } else {
                    ui->gridLayout_controls->setColumnStretch(column, 3);

                    QLabel * new_lb = new QLabel(tr(param.name.toLocal8Bit().data()), this);
                    new_lb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
                    ui->gridLayout_controls->addWidget(new_lb, 0, column);

                    QLineEdit *new_tx = new QLineEdit(param.default_value, this);
                    new_tx->setMinimumHeight(ui->pushButton_send->minimumHeight()-5);
                    new_tx->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
                    ui->gridLayout_controls->addWidget(new_tx, 1, column++);
                    new_widget = new_tx;
                }
                controls_count++;
            }
            params_controls->append(new_widget);
        }
    }

    sep_column = column++;

    ClientActionButtonsList *buttons = client_action->getButtons();
    if (buttons) {
        ui->pushButton_send->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        for(int i=0; i<buttons->size(); i++) {
            ClientParamButton btn = buttons->at(i);
            QPushButton *new_btn = new QPushButton(tr(btn.name.toLocal8Bit().data()), this);
            new_btn->setGeometry(ui->pushButton_send->geometry());
            new_btn->setMinimumSize(ui->pushButton_send->minimumSize());
            new_btn->setMaximumSize(ui->pushButton_send->maximumSize());
            new_btn->setSizePolicy(ui->pushButton_send->sizePolicy());
            ui->gridLayout_controls->addWidget(new_btn, 0, column++, 2, 1, Qt::AlignVCenter | Qt::AlignRight);
            button_controls->append(new_btn);
            connect(new_btn, SIGNAL(released()), this, SLOT(pushButton_custom_clicked()));
            controls_count++;
        }
    }

    if (controls_count==0) {
        if (new_lt) new_lt->deleteLater();
        ui->pushButton_send->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        ui->gridLayout_controls->addWidget(ui->pushButton_send, 1, 0, 2, 1, Qt::AlignVCenter | Qt::AlignCenter);
        ui->pushButton_send->setText(tr(client_action->getActionName().toLocal8Bit().data()));
        is_just_button = true;
    } else {
        ui->gridLayout_controls->setColumnMinimumWidth(sep_column, 10);
        ui->gridLayout_controls->addWidget(ui->pushButton_send, 0, column, 2, 1, Qt::AlignVCenter | Qt::AlignRight);
    }
}

void ClientBlockActionForm::sendParams(ClientParamButton* cbtn)
{
    QVector<QString>* param_values = new QVector<QString>();
    for(int i=0; i<params_controls->size(); i++) {
        if (QCheckBox *cb = dynamic_cast<QCheckBox*>(params_controls->at(i))) {
            param_values->append(cb->isChecked() ? "1" : "0");
        } else if (QDoubleSpinBox *sb = dynamic_cast<QDoubleSpinBox*>(params_controls->at(i))) {
            param_values->append(QString::number(sb->value(), 'f', sb->decimals()));
        } else if (QSpinBox *sb = dynamic_cast<QSpinBox*>(params_controls->at(i))) {
            param_values->append(QString::number(sb->value()));
        } else if (QLineEdit *tb = dynamic_cast<QLineEdit*>(params_controls->at(i))) {
            param_values->append(tb->text());
        }
    }
    client_action->sendCommandButton(param_values, cbtn);
    delete param_values;
}

void ClientBlockActionForm::on_pushButton_send_clicked()
{
    sendParams(NULL);
}

void ClientBlockActionForm::pushButton_custom_clicked()
{
    QPushButton *btn = dynamic_cast<QPushButton*>(this->sender());
    if (btn) {
        int btn_index = button_controls->indexOf(btn);
        if (btn_index>=0) {
            ClientParamButton cbtn = client_action->getButtons()->at(btn_index);
            sendParams(&cbtn);
        }
    }
}

bool ClientBlockActionForm::isJustButton() const
{
    return is_just_button;
}
