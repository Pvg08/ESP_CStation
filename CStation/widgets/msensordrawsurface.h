#ifndef MSENSORDRAWSURFACE_H
#define MSENSORDRAWSURFACE_H

#include <QWidget>
#include <QPainter>
#include <math.h>
#include "../sensor.h"

class MSensorDrawSurface : public QWidget
{
    Q_OBJECT
public:
    explicit MSensorDrawSurface(Sensor* d_sensor, QWidget *parent);

protected:
    Sensor* sensor;
    QList<SensorLogItem>* draw_data;

    virtual void paintEvent(QPaintEvent* e);
    float x_to_screen(quint64 x);
    float y_to_screen(float y);

private:
    float y_min, y_max;
    quint64 t_min, t_max, dt_grid;
    float sy_top, sy_bottom, sx_left, sx_right;
    bool show_dates;

    void updateLogGraphicsParameters();
};

#endif // MSENSORDRAWSURFACE_H
