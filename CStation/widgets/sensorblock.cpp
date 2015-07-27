#include "sensorblock.h"
#include "ui_sensorblock.h"

SensorBlock::SensorBlock(Sensor *d_sensor, QPalette base_pallete, QPalette labels_pallete, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SensorBlock)
{
    ui->setupUi(this);
    basePallete = base_pallete;
    labelsPallete = labels_pallete;
    sensor = d_sensor;
    connect(sensor, SIGNAL(destroyed(QObject*)), this, SLOT(sensor_destroyed(QObject*)));
    connect(sensor, SIGNAL(value_change()), this, SLOT(sensor_update()));
    this->setVisible(false);
}

SensorBlock::~SensorBlock()
{
    delete ui;
}

Sensor *SensorBlock::getSensor() const
{
    return sensor;
}

void SensorBlock::setValueColor(QColor color)
{
    basePallete.setColor(QPalette::Foreground, color);
    ui->lcdNumber_value->setPalette(basePallete);
    ui->label_value->setPalette(basePallete);
}

void SensorBlock::setLabelColors(QColor color)
{
    labelsPallete.setColor(QPalette::Foreground, color);
    ui->label_em->setPalette(labelsPallete);
    ui->label_sensor_name->setPalette(labelsPallete);
}

void SensorBlock::setBgColor(QColor color)
{
    basePallete.setColor(QPalette::Background, color);
    labelsPallete.setColor(QPalette::Background, color);
    ui->lcdNumber_value->setPalette(basePallete);
    ui->label_value->setPalette(basePallete);
    ui->label_em->setPalette(labelsPallete);
    ui->label_sensor_name->setPalette(labelsPallete);
}

void SensorBlock::sensor_destroyed(QObject *obj)
{
    sensor = NULL;
    this->deleteLater();
}

void SensorBlock::sensor_update()
{
    if (sensor) {
        if (sensor->getSensorDataType()==Sensor::SDT_ENUM) {
            ui->label_value->setText(sensor->getValue());
        } else {
            ui->lcdNumber_value->display(sensor->getFloatValue());
        }

        if (!isVisible()) {
            ui->label_sensor_name->setText(sensor->getSensorName() + " (DS"+QString::number(sensor->getBlockID())+")");
            ui->label_em->setText(sensor->getSensorEM());
            if (ui->label_em->text().isEmpty()) ui->label_em->setVisible(false);
            if (sensor->getSensorDataType()==Sensor::SDT_ENUM) {
                ui->lcdNumber_value->setVisible(false);
            } else {
                ui->label_value->setVisible(false);
            }
        }

    }
}

void SensorBlock::resizeEvent(QResizeEvent *event)
{
    if (ui->label_value->isVisible()) {

        QFont fnt = ui->label_value->font();
        fnt.setPixelSize((ui->label_value->width()<ui->label_value->height()?ui->label_value->width():ui->label_value->height()) / 3);
        ui->label_value->setFont(fnt);
    }
}
