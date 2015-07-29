#include "sensorsdisplayform.h"
#include "ui_sensorsdisplayform.h"

SensorsDisplayForm::SensorsDisplayForm(Server* c_server, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorsDisplayForm)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    display_block_id = 0;
    next_page_timeout = 5000;
    fullscreen_block = false;
    server = c_server;

    quint16 first_display_block_id = display_block_id = server->getNextBlockID(display_block_id);

    do {
        qDebug() << display_block_id;
        if (ClientBlock* nb = server->getClientBlockByID(display_block_id)) {
            QMap<char, Sensor *>::const_iterator i = nb->getSensors()->constBegin();
            while (i != nb->getSensors()->constEnd()) {
                qDebug() << i.key();
                new_sensor(i.value());
                ++i;
            }
        }
        display_block_id = server->getNextBlockID(display_block_id);
    } while (display_block_id && display_block_id != first_display_block_id);

    QObject::connect(server, SIGNAL(blocks_change()), this, SLOT(update_blocks_list()));
    QObject::connect(server, SIGNAL(new_sensor(Sensor*)), this, SLOT(new_sensor(Sensor*)));

    showNextSensorsPage();
}

SensorsDisplayForm::~SensorsDisplayForm()
{
    delete ui;
}

void SensorsDisplayForm::update_blocks_list()
{
    showNextSensorsPage();
}

void SensorsDisplayForm::new_sensor(Sensor *new_sensor)
{
    if (new_sensor && sensor_codes.contains(QChar(new_sensor->getSensorLetter()), Qt::CaseInsensitive)) {
        if (!sensor_counters.contains(new_sensor->getBlockID())) {
            sensor_counters.insert(new_sensor->getBlockID(), 0);
        }

        quint16 sc = sensor_counters.value(new_sensor->getBlockID());
        SensorBlock* nblock = new SensorBlock(new_sensor, ui->label_waiting->palette(), ui->label_waiting->palette(), this);

        connect(nblock, SIGNAL(sensor_click()), this, SLOT(sensorBlockClicked()));

        nblock->setVisible(false);
        ui->layout_blocks->addWidget(nblock, sc/2, sc % 2, 1, 1);

        sensor_counters[new_sensor->getBlockID()]++;
    }
}

void SensorsDisplayForm::showNextSensorsPage()
{
    if (!fullscreen_block) {
        display_block_id = server->getNextBlockID(display_block_id);
        int vis_count = 0;
        for (int i = 0; i < ui->layout_blocks->count(); ++i) {
            SensorBlock *sblock = dynamic_cast<SensorBlock*>(ui->layout_blocks->itemAt(i)->widget());
            if (sblock) {
                sblock->setVisibility(display_block_id);
            }
            if (ui->layout_blocks->itemAt(i)->widget()->isVisible()) {
                vis_count++;
            }
        }
        ui->label_waiting->setVisible(vis_count==0);
    }

    QTimer::singleShot(next_page_timeout, this, SLOT(showNextSensorsPage()));
}

void SensorsDisplayForm::sensorBlockClicked()
{
    fullscreen_block = !fullscreen_block;
    if (fullscreen_block) {
        SensorBlock *sensor_block = dynamic_cast<SensorBlock*>(this->sender());
        if (sensor_block) {
            for (int i = 0; i < ui->layout_blocks->count(); ++i) {
                ui->layout_blocks->itemAt(i)->widget()->setVisible(ui->layout_blocks->itemAt(i)->widget() == sensor_block);
            }
        }
    } else {
        showNextSensorsPage();
    }
}

QString SensorsDisplayForm::getSensorCodes() const
{
    return sensor_codes;
}

void SensorsDisplayForm::setSensorCodes(const QString &value)
{
    sensor_codes = value;
}


quint16 SensorsDisplayForm::getNextPageTimeout() const
{
    return next_page_timeout;
}

void SensorsDisplayForm::setNextPageTimeout(const quint16 &value)
{
    next_page_timeout = value;
}

