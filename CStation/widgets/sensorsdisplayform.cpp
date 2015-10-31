#include "sensorsdisplayform.h"
#include "ui_sensorsdisplayform.h"

SensorsDisplayForm::SensorsDisplayForm(Server* c_server, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorsDisplayForm)
{
    ui->setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowMinMaxButtonsHint);

    display_block_id = 0;
    next_page_timeout = 5000;
    sensor_graphics_log_interval = 24*60*60*1000;
    fullscreen_block = false;
    server = c_server;
}

void SensorsDisplayForm::showEvent(QShowEvent *event)
{
    quint16 first_display_block_id = display_block_id = server->getNextBlockID(display_block_id);

    do {
        if (ClientBlock* nb = server->getClientBlockByID(display_block_id)) {
            ClientSensors::const_iterator i = nb->getSensors()->constBegin();
            while (i != nb->getSensors()->constEnd()) {
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
    if (new_sensor && sensor_codes.contains(new_sensor->getSensorCode(), Qt::CaseInsensitive)) {
        if (!sensor_counters.contains(new_sensor->getBlockID())) {
            sensor_counters.insert(new_sensor->getBlockID(), 0);
        }

        quint16 sc = sensor_counters.value(new_sensor->getBlockID());

        QPalette p_b = ui->label_waiting->palette();
        p_b.setColor(QPalette::Background, bg_color);
        p_b.setColor(QPalette::Foreground, value_color);
        QPalette p_l = QPalette(p_b);
        p_l.setColor(QPalette::Foreground, label_color);

        new_sensor->startLogDataTracking(sensor_graphics_log_interval);

        SensorBlock* nblock = new SensorBlock(new_sensor, p_b, p_l, this);
        nblock->setGraphicsColor(graphics_color);
        nblock->setLabelColors(label_color);

        connect(nblock, SIGNAL(sensor_click()), this, SLOT(sensorBlockClicked()));

        nblock->setVisible(false);
        ui->layout_blocks->addWidget(nblock, sc/2, sc % 2, 1, 1);

        sensor_counters[new_sensor->getBlockID()]++;
        nblock->repaint();
    }
}

void SensorsDisplayForm::showNextSensorsPage()
{
    if (!fullscreen_block) {
        display_block_id = server->getNextBlockID(display_block_id);
        int vis_count = 0;
        for (int i = 0; i < ui->layout_blocks->count(); ++i) {
            SensorBlock *sblock = dynamic_cast<SensorBlock*>(ui->layout_blocks->itemAt(i)->widget());
            if (sblock && sblock->getSensor() && !sblock->getSensor()->getValue().isEmpty()) {
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

quint64 SensorsDisplayForm::getSensorGraphicsLogInterval() const
{
    return sensor_graphics_log_interval;
}

void SensorsDisplayForm::setSensorGraphicsLogInterval(const quint64 &value)
{
    sensor_graphics_log_interval = value;
    for (int i = 0; i < ui->layout_blocks->count(); ++i) {
        if (SensorBlock *sblock = dynamic_cast<SensorBlock*>(ui->layout_blocks->itemAt(i)->widget())) {
            sblock->getSensor()->startLogDataTracking(sensor_graphics_log_interval);
            sblock->repaint();
        }
    }
}

QColor SensorsDisplayForm::getGraphics_color() const
{
    return graphics_color;
}

void SensorsDisplayForm::setGraphics_color(const QColor &value)
{
    graphics_color = value;
    for (int i = 0; i < ui->layout_blocks->count(); ++i) {
        if (SensorBlock *sblock = dynamic_cast<SensorBlock*>(ui->layout_blocks->itemAt(i)->widget())) {
            sblock->setGraphicsColor(value);
        }
    }
}

QColor SensorsDisplayForm::getValue_color() const
{
    return value_color;
}

void SensorsDisplayForm::setValue_color(const QColor &value)
{
    value_color = value;
    for (int i = 0; i < ui->layout_blocks->count(); ++i) {
        if (SensorBlock *sblock = dynamic_cast<SensorBlock*>(ui->layout_blocks->itemAt(i)->widget())) {
            sblock->setValueColor(value);
        }
    }
}

QColor SensorsDisplayForm::getLabel_color() const
{
    return label_color;
}

void SensorsDisplayForm::setLabel_color(const QColor &value)
{
    label_color = value;
    for (int i = 0; i < ui->layout_blocks->count(); ++i) {
        if (SensorBlock *sblock = dynamic_cast<SensorBlock*>(ui->layout_blocks->itemAt(i)->widget())) {
            sblock->setLabelColors(value);
        }
    }
    QPalette p_l = ui->label_waiting->palette();
    p_l.setColor(QPalette::Foreground, value);
    ui->label_waiting->setPalette(p_l);
}

QColor SensorsDisplayForm::getBg_color() const
{
    return bg_color;
}

void SensorsDisplayForm::setBg_color(const QColor &value)
{
    bg_color = value;
    for (int i = 0; i < ui->layout_blocks->count(); ++i) {
        if (SensorBlock *sblock = dynamic_cast<SensorBlock*>(ui->layout_blocks->itemAt(i)->widget())) {
            sblock->setBgColor(value);
        }
    }

    QPalette p = this->palette();
    p.setColor(QPalette::Background, bg_color);
    this->setPalette(p);
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

