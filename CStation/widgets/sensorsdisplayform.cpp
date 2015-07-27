#include "sensorsdisplayform.h"
#include "ui_sensorsdisplayform.h"

SensorsDisplayForm::SensorsDisplayForm(Server* c_server, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorsDisplayForm)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    display_block_id = 0;
    server = c_server;
    QObject::connect(server, SIGNAL(blocks_change()), this, SLOT(update_blocks_list()));
    QObject::connect(server, SIGNAL(sensors_change(quint16)), this, SLOT(update_sensors_values(quint16)));
    showNextSensorsPage();
}

SensorsDisplayForm::~SensorsDisplayForm()
{
    delete ui;
}

void SensorsDisplayForm::update_blocks_list()
{
    //
}

void SensorsDisplayForm::update_sensors_values(quint16 block_id)
{
    //
}

void SensorsDisplayForm::showNextSensorsPage()
{
    //
}
