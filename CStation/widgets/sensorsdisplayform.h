#ifndef SENSORSDISPLAYFORM_H
#define SENSORSDISPLAYFORM_H

#include <QDialog>
#include "./sensorblock.h"
#include "../server.h"

namespace Ui {
class SensorsDisplayForm;
}

class SensorsDisplayForm : public QDialog
{
    Q_OBJECT

public:
    explicit SensorsDisplayForm(Server* c_server, QWidget *parent = 0);
    ~SensorsDisplayForm();

    quint16 getNextPageTimeout() const;
    void setNextPageTimeout(const quint16 &value);
    QString getSensorCodes() const;
    void setSensorCodes(const QString &value);

private slots:
    void update_blocks_list();
    void new_sensor(Sensor* new_sensor);
    void showNextSensorsPage();
    void sensorBlockClicked();

private:
    Ui::SensorsDisplayForm *ui;
    Server* server;
    QString sensor_codes;

    bool fullscreen_block;

    quint16 display_block_id;
    quint16 next_page_timeout;
    QMap<unsigned, unsigned> sensor_counters;
};

#endif // SENSORSDISPLAYFORM_H
