#ifndef SENSORBLOCK_H
#define SENSORBLOCK_H

#include <QWidget>
#include <QPalette>
#include "sensor.h"

namespace Ui {
class SensorBlock;
}

class SensorBlock : public QWidget
{
    Q_OBJECT

public:
    explicit SensorBlock(Sensor *d_sensor, QPalette base_pallete, QPalette labels_pallete, QWidget *parent = 0);
    ~SensorBlock();
    void setValueColor(QColor color);
    void setLabelColors(QColor color);
    void setBgColor(QColor color);
    void setGraphicsColor(QColor color);
    void setVisibility(quint16 block_id);

    Sensor *getSensor() const;

private slots:
    void sensor_destroyed(QObject* obj);
    void sensor_update();

signals:
    void sensor_click();

private:
    Ui::SensorBlock *ui;
    QPalette basePallete, labelsPallete;
    Sensor *sensor;
    bool value_is_pressed;

    virtual void resizeEvent(QResizeEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
};

#endif // SENSORBLOCK_H
