#include "msensordrawsurface.h"

MSensorDrawSurface::MSensorDrawSurface(Sensor *d_sensor, QWidget *parent) :
    QWidget(parent)
{
    sensor = d_sensor;
    draw_data = sensor->startLogDataTracking(24*60*60*1000);
    sy_top = 0;
    sy_bottom = height();
    sx_left = 0;
    sx_right = width();
}

void MSensorDrawSurface::updateLogGraphicsParameters()
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
    show_dates = (t_max - t_min) > 24*60*60*1000;

    int min_w;
    int max_w;
    if (sensor && sensor->getSensorDataType() == Sensor::SDT_ENUM) {
        min_w = fontMetrics().width(sensor->getEnumFalse());
        max_w = fontMetrics().width(sensor->getEnumTrue());
    } else {
        min_w = fontMetrics().width(QString::number(y_min, 'g', 4));
        max_w = fontMetrics().width(QString::number(y_max, 'g', 4));
    }

    sy_top = fontMetrics().height() + 2;
    if (show_dates) sy_top += fontMetrics().height() + 1;
    sy_bottom = height() - 2;
    sx_left = (min_w>max_w?min_w:max_w)+3;
    sx_right = width()-1;
}

void MSensorDrawSurface::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    if (this->isHidden() || !this->isVisible()) return;

    QPainter painter(this);
    painter.setBackground(palette().background().color());
    painter.setPen(palette().background().color());
    painter.drawRect(rect().left(),rect().top(),rect().right()-1,rect().bottom()-1);
    painter.setFont(this->font());

    if (!sensor || sensor->getValue().isEmpty()) return;
    if (!draw_data || draw_data->size()<2) return;

    updateLogGraphicsParameters();

    float x0, y0, x1, y1;
    quint64 t = t_min;
    QString t_text;

    painter.setPen(palette().text().color());

    y0 = y_to_screen(y_min);
    y1 = y_to_screen(y_max);

    if (sensor && sensor->getSensorDataType() == Sensor::SDT_ENUM) {
        if (y_max > y_min) {
            painter.drawLine(0, y0, width(), y0);
            painter.drawText(QPoint(1, y0-1), sensor->getEnumFalse());
            painter.drawLine(0, y1, width(), y1);
            painter.drawText(QPoint(1, y1+fontMetrics().height()-1), sensor->getEnumTrue());
        } else {
            painter.drawLine(0, y0, width(), y0);
            painter.drawText(QPoint(1, y0-1), y_min>0 ? sensor->getEnumTrue() : sensor->getEnumFalse());
        }
    } else {
        painter.drawLine(0, y0, width(), y0);
        painter.drawText(QPoint(1, y0-1), QString::number(y_min, 'g', 4));
        if (y_max > y_min) {
            painter.drawLine(0, y1, width(), y1);
            painter.drawText(QPoint(1, y1+fontMetrics().height()-1), QString::number(y_max, 'g', 4));
        }
    }

    do {
        x0 = x_to_screen(t);
        painter.drawLine(x0, 0, x0, height());
        if (show_dates) {
            t_text = QDateTime::fromMSecsSinceEpoch(t).toString("dd.MM.yyyy");
            painter.drawText(QPoint(x0+2, fontMetrics().height()-1), t_text);
        }
        t_text = QDateTime::fromMSecsSinceEpoch(t).toString("hh:mm:ss");
        painter.drawText(QPoint(x0+2, show_dates ? (fontMetrics().height()*2-1) : (fontMetrics().height()-1)), t_text);
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
        if (sensor && sensor->getSensorDataType() == Sensor::SDT_ENUM) {
            if (draw_data->at(i).log_value>0 && draw_data->at(i-1).log_value>0) {
                painter.fillRect(x0, y0, x1-x0, y_to_screen(y_min)-y0+1, QBrush(this->palette().foreground().color(), Qt::DiagCrossPattern));
            }
        }
        painter.drawLine(x0, y0, x1, y1);
    }
}

float MSensorDrawSurface::x_to_screen(quint64 x)
{
    return sx_left + ((float) (x-t_min) / (t_max-t_min)) * (sx_right - sx_left);
}

float MSensorDrawSurface::y_to_screen(float y)
{
    float result;
    if (y_max > y_min) {
        result = sy_bottom - ( (y-y_min) / (y_max-y_min)) * (sy_bottom - sy_top);
    } else {
        result = 0.5*(sy_bottom - sy_top);
    }
    return result;
}
