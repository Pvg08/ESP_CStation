#include "msensordrawsurface.h"

MSensorDrawSurface::MSensorDrawSurface(Sensor *d_sensor, QWidget *parent) :
    QWidget(parent)
{
    sensor = d_sensor;
    draw_data = sensor->startLogDataTracking(24*60*60*1000);
}

void MSensorDrawSurface::setMinMax()
{
    y_min = draw_data->at(0).log_value;
    y_max = y_min;
    bool min_set = false;
    bool max_set = false;
    float tmp;
    for(int i=0; i<draw_data->size(); ++i) {
        tmp = draw_data->at(i).log_value;
        if (tmp>=sensor->getFromValue() && tmp<=sensor->getToValue()) {
            if (!min_set || tmp < y_min) {
                y_min = tmp;
                min_set = true;
            }
            if (!max_set || tmp > y_max) {
                y_max = tmp;
                max_set = true;
            }
        }
    }
    t_min = draw_data->first().log_time;
    t_max = draw_data->last().log_time;
    dt_grid = (t_max-t_min) / 4;
}

void MSensorDrawSurface::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    if (this->isHidden() || !this->isVisible()) return;

    QPainter painter(this);
    painter.setBackground(palette().background().color());
    painter.setPen(palette().background().color());
    painter.drawRect(rect().left(),rect().top(),rect().right()-1,rect().bottom()-1);

    if (!sensor || sensor->getValue().isEmpty()) return;
    if (!draw_data || draw_data->size()<2) return;

    setMinMax();

    float x0, y0, x1, y1;
    quint64 t = t_min;
    QString t_text;
    QFontMetrics fm = painter.fontMetrics();

    painter.setPen(palette().text().color());

    y0 = y_to_screen(y_min);
    painter.drawLine(0, y0, width(), y0);
    t_text = QString::number(y_min, 'g', 3);
    painter.drawText(0, y0+1, t_text);

    if (y_max > y_min) {
        y0 = y_to_screen(y_max);
        painter.drawLine(0, y0, width(), y0);
        painter.drawText(5, y0-fm.height()-1, QString::number(y_max, 'g', 3));
    }

    do {
        x0 = x_to_screen(t);
        painter.drawLine(x0,0,x0,height());
        t_text = QDateTime::fromMSecsSinceEpoch(t).toString("dd.MM.yyyy");
        painter.drawText(x0+5, fm.height()+1, t_text);
        t_text = QDateTime::fromMSecsSinceEpoch(t).toString("hh:mm:ss");
        painter.drawText(x0+5, fm.height()*2+2, t_text);
        t = t+dt_grid;
    } while (t<=t_max);

    painter.setPen(QPen(QBrush(this->palette().foreground().color()), 2));

    x1 = x_to_screen(t_min);
    y1 = y_to_screen(draw_data->first().log_value);
    for(int i=1;i<draw_data->size();i++) {
        x0 = x1;
        y0 = y1;
        x1 = x_to_screen(draw_data->at(i).log_time);
        y1 = y_to_screen(draw_data->at(i).log_value);
        painter.drawLine(x0, y0, x1, y1);
    }
}

float MSensorDrawSurface::x_to_screen(quint64 x)
{
    return ((float) (x-t_min) / (t_max-t_min)) * width();
}

float MSensorDrawSurface::y_to_screen(float y)
{
    float result;
    if (y_max > y_min) {
        result = height()*0.95 - ( (y-y_min) / (y_max-y_min)) * height() * 0.9;
    } else {
        result = 0.5*height();
    }
    return result;
}
